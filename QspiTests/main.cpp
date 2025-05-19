#include "daisy_seed.h"

using namespace daisy;

#ifndef DSY_BOOT_TIMEOUT_MS
#define DSY_BOOT_TIMEOUT_MS 2000
#endif

uint32_t startup_process()
{
	// Enable backup SRAM
	System::InitBackupSram();

	// Write the bootloader version
	boot_info.version = System::BootInfo::Version::v6_1;

	if (boot_info.status == System::BootInfo::Type::JUMP)
	{
		boot_info.status = System::BootInfo::Type::INVALID;
		// Do jump
		typedef void _Noreturn (*EntryPoint)(void);

		auto memory = System::GetMemoryRegion(boot_info.data);

		if (memory == System::MemoryRegion::QSPI)
		{
			QSPIHandle::Config qspi_config;

			qspi_config.device = QSPIHandle::Config::Device::IS25LP064A;
			qspi_config.mode = QSPIHandle::Config::Mode::MEMORY_MAPPED;

			qspi_config.pin_config.io0 = Pin(PORTF, 8);
			qspi_config.pin_config.io1 = Pin(PORTF, 9);
			qspi_config.pin_config.io2 = Pin(PORTF, 7);
			qspi_config.pin_config.io3 = Pin(PORTF, 6);
			qspi_config.pin_config.clk = Pin(PORTF, 10);
			qspi_config.pin_config.ncs = Pin(PORTG, 6);

			QSPIHandle qspi;

			qspi.Init(qspi_config);
		}

		volatile uint32_t application_address = *(volatile uint32_t *)(boot_info.data + 4);
		EntryPoint application = (EntryPoint)(application_address);
		SCB->VTOR = boot_info.data;
		__set_MSP(*((volatile uint32_t *)boot_info.data));
		application();
	}

	if (boot_info.status == System::BootInfo::Type::SKIP_TIMEOUT)
	{
		boot_info.status = System::BootInfo::Type::INVALID;
		// do the regular startup, but skip the timeout
		return 100;
	}
	else if (boot_info.status == System::BootInfo::Type::INF_TIMEOUT)
	{
		boot_info.status = System::BootInfo::Type::INVALID;
		// do startup permanently into boot/DFU mode (uint32_t max milliseconds, or 1000+ hours)
		return UINT32_MAX;
	}

	return DSY_BOOT_TIMEOUT_MS;
}

DaisySeed hw;

static constexpr uint32_t kWriteVal1 = 0xDEADBEEF;
static constexpr uint32_t kWriteVal2 = 0xC0A1E5CE;
static constexpr uint32_t kWriteVal3 = 0x00000000;
static constexpr uint32_t kEraseVal = 0xFFFFFFFF;

void AudioCallback(AudioHandle::InputBuffer in,
				   AudioHandle::OutputBuffer out,
				   size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		out[0][i] = out[1][i] = 0;
	}
}

static constexpr size_t kNumWords = 24000;

uint32_t __attribute__((section(".dtcmram_bss"))) workspace[kNumWords];

int main(void)
{

	uint32_t timeout_ms = startup_process();

	hw.Init(true);

	SCB_DisableDCache();

	hw.StartAudio(AudioCallback);

	// zero workspace
	std::fill(workspace, workspace + kNumWords, 0);

	size_t erase_err = 0;
	size_t write_err = 0;

	// Very Basic QSPI tests
	// Erase the first words
	hw.qspi.Erase(0, kNumWords * sizeof(uint32_t));
	// check the mem
	for (size_t i = 0; i < kNumWords; i++)
	{
		uint32_t *val = (uint32_t *)(0x90000000 + (i * sizeof(uint32_t)));
		erase_err += (*val != kEraseVal);
	}

	// Write check 1
	std::fill(workspace, workspace + kNumWords, kWriteVal3);
	hw.qspi.Write(0, kNumWords * sizeof(uint32_t), (uint8_t *)workspace);

	// check the mem
	for (size_t i = 0; i < kNumWords; i++)
	{
		uint32_t *val = (uint32_t *)(0x90000000 + (i * sizeof(uint32_t)));
		write_err += (*val != kWriteVal3);
	}

	while (1)
	{
	}
}
