#!/usr/bin/env python3
import sys
import os
import xml.etree.ElementTree as ET

def generate_helpers(xml_file, out_file):
    tree = ET.parse(xml_file)
    root = tree.getroot()
    
    protocol_name = root.get('name')
    base_name = os.path.basename(xml_file).replace('.xml', '')
    
    with open(out_file, 'w') as f:
        f.write(f"/* Generated from {os.path.basename(xml_file)} */\n\n")
        f.write("#ifndef LVKW_WAYLAND_HELPERS_" + protocol_name.upper().replace('-', '_') + "_H\n")
        f.write("#define LVKW_WAYLAND_HELPERS_" + protocol_name.upper().replace('-', '_') + "_H\n\n")
        f.write("#include <stdint.h>\n")
        f.write("#include <stddef.h>\n\n")
        
        # We rely on the context structure being defined before including this header
        f.write("typedef struct LVKW_Context_WL LVKW_Context_WL;\n\n")
        
        for interface in root.findall('interface'):
            iface_name = interface.get('name')
            
            # Forward declarations to avoid "declared inside parameter list" warnings
            f.write(f"struct {iface_name};\n")
            f.write(f"struct {iface_name}_listener;\n")
            
            # Also forward declare interfaces used in requests
            for request in interface.findall('request'):
                for arg in request.findall('arg'):
                    arg_iface = arg.get('interface')
                    if arg_iface:
                        f.write(f"struct {arg_iface};\n")

            generated_funcs = set()
            f.write(f"/* interface {iface_name} */\n")
            
            def write_func(name, ret, args, body):
                if name in generated_funcs:
                    return
                generated_funcs.add(name)
                f.write(f"static inline {ret}\n{name}({', '.join(args)})\n")
                f.write("{\n")
                for line in body:
                    f.write(f"\t{line}\n")
                f.write("}\n\n")

            # set_user_data
            write_func(f"lvkw_{iface_name}_set_user_data", "void", 
                       [f"LVKW_Context_WL *ctx", f"struct {iface_name} *{iface_name}", "void *user_data"],
                       [f"ctx->dlib.wl.proxy_set_user_data((struct wl_proxy *) {iface_name}, user_data);"])
            
            # get_user_data
            write_func(f"lvkw_{iface_name}_get_user_data", "void *", 
                       [f"LVKW_Context_WL *ctx", f"struct {iface_name} *{iface_name}"],
                       [f"return ctx->dlib.wl.proxy_get_user_data((struct wl_proxy *) {iface_name});"])

            # get_version
            write_func(f"lvkw_{iface_name}_get_version", "uint32_t", 
                       [f"LVKW_Context_WL *ctx", f"struct {iface_name} *{iface_name}"],
                       [f"return ctx->dlib.wl.proxy_get_version((struct wl_proxy *) {iface_name});"])
            
            # add_listener
            write_func(f"lvkw_{iface_name}_add_listener", "int", 
                       [f"LVKW_Context_WL *ctx", f"struct {iface_name} *{iface_name}", f"const struct {iface_name}_listener *listener", "void *data"],
                       [f"return ctx->dlib.wl.proxy_add_listener((struct wl_proxy *) {iface_name}, (void (**)(void)) listener, data);"])

            # requests
            for request in interface.findall('request'):
                req_name = request.get('name')
                is_destructor = request.get('type') == 'destructor'
                args = request.findall('arg')
                
                c_args = [f"LVKW_Context_WL *ctx", f"struct {iface_name} *{iface_name}"]
                new_id_arg = None
                for arg in args:
                    arg_name = arg.get('name')
                    arg_type = arg.get('type')
                    arg_iface = arg.get('interface')
                    
                    if arg_type == 'int': c_args.append(f"int32_t {arg_name}")
                    elif arg_type == 'uint': c_args.append(f"uint32_t {arg_name}")
                    elif arg_type == 'fixed': c_args.append(f"wl_fixed_t {arg_name}")
                    elif arg_type == 'string': c_args.append(f"const char *{arg_name}")
                    elif arg_type == 'object': c_args.append(f"struct {arg_iface} *{arg_name}")
                    elif arg_type == 'array': c_args.append(f"struct wl_array *{arg_name}")
                    elif arg_type == 'fd': c_args.append(f"int32_t {arg_name}")
                    elif arg_type == 'new_id':
                        new_id_arg = arg
                        if not arg_iface:
                            c_args.append(f"const struct wl_interface *interface")
                            c_args.append(f"uint32_t version")

                ret_type = "void"
                if new_id_arg is not None:
                    ret_type = f"struct {new_id_arg.get('interface')} *" if new_id_arg.get('interface') else "void *"

                body = []
                if new_id_arg is not None:
                    body.append("struct wl_proxy *id;")
                
                marshal_flags = "WL_MARSHAL_FLAG_DESTROY" if is_destructor else "0"
                opcode = f"{iface_name.upper()}_{req_name.upper()}"
                marshal_args = [f"(struct wl_proxy *) {iface_name}", opcode]
                
                if new_id_arg is not None:
                    marshal_args.append(f"&{new_id_arg.get('interface')}_interface" if new_id_arg.get('interface') else "interface")
                else:
                    marshal_args.append("NULL")
                
                marshal_args.append(f"ctx->dlib.wl.proxy_get_version((struct wl_proxy *) {iface_name})")
                marshal_args.append(marshal_flags)
                
                for arg in args:
                    if arg.get('type') == 'new_id':
                        if not arg.get('interface'):
                            marshal_args.extend(["interface->name", "version"])
                        marshal_args.append("NULL")
                    else:
                        marshal_args.append(arg.get('name'))
                
                if new_id_arg is not None:
                    body.append(f"id = ctx->dlib.wl.proxy_marshal_flags({', '.join(marshal_args)});")
                    body.append(f"return ({ret_type}) id;")
                else:
                    body.append(f"ctx->dlib.wl.proxy_marshal_flags({', '.join(marshal_args)});")

                write_func(f"lvkw_{iface_name}_{req_name}", ret_type, c_args, body)

            # Default destroy if not already defined as a request
            write_func(f"lvkw_{iface_name}_destroy", "void", 
                       [f"LVKW_Context_WL *ctx", f"struct {iface_name} *{iface_name}"],
                       [f"ctx->dlib.wl.proxy_destroy((struct wl_proxy *) {iface_name});"])

        f.write("#endif\n")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <xml_file> <out_file>")
        sys.exit(1)
    generate_helpers(sys.argv[1], sys.argv[2])
