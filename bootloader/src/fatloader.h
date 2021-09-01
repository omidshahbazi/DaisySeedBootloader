#include <string.h>
#include "daisy_seed.h"

using namespace daisy;

SdmmcHandler sd;

enum Result {
	ABSENT,
	PRESENT,
	ERR,
};

#define MAX_FILENAME_LEN 256
#define TRY(func) if (func != FR_OK) return Result::PRESENT

char binfile[MAX_FILENAME_LEN];

Result LoadFAT(FILINFO* info);

Result CheckFAT();