#include <string.h>
#include "daisy_seed.h"

// TODO -- this will need to be extended for USB drives

using namespace daisy;

enum Result {
	ABSENT,
	PRESENT,
	ERR,
};

void MsdPrepare(DaisySeed& hw);
bool MsdReady();

bool EnsureValidBinary(size_t file_size, System::ProgramMemory* mem);

Result LoadFAT(DaisySeed& hw, FILINFO* info, uint32_t base_address);

Result TryLoadingFAT(DaisySeed& hw, uint32_t base_address);