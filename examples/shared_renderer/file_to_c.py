import sys
import os
import struct

def generate_c_source(entries, output_c, output_h):
    with open(output_c, 'w') as c_file, open(output_h, 'w') as h_file:
        guard = os.path.basename(output_h).replace('.', '_').upper() + "_INCLUDED"
        h_file.write(f"#ifndef {guard}\n")
        h_file.write(f"#define {guard}\n\n")
        h_file.write("#include <stdint.h>\n")
        h_file.write("#include <stddef.h>\n\n")
        
        c_file.write(f'#include "{os.path.basename(output_h)}"\n\n')

        for var_name, input_file in entries:
            with open(input_file, 'rb') as f:
                data = f.read()
            
            # Ensure 4-byte alignment for SPV
            padding = (4 - (len(data) % 4)) % 4
            padded_data = data + b'\0' * padding
            
            words = struct.unpack(f'<{len(padded_data)//4}I', padded_data)
            
            c_file.write(f"const uint32_t {var_name}[] = {{\n")
            for i, word in enumerate(words):
                c_file.write(f"0x{word:08x}, ")
                if (i + 1) % 8 == 0:
                    c_file.write("\n")
            c_file.write("\n};\n")
            c_file.write(f"const size_t {var_name}_len = {len(data)};\n\n")
            
            h_file.write(f"extern const uint32_t {var_name}[];\n")
            h_file.write(f"extern const size_t {var_name}_len;\n\n")

        h_file.write("#endif\n")

if __name__ == "__main__":
    if len(sys.argv) < 4 or (len(sys.argv) - 3) % 2 != 0:
        print("Usage: python file_to_c.py <output_c> <output_h> <var_name1> <file1> [<var_name2> <file2> ...]")
        sys.exit(1)

    output_c = sys.argv[1]
    output_h = sys.argv[2]
    
    entries = []
    for i in range(3, len(sys.argv), 2):
        entries.append((sys.argv[i], sys.argv[i+1]))

    generate_c_source(entries, output_c, output_h)
