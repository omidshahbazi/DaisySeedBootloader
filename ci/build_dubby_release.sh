#!/bin/bash

OUTPUT_DIR=dist-dubby
DESC_MFR_NAME="Componental"
DESC_PRODUCT_NAME="Dubby"
CUSTOM_PREFIX="dubby_"

mkdir -p $OUTPUT_DIR
rm $OUTPUT_DIR/*

# Build the variants
root_dir=$(pwd)

cd $root_dir/bootloader

echo "Building Internal DFU 2000ms"
make clean
EXTRA_C_DEFS="-DDSY_USB_DESC_MFR_STR=\\\"Componental\\\" -DDSY_USB_DESC_PRODUCT_STR=\\\"Dubby\\\" -DDSY_BOOT_TIMEOUT_MS=2000" TARGET_SUFFIX=-intdfu-2000ms make
cp build/*.bin ../$OUTPUT_DIR/

echo "Building Internal DFU 10ms"
make clean
EXTRA_C_DEFS="-DDSY_USB_DESC_MFR_STR=\\\"Componental\\\" -DDSY_USB_DESC_PRODUCT_STR=\\\"Dubby\\\" -DDSY_BOOT_TIMEOUT_MS=10" TARGET_SUFFIX=-intdfu-10ms make
cp build/*.bin ../$OUTPUT_DIR/

echo "Building External DFU 2000ms"
make clean
EXTRA_C_DEFS="-DDSY_USB_DESC_MFR_STR=\\\"Componental\\\" -DDSY_USB_DESC_PRODUCT_STR=\\\"Dubby\\\" -DDSY_BOOT_TIMEOUT_MS=2000 -DDSY_DFU_USE_EXT_USB" TARGET_SUFFIX=-extdfu-2000ms make
cp build/*.bin ../$OUTPUT_DIR/

echo "Building External DFU 10ms"
make clean
EXTRA_C_DEFS="-DDSY_USB_DESC_MFR_STR=\\\"Componental\\\" -DDSY_USB_DESC_PRODUCT_STR=\\\"Dubby\\\" -DDSY_BOOT_TIMEOUT_MS=10 -DDSY_DFU_USE_EXT_USB" TARGET_SUFFIX=-extdfu-10ms make
cp build/*.bin ../$OUTPUT_DIR/

echo "renaming files..."
for f in ../$OUTPUT_DIR/dsy_bootloader_v*; do
    [ -e "$f" ] || continue
    base="$(basename "$f")"
    new_name="../$OUTPUT_DIR/${CUSTOM_PREFIX}${base#dsy_}"
    mv $f $new_name
done

echo "Done."
