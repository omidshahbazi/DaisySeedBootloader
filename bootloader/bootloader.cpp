#include "dfu.h"
#include "daisy_seed.h"

using namespace daisy;

DaisySeed hw;
DFUHandle dfu;

int main(void)
{
	hw.Configure();
	hw.Init();

	dfu.Init(&hw.qspi);

	while(1) {
		// bool blink = System::GetNow() & (1 << 10);
		// hw.SetLed(blink);
	}
}
