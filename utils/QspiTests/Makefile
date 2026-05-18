# Project Name
TARGET = main

# Sources
CPP_SOURCES = main.cpp \
$(wildcard src/*.cpp)

#USE_FATFS = 1

# Modification flag for using external USB port as DFU
#C_DEFS += -DDSY_DFU_USE_EXT_USB=1
# Modify default boot timeout
#C_DEFS += -DDSY_BOOT_TIMEOUT_MS=200

C_INCLUDES = -Isrc

C_SOURCES += \
../../cube_dfu/Middlewares/ST/STM32_USB_Device_Library/Class/DFU/Src/usbd_dfu.c \
../../cube_dfu/USB_DEVICE/App/usbd_desc.c

C_INCLUDES += \
-I. \
-I../../cube_dfu/Middlewares/ST/STM32_USB_Device_Library/Class/DFU/Inc \
-I../../cube_dfu/USB_DEVICE/App

LDSCRIPT = boot_linker.lds

# WARNING -- an opt of -O3 causes the jump to fail and makes USB connectivity unreliable
OPT = -O0
DEBUG=1

# CFLAGS += -DUSBD_SUPPORT_USER_STRING_DESC=1

# Library Locations
LIBDAISY_DIR = ../../libDaisy

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
