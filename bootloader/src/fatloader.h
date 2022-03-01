#include <string.h>
#include "daisy_seed.h"

#define BOOTLOADER_LOG_NAME "daisy_boot_log.txt"

using namespace daisy;

enum Result {
	ABSENT,
	PRESENT,
	ALREADY_LOADED,
	ERR,
};

Result EnsureValidBinary(size_t file_size, System::MemoryRegion* mem, uint32_t base_address);

Result LoadFAT(DaisySeed& hw, FILINFO* info, uint32_t base_address);

Result TryLoadingFAT(DaisySeed& hw, uint32_t base_address);

void Deinit_Fatfs();