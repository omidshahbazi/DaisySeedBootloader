#include "bootloader.h"
#include "fatloader.h"
#include "daisy_seed.h"

using namespace daisy;

DaisySeed hw;
Bootloader boot;

void ManageMsd()
{
	if (MsdReady())
	{
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
	}
}

int main(void)
{
	hw.Configure();
	hw.Init(true);

	SCB_DisableDCache();

	boot.Init(hw);

	MsdPrepare(hw);

	// Result res = TryLoadingFAT(hw, System::qspi_start - QSPI_INITIAL);

	// if (res == Result::ERR)
	// {
	// 	// Either there was a .bin file with an invalid executable or
	// 	// an sdcard was present that failed to mount / open
	// 	boot.SosLed();
	// }
	// else if (res == Result::PRESENT)
	// { 
	// 	boot.LoadProgram();
	// }

	// No SD card, so check for USB drive

	// TODO -- usb drive

	// Otherwise, wait for a DFU interaction

	while(1) {
		ManageMsd();
		boot.AwaitDFU();
	}
}
