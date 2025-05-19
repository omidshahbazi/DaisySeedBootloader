# Project Name
TARGET = TestApp

# Sources
CPP_SOURCES = main.cpp

C_INCLUDES += -I.

# Library Locations
LIBDAISY_DIR = ../libDaisy
DAISYSP_DIR = ../DaisySP

# Linker
# APP_TYPE = BOOT_QSPI
# LDSCRIPT = sram_linker.lds
# C_DEFS += -DBOOT_APP

APP_TYPE=BOOT_SRAM
LDSCRIPT=sram_bss_linker.lds

OPT = -O0
DEBUG=1

# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

