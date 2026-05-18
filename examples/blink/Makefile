# Project Name
TARGET = blink

# Sources
CPP_SOURCES = main.cpp

# Library Locations
LIBDAISY_DIR = ../../libDaisy

# Linker
APP_TYPE=BOOT_SRAM
# APP_TYPE = BOOT_QSPI
# LDSCRIPT = sram_linker.lds

OPT = -O0

# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

