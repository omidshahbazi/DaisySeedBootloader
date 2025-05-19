# Changelog

## v6.3.0

* Implements the QSPI Stabilization required to recover from the intermittent seed issue where the QSPI can get itself into write protect mode.

## v6.2.0

* This version was identical to v6.1, but had variants manually generated for various timeouts and repurposing the secondary USB for DFU instead of MSC

## v6.1.0
* Reduced the speed and bits of the SDMMC peripheral to 1 bit at 12.5 MHz

## v6.0.0
* Restructured the bootloader to offer a perfectly clean slate for the application it jumps to
  * This fixes any issues related to pins not functioning as expected when using the bootloader
* Added the ability to libDaisy for returning to the Daisy bootloader
  * This includes an additional option for (mostly) skipping the timeout period
* \>=v6 will not be compatible with applications that were compiled with libDaisy <=v5.2

## v5.4.0
* USB detection time increased by around 1 second
* Logical flow has been simplified

## v5.3.0
* Adjusts state machine to check SD and USB sources in a loop, meaning they're now hot-pluggable and should also be more reliable

## v5.1.0
* Error LED gives blink sequence related to error instead of SOS
* Improved LED and button responsiveness
* Refactored code for clearer structure and improved extensibility
* Reverted PID

## v5.0.0
* Added transaction logging for FatFS interactions
* FatFS interactions will only overwrite QSPI if the program is different from the one stored
* DFU class descriptor now uses 0xA360 PID, with "Electromsith" manufacturer string and "Daisy Bootloader" product string
* If the FatFS interface finds an invalid binary, the Daisy emits a single SOS and then ignores connected FatFS media (operating as if it were unconnected)
* Startup time reduced from 5 seconds to 2.5

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
