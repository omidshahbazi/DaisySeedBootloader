#include "fatloader.h"

#define MAX_FILENAME_LEN 256
#define TRY(func) if (func != FR_OK) return Result::PRESENT

char binfile[MAX_FILENAME_LEN];

#define FILE_BUFF_LEN 1024
static uint8_t file_data[FILE_BUFF_LEN];

// TODO -- make this a static constexpr of qspi
#define PAGE_SIZE 65536

SdmmcHandler sd;

bool EnsureValidBinary(FIL* file, size_t file_size)
{
  // right now, we're just expecting the raw binary
	uint32_t stack_ptr;
  uint32_t entry_point;
  UINT read;
  f_read(&SDFile, &stack_ptr, sizeof(stack_ptr), &read);
  f_read(&SDFile, &entry_point, sizeof(entry_point), &read);

  if (stack_ptr != System::expected_stack)
  {
    return false; // no need to rewind if the file isn't valid
  }

  // Verifying that the sizes match up
  auto mem = System::GetProgramMemory(entry_point);
  bool valid = true;
  switch (mem)
  {
    case System::AXI_SRAM:
      if (file_size > System::sram_end - System::sram_start)
        valid = false;
      break;
    case System::QSPI:
      if (file_size > System::qspi_end - System::qspi_start)
        valid = false;
      break;
    default:
      valid = false;
      break;
  }

  f_rewind(&SDFile);
  return valid;
}

Result LoadFAT(DaisySeed& hw, FILINFO* info, uint32_t base_address)
{
  size_t file_size = info->fsize;

	if (f_open(&SDFile, info->fname, FA_OPEN_EXISTING | FA_READ) != FR_OK)
	{
		return Result::ERR;
	}

  if (EnsureValidBinary(&SDFile, file_size))
  {
    // Write file data to QSPI
    UINT data_read;
    uint32_t data_written = 0;

    do 
    {
      if (data_written % PAGE_SIZE == 0)
      {
        hw.qspi.Erase(base_address + data_written, base_address + data_written + PAGE_SIZE);
      }
      f_read(&SDFile, file_data, FILE_BUFF_LEN, &data_read);
      
      hw.qspi.Write(base_address + data_written, data_read, file_data);
      data_written += data_read;
    }
    while (data_read == FILE_BUFF_LEN);

    return Result::PRESENT;
    
  }
  else
  {
    // sos?
    return Result::ERR;
  }
  
  return Result::PRESENT;
}

Result TryLoadingFAT(DaisySeed& hw, uint32_t base_address)
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
        return LoadFAT(hw, &info, base_address);
      }
    } while(result == FR_OK);

		// Not sure if this needs to be closed before reading files
    f_closedir(&dir);

    return Result::PRESENT;
	}

	return Result::ABSENT;
}