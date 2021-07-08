#include "daisy_seed.h"

using namespace daisy;

#define TEST_LEN 8192

uint8_t DSY_QSPI_BSS buffer[TEST_LEN];

DaisySeed hw;

int main(void)
{
	hw.Configure();
	hw.Init();

	hw.StartLog();
	QSPIHandle::Config config = hw.qspi.GetConfig();
	config.mode = QSPIHandle::Config::Mode::DSY_MEMORY_MAPPED;
	hw.qspi.Init(config);

	size_t index = 0;

	while(1) {
		hw.PrintLine("Address: %d, Char: %c", index, (char) buffer[index]);
		index = (index + 1) % TEST_LEN;
		hw.DelayMs(500);
	}
}
