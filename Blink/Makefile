# Project Name
TARGET = Blink

# Sources
CPP_SOURCES = Blink.cpp

C_INCLUDES += -I. \

# Library Locations
LIBDAISY_DIR = ../libDaisy
DAISYSP_DIR = ../DaisySP

# Linker
LDSCRIPT = sram_linker_sdram.lds

#OPT = -O0

# NOTE -- this is necessary to ensure the startup doesn't try to initialize sdram
C_DEFS += -DBOOT_VOLATILE

# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

