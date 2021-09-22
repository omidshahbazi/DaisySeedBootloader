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

	SCB_DisableDCache();

	boot.Init(hw);

	while(1) {
		Result res = TryLoadingFAT(hw, System::kQspiStart - QSPI_INITIAL);

		if (res == Result::ERR)
		{
			// Either there was a .bin file with an invalid executable or
			// an drive was present that failed to mount / open
			boot.SosLed();
		}
		else if (res == Result::PRESENT)
		{ 
			boot.LoadProgram();
		}

		boot.AwaitDFU();
	}
}
