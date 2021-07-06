#include <cstdint>

#include "dfu.h"
#include "per/qspi.h"

#include "usbd_desc.h"

#include "usbd_dfu.h"
#include "usbd_dfu_if.h"

using namespace daisy;

extern "C"
{
    USBD_HandleTypeDef hUsbDeviceFS;
}

class DFUHandle::Impl {
    public:
        Impl() {}
        ~Impl() {}

        Result Init(QSPIHandle* qspi);

        Result MemoryInit();
        Result MemoryDeinit();
        Result MemoryErase(uint32_t Add);
        Result MemoryWrite(uint8_t *src, uint8_t *dest, uint32_t Len);
        Result MemoryRead(uint8_t *src, uint8_t *dest, uint32_t Len);
        Result MemoryStatus(uint32_t Add, uint8_t Cmd, uint8_t *buffer);

    private:

        void VerifyQspiMode(QSPIHandle::Config::Mode mode);

        QSPIHandle* qspi_;
};

// Global dfu handle
DFUHandle::Impl dfu_impl;
uint8_t DSY_QSPI_TEXT qspi_buffer[TEST_LEN];

void DFUHandle::Impl::VerifyQspiMode(QSPIHandle::Config::Mode mode)
{
    if (qspi_->GetConfig().mode != mode)
    {
        QSPIHandle::Config config = qspi_->GetConfig();
        config.mode = mode;
        qspi_->Init(config);
    }
}

DFUHandle::Result DFUHandle::Impl::Init(QSPIHandle* qspi)
{
    if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK)
    {
        return Result::ERR;
    }
    if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_DFU) != USBD_OK)
    {
        return Result::ERR;
    }
    if (USBD_DFU_RegisterMedia(&hUsbDeviceFS, &USBD_DFU_fops_FS) != USBD_OK)
    {
        return Result::ERR;
    }
    if (USBD_Start(&hUsbDeviceFS) != USBD_OK)
    {
        return Result::ERR;
    }
    qspi_ = qspi;

    VerifyQspiMode(QSPIHandle::Config::Mode::INDIRECT_POLLING);

    return Result::OK;
}

// These two methods are questionable -- this might not be the desired behavior
DFUHandle::Result DFUHandle::Impl::MemoryInit()
{
    VerifyQspiMode(QSPIHandle::Config::Mode::INDIRECT_POLLING);
    return Result::OK;
}

DFUHandle::Result DFUHandle::Impl::MemoryDeinit()
{
    VerifyQspiMode(QSPIHandle::Config::Mode::DSY_MEMORY_MAPPED);
    return Result::OK;
}

// TODO -- erasing in 4k chunks is significantly slower
// than doing so in 32k
DFUHandle::Result DFUHandle::Impl::MemoryErase(uint32_t Add)
{
    VerifyQspiMode(QSPIHandle::Config::Mode::INDIRECT_POLLING);
    qspi_->Erase(Add, Add + 4096);
    return Result::OK;
}

DFUHandle::Result DFUHandle::Impl::MemoryWrite(uint8_t *src, uint8_t *dest, uint32_t Len)
{
    VerifyQspiMode(QSPIHandle::Config::Mode::INDIRECT_POLLING);
    qspi_->Write(*dest, Len, src);
    return Result::OK;
}

DFUHandle::Result DFUHandle::Impl::MemoryRead(uint8_t *src, uint8_t *dest, uint32_t Len)
{
    VerifyQspiMode(QSPIHandle::Config::Mode::DSY_MEMORY_MAPPED);
    for (size_t i = 0; i < Len; i++)
        dest[i] = qspi_buffer[*src + i];
    return Result::OK;
}

DFUHandle::Result DFUHandle::Impl::MemoryStatus(uint32_t Add, uint8_t Cmd, uint8_t *buffer)
{
    switch (Cmd)
    {
        case DFU_MEDIA_PROGRAM:
        // I'm assuming this is little-endian
        // 3 -> 3 milliseconds (0.2 typ page program * 16 (= 4096 bytes))
        buffer[0] = 3;
        break;

        default:
        case DFU_MEDIA_ERASE:
        // 45 milliseconds typ 4k erase
        buffer[0] = 45;
        break;
    }
    return Result::OK;
}

