# Project Name
TARGET = oscillator

# Bootloader, execute from QSPI
APP_TYPE=BOOT_QSPI

DEBUG=1
OPT=-O0

# Sources
CPP_SOURCES = oscillator.cpp

# Library Locations
LIBDAISY_DIR ?= ../../libDaisy

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

