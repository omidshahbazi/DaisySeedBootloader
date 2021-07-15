# Project Name
TARGET = Blink

# Sources
CPP_SOURCES = Blink.cpp

C_INCLUDES += -I. \

# Library Locations
LIBDAISY_DIR = ../libDaisy
DAISYSP_DIR = ../DaisySP

# Linker
LDSCRIPT = sram_linker.lds

# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

