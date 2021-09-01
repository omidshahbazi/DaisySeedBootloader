#include "bootloader.h"
#include "fatloader.h"
#include "daisy_seed.h"

using namespace daisy;

DaisySeed hw;
Bootloader boot;



int main(void)
{
	hw.Configure();
	hw.Init(true);

	Result res = CheckFAT();

	if (res == Result::PRESENT)
	{
		// indicate error
	}

	// No SD card, so check for USB drive

	// TODO -- usb drive

	// Otherwise, wait for a DFU interaction
	while(1) {
		boot.AwaitDFU();
	}
}
