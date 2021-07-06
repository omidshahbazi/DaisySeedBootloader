# Project Name
TARGET = bootloader

# Sources
CPP_SOURCES = bootloader.cpp \
dfu.cpp

C_SOURCES += \
../cube_dfu/Middlewares/ST/STM32_USB_Device_Library/Class/DFU/Src/usbd_dfu.c \
../cube_dfu/USB_DEVICE/App/usbd_desc.c 

C_INCLUDES += \
-I. \
-I../cube_dfu/Middlewares/ST/STM32_USB_Device_Library/Class/DFU/Inc \
-I../cube_dfu/Middlewares/ST/STM32_USB_Device_Library/Core/Inc \
-I../cube_dfu/USB_DEVICE/App 

# Library Locations
LIBDAISY_DIR = ../libDaisy

OPT = -Os

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
