#!/bin/bash

mkdir -p dist
rm dist/*

# Build the variants
root_dir=$(pwd)

cd $root_dir/bootloader

echo "Building Internal DFU 2000ms"
make clean;
EXTRA_C_DEFS=-DDSY_BOOT_TIMEOUT_MS=2000 TARGET_SUFFIX=-intdfu-2000ms make
cp build/*.bin ../dist/

echo "Building Internal DFU 10ms"
make clean;
EXTRA_C_DEFS=-DDSY_BOOT_TIMEOUT_MS=10 TARGET_SUFFIX=-intdfu-10ms make
cp build/*.bin ../dist/

echo "Building External DFU 2000ms"
make clean;
EXTRA_C_DEFS="-DDSY_BOOT_TIMEOUT_MS=2000 -DDSY_DFU_USE_EXT_USB" TARGET_SUFFIX=-extdfu-2000ms make
cp build/*.bin ../dist/

echo "Building External DFU 10ms"
make clean;
EXTRA_C_DEFS="-DDSY_BOOT_TIMEOUT_MS=10 -DDSY_DFU_USE_EXT_USB" TARGET_SUFFIX=-extdfu-10ms make
cp build/*.bin ../dist/

echo "Done."

