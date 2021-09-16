#include "fatloader.h"
#include "msd.h"

#define MAX_FILENAME_LEN 256
#define TRY(func) if (func != FR_OK) return Result::PRESENT

char binfile[MAX_FILENAME_LEN];

#define FILE_BUFF_LEN 1024
static uint8_t file_data[FILE_BUFF_LEN];

// TODO -- make this a static constexpr of qspi
#define PAGE_SIZE 65536

// SdmmcHandler sd;
MSDHandle msd;

bool EnsureValidBinary(size_t file_size, System::ProgramMemory* mem)
{
  // right now, we're just expecting the raw binary
	uint32_t stack_ptr;
  uint32_t entry_point;
  UINT read;
  f_read(&SDFile, &stack_ptr, sizeof(uint32_t), &read);
  f_read(&SDFile, &entry_point, sizeof(uint32_t), &read);

  if (stack_ptr != System::expected_stack)
  {
    return false; // no need to rewind if the file isn't valid
  }

  // verifying that a valid memory space is used
  *mem = System::GetProgramMemory(entry_point);
  bool valid = true;
  if (*mem == System::ProgramMemory::INVALID_ADDRESS || *mem == System::ProgramMemory::INTERNAL_FLASH)
    valid = false;

  // Verifying that the sizes match up
  switch (*mem)
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

  System::ProgramMemory mem;

  if (EnsureValidBinary(file_size, &mem))
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

      // // Ensuring the memory isn't overrun
      // switch (mem) 
      // {
      //   case System::AXI_SRAM:
      //     if (data_written + data_read >= System::sram_end - System::sram_start)
      //       return Result::ERR;
      //     break;
      //   case System::QSPI:
      //     if (data_written + data_read >= System::qspi_end - System::qspi_start)
      //       return Result::ERR;
      //     break;
      //   default:
      //     return Result::ERR;
      // }
      
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
	// // Init SD Card
	// SdmmcHandler::Config sd_cfg;
	// sd_cfg.Defaults();
	// sd.Init(sd_cfg);

	// // Links libdaisy i/o to fatfs driver.
	// dsy_fatfs_init();

  msd.Init(hw);

	// Mount SD Card
	if (f_mount(&SDFatFS, SDPath, 1) == FR_OK)
	{
		DIR dir;
		FILINFO info;
		FRESULT result = FR_OK;

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

      if(strstr(info.fname, ".bin") || strstr(info.fname, ".BIN"))
      {
        return LoadFAT(hw, &info, base_address);
      }
    } while(result == FR_OK);

		// Not sure if this needs to be closed
    f_closedir(&dir);
	}

	return Result::ABSENT;
}