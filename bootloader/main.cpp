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

	bool attempted_fat = false;

	while(1) 
	{
		if (!attempted_fat)
		{
			Result res = TryLoadingFAT(hw, System::kQspiBootloaderOffset);
			if (res == Result::ERR)
			{
				// Either there was a .bin file with an invalid executable or
				// an drive was present that failed to mount / open
				boot.TriggerSos();
				attempted_fat = true;
			}
			else if (res == Result::PRESENT)
			{ 
				boot.LoadProgram();
				// If we got here without restarting, the loading encountered an error
				attempted_fat = true;
				// boot.DeInit();
				// goto restart;
			}
			else if (res == Result::ALREADY_LOADED)
			{
				attempted_fat = true;
			}
		}

		uint8_t error = boot.AwaitDFU();
		if (error)
		{
			boot.DeInit();
			boot.Init(hw);
		}
	}
}
