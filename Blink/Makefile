# Project Name
TARGET = Blink

# Sources
CPP_SOURCES = Blink.cpp

C_INCLUDES += -I. 

APP_MODE=BOOT_SRAM

# Library Locations
LIBDAISY_DIR = ../libDaisy
DAISYSP_DIR = ../DaisySP

# Linker
# APP_TYPE = BOOT_QSPI
LDSCRIPT = sram_linker.lds
#C_DEFS += -DBOOT_APP

OPT = -O0

# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

