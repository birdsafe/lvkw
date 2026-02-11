#!/bin/bash
set -e

PROTO_DIR="src/lvkw/wayland/protocols"
XML_DIR="$PROTO_DIR/xml"
GEN_DIR="$PROTO_DIR/generated"

# Check if wayland-scanner is available
if ! command -v wayland-scanner &> /dev/null; then
    echo "Error: wayland-scanner not found in PATH."
    exit 1
fi

mkdir -p "$GEN_DIR"

# Helper function
gen_proto() {
    local xml="$1"
    local base=$(basename "$xml" .xml)
    
    echo "Generating protocols for $base..."
    
    wayland-scanner client-header "$xml" "$GEN_DIR/$base-client-protocol.h"
    wayland-scanner private-code "$xml" "$GEN_DIR/$base-client-protocol.inc.h"
}

# Generate for all XML files in the directory
for xml in "$XML_DIR"/*.xml; do
    gen_proto "$xml"
done

echo "Done."
