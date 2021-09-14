#!/bin/bash
VERSION=v3
dfu-util -a 0 -s 0x08000000:leave -D ./dsy_bootloader_$VERSION.bin -d ,0483:df11
