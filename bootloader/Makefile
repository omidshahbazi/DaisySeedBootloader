# Project Name
TARGET = main

# Sources
CPP_SOURCES = main.cpp \
src/bootloader.cpp \
src/dfu.cpp \
src/fatloader.cpp \
src/msd.cpp

# USE_FATFS = 1
C_SOURCES += \
../cube_dfu/Middlewares/Third_Party/FatFS/src/diskio.c \
../cube_dfu/Middlewares/Third_Party/FatFS/src/ff_gen_drv.c \
../cube_dfu/Middlewares/Third_Party/FatFS/src/ff.c \
../cube_dfu/FATFS/App/fatfs.c

C_SOURCES += \
../cube_dfu/FATFS/Target/usbh_diskio.c \
../cube_dfu/USB_HOST/App/usb_host.c \
../cube_dfu/USB_HOST/Target/usbh_conf.c \
../cube_dfu/Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Src/usbh_msc_bot.c \
../cube_dfu/Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Src/usbh_msc_scsi.c \
../cube_dfu/Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Src/usbh_msc.c \
../cube_dfu/Middlewares/ST/STM32_USB_Host_Library/Core/Src/usbh_conf_template.c \
../cube_dfu/Middlewares/ST/STM32_USB_Host_Library/Core/Src/usbh_core.c \
../cube_dfu/Middlewares/ST/STM32_USB_Host_Library/Core/Src/usbh_ctlreq.c \
../cube_dfu/Middlewares/ST/STM32_USB_Host_Library/Core/Src/usbh_ioreq.c \
../cube_dfu/Middlewares/ST/STM32_USB_Host_Library/Core/Src/usbh_pipes.c

C_INCLUDES = -Isrc

C_SOURCES += \
../cube_dfu/Middlewares/ST/STM32_USB_Device_Library/Class/DFU/Src/usbd_dfu.c \
../cube_dfu/USB_DEVICE/App/usbd_desc.c 

C_INCLUDES += \
-I. \
-I../cube_dfu/Middlewares/ST/STM32_USB_Device_Library/Class/DFU/Inc \
-I../cube_dfu/Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc \
-I../cube_dfu/Middlewares/ST/STM32_USB_Host_Library/Core/Inc \
-I../cube_dfu/USB_DEVICE/App \
-I../cube_dfu/Middlewares/Third_Party/FatFS/src \
-I../cube_dfu/FATFS/Target \
-I../cube_dfu/FATFS/App \
-I../cube_dfu/USB_HOST/App \
-I../cube_dfu/USB_HOST/Target


# -I../cube_dfu/Middlewares/ST/STM32_USB_Device_Library/Core/Inc 

LDSCRIPT = boot_linker.lds

# WARNING -- an opt of -O3 causes the jump to fail and makes USB connectivity unreliable
OPT = -O0

# CFLAGS += -DUSBD_SUPPORT_USER_STRING_DESC=1

# Library Locations
LIBDAISY_DIR = ../libDaisy

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
