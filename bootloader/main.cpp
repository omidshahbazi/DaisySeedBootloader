#include "bootloader.h"
#include "fatloader.h"
#include "daisy_seed.h"

using namespace daisy;

DaisySeed hw;
Bootloader boot;

// NOTE -- seemingly random things will cause the startup to be unreliable.
// Not sure why this is so touchy.

int main(void)
{
	hw.Configure();
	hw.Init(true);

	// This is ABSOLUTELY necessary to prevent random USB connectivity failure on startup
	// and I have no idea why
	hw.DelayMs(10);

	boot.Init(hw);

	Result res = TryLoadingFAT(hw, System::qspi_start - QSPI_INITIAL);

	if (res == Result::ERR)
	{
		// Either there was a .bin file with an invalid executable or
		// an sdcard was present that failed to mount / open
		boot.SosLed();
	}
	else if (res == Result::PRESENT)
	{
		boot.LoadProgram();
	}

	// No SD card, so check for USB drive

	// TODO -- usb drive

	// Otherwise, wait for a DFU interaction
	while(1) {
		boot.AwaitDFU();
	}
}
