# ES_QSPI_Bootloader

This project can be built with the top-level makefile or with VSCode tasks. You can do something like the following to build and upload in MIDI mode:

~~~ bash
make 
make upload-dfu
~~~

This project has also been set up for debugging. Hitting F5 should do the trick ;). As long as the Daisy has previously enumerated itself to the host (and you didn't physically disconnect the board), you should be able to do debugging with a successful USB connection no problem.

## Progress

- Set up a project and build environment for a DFU program. Cube's abstractions are _somewhat_ helpful in this case, largely existing in the `usb_dfu_if.c` file. I think that's where most of our logic can go for writing to spi flash.