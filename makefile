
# quick and dirty

CHIPSET ?= stm32h7x

PROJECT = cube_dfu

TARGET = $(PROJECT)/build/$(PROJECT)

TARGET_BIN=$(TARGET).bin
TARGET_ELF=$(TARGET).elf
FLASH_ADDRESS ?= 0x08000000

OCD=openocd
OCD_DIR ?= /usr/local/share/openocd/scripts
PGM_DEVICE ?= interface/stlink.cfg
OCDFLAGS = -f $(PGM_DEVICE) -f target/$(CHIPSET).cfg

ifdef DEBUG
CFLAGS += -g
endif

.PHONY: all program-dfu build-midi build-cdc program-ocd clean build-both

all: build-midi

build-midi:
	cd $(PROJECT); make MIDI=1;

build-cdc:
	cd $(PROJECT); make;

build-both:
	cd $(PROJECT); make BOTH=1;

program-dfu: build-midi
	dfu-util -a 0 -s $(FLASH_ADDRESS):leave -D .$(BUILD_DIR)/$(TARGET_BIN) -d ,0483:df11

program-ocd: build-midi
	$(OCD) -s $(OCD_DIR) $(OCDFLAGS) \
		-c "program ./build/$(TARGET).elf verify reset exit"

program-dfu-cdc: build-cdc
	dfu-util -a 0 -s $(FLASH_ADDRESS):leave -D .$(BUILD_DIR)/$(TARGET_BIN) -d ,0483:df11

program-ocd-cdc: build-cdc
	$(OCD) -s $(OCD_DIR) $(OCDFLAGS) \
		-c "program ./build/$(TARGET).elf verify reset exit"

clean:
	cd $(PROJECT); make clean;