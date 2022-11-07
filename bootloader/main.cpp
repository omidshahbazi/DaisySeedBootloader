#include "bootloader.h"
#include "daisy_seed.h"

using namespace daisy;

DaisySeed hw;
Bootloader boot;

void AudioCallback(AudioHandle::InputBuffer  in,
				   AudioHandle::OutputBuffer out,
				   size_t                    size)
{
	for (size_t i = 0; i < size; i++)
	{
		out[0][i] = out[1][i] = 0;
	}

	boot.AudioProcess(in, out, size);
}

int main(void) {

	uint32_t timeout_ms = startup_process();

	hw.Configure();
	hw.Init(true);

	SCB_DisableDCache();

	boot.Init(hw, timeout_ms);

	hw.StartAudio(AudioCallback);

	boot.IoInit();

	while(1)
	{
		boot.LoopProcess();
	}
}
