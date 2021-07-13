#include "dfu.h"
#include "daisy_seed.h"

using namespace daisy;

DaisySeed hw;
DFUHandle dfu;

int main(void)
{
	hw.Configure();
	hw.Init();

	dfu.Init(&hw);

	while(1) {
		dfu.PollJump();
	}
}
