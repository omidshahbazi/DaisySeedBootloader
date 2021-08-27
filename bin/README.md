# Changelog

## v2.0.0

* Added optional boot button press to extend DFU wait indefinitely
* Automatically parses binary and jumps to appropriate location
  * Currently supports SRAM and QSPI
* Removed most deinitialization

## v1.1

* Added SDRAM Init back into Daisy Seed to prevent issues using SDRAM in applications

## v1

* DFU Bootloader that waits at startup for 5 seconds.
