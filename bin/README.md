# Changelog

## v4.0.0
* Added USB Mass Storage Device bootloading capabilities
  * The FatFS code now supports USB drives
  * Drives can be hot-plugged at any time
  * SD cards take precedence if both an SD card and USB drive are present

## v3.0.0
* Added FATFS-based bootloading capabilities
  * Currently supports only SDCARDS
  * SD card must be inserted before reset / power on -- inserting during the bootload wait period will not result in correct detection

## v2.0.0

* Added optional boot button press to extend DFU wait indefinitely
* Automatically parses binary and jumps to appropriate location
  * Currently supports SRAM and QSPI
* Removed most deinitialization

## v1.1

* Added SDRAM Init back into Daisy Seed to prevent issues using SDRAM in applications

## v1

* DFU Bootloader that waits at startup for 5 seconds.
