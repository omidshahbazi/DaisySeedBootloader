#include "fatloader.h"

Result LoadFAT(FILINFO* info)
{
	if (f_open(&SDFile, info->fname, FA_OPEN_EXISTING | FA_READ) != FR_OK)
	{
		return Result::ERR;
	}
	// right now, we're just expecting the raw binary
	

  
  return Result::PRESENT;
}

Result CheckFAT()
{
	// Init SD Card
	SdmmcHandler::Config sd_cfg;
	sd_cfg.Defaults();
	sd.Init(sd_cfg);

	// Links libdaisy i/o to fatfs driver.
	dsy_fatfs_init();

	// Mount SD Card
	if (f_mount(&SDFatFS, SDPath, 1) == FR_OK)
	{
		DIR dir;
		FILINFO info;
		FRESULT result = FR_OK;
		char *  name;

		if(f_opendir(&dir, SDPath) != FR_OK)
    {
        return Result::ERR;
    }
    do
    {
        result = f_readdir(&dir, &info);
        // Exit if bad read or NULL fname
        if(result != FR_OK || info.fname[0] == 0)
            break;
        // Skip if its a directory or a hidden file.
        if(info.fattrib & (AM_HID | AM_DIR))
            continue;

        name = info.fname;

				if(strstr(name, ".bin") || strstr(name, ".BIN"))
				{
					// strcpy(binfile, name);
					LoadFAT(&info);
				}
    } while(result == FR_OK);

		// Not sure if this needs to be closed before reading files
    f_closedir(&dir);

    return Result::PRESENT;
	}

	return Result::ABSENT;
}