#include <cstdint>

#include "dfu.h"
#include "per/qspi.h"

#include "usbd_def.h"
#include "usbd_desc.h"
#include "usbd_dfu.h"
#include "usbd_dfu_if.h"
#include "system.h"

using namespace daisy;

extern "C"
{
    USBD_HandleTypeDef hUsbDeviceFS;
    extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
    void load_program();
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

        void LoadProgram();

    private:

        void VerifyQspiMode(QSPIHandle::Config::Mode mode);
        static constexpr uint32_t addr_offset_ = 0x90000000U;
        static constexpr uint32_t sector_size_ = 0x10000U;
        size_t data_written_;
        QSPIHandle* qspi_;
};

// Global dfu handle
DFUHandle::Impl dfu_impl;
uint8_t DSY_QSPI_BSS qspi_buffer[PROGRAM_SPACE];
uint8_t DSY_SRAM_EXEC sram_program[SRAM_SPACE];
uint8_t DSY_ITCMRAM_EXEC itcmram_program[ITCMRAM_SPACE];

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
    data_written_ = 0;

    return Result::OK;
}

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

DFUHandle::Result DFUHandle::Impl::MemoryErase(uint32_t Add)
{
    VerifyQspiMode(QSPIHandle::Config::Mode::INDIRECT_POLLING);
    Add -= addr_offset_;
    qspi_->Erase(Add, Add + sector_size_);
    return Result::OK;
}

DFUHandle::Result DFUHandle::Impl::MemoryWrite(uint8_t *src, uint8_t *dest, uint32_t Len)
{
    VerifyQspiMode(QSPIHandle::Config::Mode::INDIRECT_POLLING);
    uint32_t write_addr = (uint32_t) dest - addr_offset_;
    qspi_->Write(write_addr, Len, src);
    data_written_ += Len;
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
        buffer[0] = 0;  // bStatus (0 = OK) TODO -- make this actually check the status
        // I'm assuming this is little-endian
        // 3 -> 3 milliseconds (0.2 typ page program * 16 (= 4096 bytes))
        buffer[1] = 3;  // bwPollTimeout 0
        buffer[2] = 0;  // bwPollTimeout 1
        buffer[3] = 0;  // bwPollTimeout 2
        buffer[4] = 4;  // bState (4 = dfuDNBUSY)
        buffer[5] = 0;  // no state string
        break;

        default:
        case DFU_MEDIA_ERASE:
        // 150 milliseconds typ 64k erase
        buffer[0] = 0;   // bStatus (0 = OK) TODO -- make this actually check the status
        buffer[1] = 150; // bwPollTimeout 0
        buffer[2] = 0;   // bwPollTimeout 1
        buffer[3] = 0;   // bwPollTimeout 2
        buffer[4] = 4;   // bState (4 = dfuDNBUSY)
        buffer[5] = 0;   // no state string
        break;
    }
    return Result::OK;
}

void DFUHandle::Impl::LoadProgram()
{
    VerifyQspiMode(QSPIHandle::Config::Mode::DSY_MEMORY_MAPPED);
    for (size_t i = 0; i < data_written_; i++)
    {
        if (i < SRAM_SPACE)
            sram_program[i] = qspi_buffer[i];
        else
            itcmram_program[i - SRAM_SPACE] = qspi_buffer[i];
    }

    __DSB();
    SCB_DisableDCache();
    SCB_DisableICache();

    //shut down any tasks running
    // TODO -- stop all processes here (usb, etc.)
    HAL_RCC_DeInit();
    qspi_->Deinit();
    // HAL_DeInit(); // NOTE -- this causes the program to fail!

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    //disbale interuppts
    __set_PRIMASK(1);
    __disable_irq();

    // Loading address where we'll execute the program
    asm volatile(
        "mov r1, %0"
        :
        : "n"((EXEC_START >> 16) & 0xFFFF)
    );

    asm volatile("lsl r1, #16");

    // The lower two bytes will probably always
    // be zero, but just in case...
    asm volatile(
        "orr r1, %0"
        :
        : "n"(EXEC_START & 0xFFFF)
    );
    
    // No need to set the stack pointer, MSP, 
    // or VTOR, since the startup
    // routine does that anyway

    // The program binary stores the entry
    // point in the second 4-byte word, so
    // we load that into the register we'll
    // use to jump
    asm volatile("ldr r2, [r1, #4]");

    // Branch to the value in r2
    asm volatile("bx r2");
}

extern "C" 
{
    // The chip is split into 3 regions -- the first 256k is broken into 64 4K 
    // segments so smaller portions can be rewritten if necessary (say we need
    // a lookup table or something). The rest of the chip is split in two because
    // this memory layout syntax is very limited, and the number of segments
    // can only be two digits (so 124*64Kg is't possible).
    #define FLASH_INT_STR "@Flash /0x90000000/64*4Kg/0x90040000/60*64Kg/0x90400000/60*64Kg"

    uint16_t MEM_If_Init_FS(void);
    uint16_t MEM_If_Erase_FS(uint32_t Add);
    uint16_t MEM_If_Write_FS(uint8_t *src, uint8_t *dest, uint32_t Len);
    uint8_t* MEM_If_Read_FS(uint8_t *src, uint8_t *dest, uint32_t Len);
    uint16_t MEM_If_DeInit_FS(void);
    uint16_t MEM_If_GetStatus_FS(uint32_t Add, uint8_t Cmd, uint8_t *buffer);

    __ALIGN_BEGIN USBD_DFU_MediaTypeDef USBD_DFU_fops_FS __ALIGN_END =
    {
        (uint8_t*)FLASH_INT_STR,
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
    }

    void load_program()
    {
        dfu_impl.LoadProgram();
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

// IRQ Handler
extern "C"
{
    void OTG_FS_EP1_OUT_IRQHandler(void)
    {
        HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
    }

    void OTG_FS_EP1_IN_IRQHandler(void)
    {
        HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
    }

    void OTG_FS_IRQHandler(void) { HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS); }
}