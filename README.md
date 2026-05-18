# Daisy Bootloader

## Summary

The Daisy Bootloader is an application that runs on the Daisy Seed, Seed2 DFM,
and PatchSM boards.
The program allows for firmware updates in a few ways:

- USB DFU on either the Daisy's built-in USB port, or an external port
connected to the Daisy's USB-capable pins
- Placing a file on a media drive (either SD Card, or as USB Host)

The bootloader stores firmware in the Daisy's QSPI flash starting at address
0x90040000 (256kB offset from QSPI start address).
The maximum size of a valid program will depend on how the program is compiled.
If a program is compiled for execution within the processor's internal memories
(ideal for best performance), the maximum size is 512kB with a custom linker
script. Meanwhile, a program compiled with `APP_TYPE=BOOT_QSPI` will execute the
program directly from the QSPI flash, allowing for rograms up to ~7.75MB in
size, but performance becomes more cache-dependent.

After checking for, and performing any necessary firmware updates, the bootloader
updates its state in the Daisy's backup SRAM, and does a full reset to uninitialize
any peripherals, etc. used for firmware update.
After the reset, the application will do minimal initialization to either startup the
QSPI, or to copy the binary from QSPI into SRAM for execution, and then jump to the
user firmware.

libDaisy allows users to trigger a jump back to the bootloader with control over the
timeout before booting back to the application (including an "infinite" timeout).

## Organization

- bootloader: the bootloader application source code
- cube_dfu: exports from STM32CubeMX providing USB classes that were not present
in libDaisy
- shared: the bootloader implementation, which can be used in other applications
to create customized bootloaders
- examples: example programs that are built for the bootloader
- utils: utility programs that assist in the development of the bootloader
- ci: helper scripts used for compiling multiple variants at once
- dist: latest distributable binaries in all variants of the current version of
the bootloader

## Development and Debugging

The `.vscode/` directory contains tasks for building the default configuration,
and debug configurations for the bootloader and examples.

When attempting to debug applications running on the bootloader, it is important to progarm the Daisy with the example using supported bootloader methods prior to attempting to launch the debugger.
At this time, the debug launch commands _do not_ program the application into the Daisy's QSPI flash, meaning the debugger may attach to out-dated, or incorrect firmware if the binary is not loaded through the bootloader first.


## Customization

There are a few ways of customizing the bootloader build:

1. Custom descriptor manufacturer/product strings
2. Custom timeouts/USB configu
3. Custom hardware targets, etc.

### C Defines

The two C defines for controlling USB descriptors are: `DSY_USB_DESC_MFR_STR` and `DSY_USB_DESC_PRODUCT_STR`

```sh
#!/bin/bash

# Customize these values:
MFR_STR="My Company"
PRODUCT_STR="Thingamabob 9000"
TIMEOUT_MS=10

# build the defines
EXTRA_C_DEFS=""
EXTRA_C_DEFS+=" -DDSY_USB_DESC_MFR_STR=\\\"${MFR_STR// /\\ }\\\""
EXTRA_C_DEFS+=" -DDSY_USB_DESC_PRODUCT_STR=\\\"${PRODUCT_STR// /\\ }\\\""
EXTRA_C_DEFS+=" -DDSY_BOOT_TIMEOUT_MS=${TIMEOUT_MS}"
EXTRA_C_DEFS+=" -DDSY_DFU_USE_EXT_USB" # comment this out to use the internal

# control the suffix on the build file:
TARGET_SUFFIX="-extdfu-${TIMEOUT_MS}ms"

# build it!

root_dir=$(pwd)
cd $root_dir/bootloader
make clean;
EXTRA_C_DEFS="${EXTRA_C_DEFS}" TARGET_SUFFIX=${TARGET_SUFFIX} make
OUTBIN=$(ls build/ | grep .bin)
echo "done building: ${OUTBIN}"
cd $root_dir
```

After building, this will provide the following output:

```
Memory region         Used Size  Region Size  %age Used
           FLASH:      127044 B       128 KB     96.93%
         DTCMRAM:       18468 B       128 KB     14.09%
       SRAM_EXEC:        480 KB       480 KB    100.00%
      SRAM_SPARE:       19620 B        32 KB     59.88%
      RAM_D2_DMA:       17880 B        32 KB     54.57%
          RAM_D2:          0 GB       256 KB      0.00%
          RAM_D3:          0 GB        64 KB      0.00%
     BACKUP_SRAM:          12 B         4 KB      0.29%
    ITCMRAM_EXEC:          0 GB        64 KB      0.00%
           SDRAM:          0 GB        64 MB      0.00%
       QSPIFLASH:        576 KB      7936 KB      7.26%
arm-none-eabi-objcopy -O ihex build/dsy_bootloader_v6_4-extdfu-10ms.elf build/dsy_bootloader_v6_4-extdfu-10ms.hex
arm-none-eabi-objcopy -O binary -S build/dsy_bootloader_v6_4-extdfu-10ms.elf build/dsy_bootloader_v6_4-extdfu-10ms.bin
done building: dsy_bootloader_v6_4-extdfu-10ms.bin
```

### Custom Hardware Integration

If the build-customizations aren't enough, and you need to make further tweaks (e.g. holding a button on startup consistently entering the bootloader, etc.) these can be done by building your own application that uses the `shared/` files.

The signature for the `Bootloader::Init` method has inputs for the most commonly tweaked things:

```cpp
  Result Init(QSPIHandle &qspi, Pin led_pin, Pin btn_pin, uint32_t timeout, deinit_cb_t deinit_cb = nullptr, void* deinit_context = nullptr);
```

In most cases, the `qspi` parameter will simply be the corresponding SOMs (i.e. DaisySeed, DaisyPatchSM) qspi member.

The `led_pin` parameter is the pin used as an output to indicate bootloader state visually

The `btn_pin` parameter is the pin that will be configured as an input to read to _remain_ in the bootloader once the program is running
If you'd like to program the bootloader to _always_ stay in the bootloader if this is held down at startup, it's possible to tweak the application flow to do so:

```cpp
const daisy::Pin kButtonPin = seed::D0;

daisy::GPIO btn;
btn.Init(kButtonPin, daisy::GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

timeout_ms = startup_process(&qspi_config);
if (!btn.Read()) {
    timeout_ms = UINT32_MAX;
}

// business as usual
hw.Init(true);
boot.Init(hw.qspi, Pin(daisy::PORTC, 7), kButtonPin, timeout_ms, MyDeInitCallback, (void*)&hw);
hw.StartAudio(AudioCallback);
boot.IoInit();

while(1) {
    boot.LoopProcess();
}
```

The `timeout` param controls how long the LoopProcess method will continue to wait for updates, before automatically resetting and jumping to the application. This value is in milliseconds.
As seen above, the "infinite" timeout is done by setting this to UINT32_MAX, which is ~50 days.

The `deinit_cb` parameter takes an optional deinitialization callback with a signature `void Callback(void* context)` that is called prior to the bootloader attempting to jump to the application firmware.

The `deinit_context` is an optional parameter that will be passed to the `deinit_cb` when it is run.

### Going further

Beyond this, it is possible to add new methods of firmware update, remove existing ones, etc.

In most cases, we recommend creating a new repo for your project, and loading the DaisyBootloader as a submodule.
This lets you easily update to the latest bootloader versions while maintaining your customizations.
If your project requires modifications to the bootloader source code not possible with the existing API, we recommend creating a fork of the repo to facilitate syncronization with future changes, or even contributing enhancements to the API for improved customization.