extern "C" 
{

    #define FLASH_DESC_STR "@Internal Flash   /0x08000000/03*016Ka,01*016Kg,01*064Kg,07*128Kg,04*016Kg,01*064Kg,07*128Kg"

    uint16_t MEM_If_Init_FS(void);
    uint16_t MEM_If_Erase_FS(uint32_t Add);
    uint16_t MEM_If_Write_FS(uint8_t *src, uint8_t *dest, uint32_t Len);
    uint8_t* MEM_If_Read_FS(uint8_t *src, uint8_t *dest, uint32_t Len);
    uint16_t MEM_If_DeInit_FS(void);
    uint16_t MEM_If_GetStatus_FS(uint32_t Add, uint8_t Cmd, uint8_t *buffer);

    __ALIGN_BEGIN USBD_DFU_MediaTypeDef USBD_DFU_fops_FS __ALIGN_END =
    {
        (uint8_t*)FLASH_DESC_STR,
        MEM_If_Init_FS,
        MEM_If_DeInit_FS,
        MEM_If_Erase_FS,
        MEM_If_Write_FS,
        MEM_If_Read_FS,
        MEM_If_GetStatus_FS
    };

    /**
     * @brief  Memory initialization routine.
     * @retval USBD_OK if operation is successful, MAL_FAIL else.
     */
    uint16_t MEM_If_Init_FS(void)
    {
        return dfu_impl.MemoryInit();
    }

    /**
     * @brief  De-Initializes Memory
     * @retval USBD_OK if operation is successful, MAL_FAIL else
     */
    uint16_t MEM_If_DeInit_FS(void)
    {
        return dfu_impl.MemoryDeinit();
    }

    /**
     * @brief  Erase sector.
     * @param  Add: Address of sector to be erased.
     * @retval 0 if operation is successful, MAL_FAIL else.
     */
    uint16_t MEM_If_Erase_FS(uint32_t Add)
    {
        return dfu_impl.MemoryErase(Add);
    }

    /**
     * @brief  Memory write routine.
     * @param  src: Pointer to the source buffer. Address to be written to.
     * @param  dest: Address to write to.
     * @param  Len: Number of data to be written (in bytes).
     * @retval USBD_OK if operation is successful, MAL_FAIL else.
     */
    uint16_t MEM_If_Write_FS(uint8_t *src, uint8_t *dest, uint32_t Len)
    {
        return dfu_impl.MemoryWrite(src, dest, Len);
    }

    /**
     * @brief  Memory read routine.
     * @param  src: Address to read from.
     * @param  dest: Pointer to the destination buffer.
     * @param  Len: Number of data to be read (in bytes).
     * @retval Pointer to the physical address where data should be read.
     */
    uint8_t *MEM_If_Read_FS(uint8_t *src, uint8_t *dest, uint32_t Len)
    {
        /* Return a valid address to avoid HardFault */
        return (uint8_t*) dfu_impl.MemoryRead(src, dest, Len);
    }

    /**
     * @brief  Get status routine
     * @param  Add: Address to be read from
     * @param  Cmd: Number of data to be read (in bytes)
     * @param  buffer: used for returning the time necessary for a program or an erase operation
     * @retval USBD_OK if operation is successful
     */
    uint16_t MEM_If_GetStatus_FS(uint32_t Add, uint8_t Cmd, uint8_t *buffer)
    {
        return dfu_impl.MemoryStatus(Add, Cmd, buffer);
        // /* USER CODE BEGIN 5 */
        // switch (Cmd)
        // {
        //     case DFU_MEDIA_PROGRAM:

        //     break;

        //     case DFU_MEDIA_ERASE:
        //     default:

        //     break;
        // }
        // return (USBD_OK);
        // /* USER CODE END 5 */
    }
}

/////////////////////////////////////////////////
// DFUHandle::Impl -> DFUHandle                 
/////////////////////////////////////////////////

DFUHandle::Result DFUHandle::Init(QSPIHandle* qspi)
{
    pimpl_ = &dfu_impl;
    return pimpl_->Init(qspi);
}