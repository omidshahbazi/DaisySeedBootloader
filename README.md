# ES_QSPI_Bootloader

This project is organized into dependencies (cube_dfu, DaisySP, and libDaisy), the bootloader application (bootloader), and various target applications. When building the bootloader, make sure libDaisy is build _without_ VOLATILE defined. When building target applications, make sure VOLATILE _is_ defined (i.e. build VOLATILE=1, or by using the VSCode task build_libdaisy_volatile).

To upload to the bootloader, use the program-volatile target. It should produce the following command:

~~~ bash
dfu-util -a 0 -s 0x90040000:leave -D path/to/program.bin -d ,0483:df11
~~~

## Notes

- NOTE -- the dfu class really needs a solid way of ensuring the USB is disconnected before jumping. If you try to jump before it's disconnected (and in the process disable interrupts), then the program will hang on a `HAL_Delay` call made by the LL usb drivers. Probably the best thing to do would be to have some flag set by the USD disconnect function in the HAL drivers that we can reference in our code. Yes, it does require modifying HAL stuff, but we don't have to rely on any hacky delays or anything. 
