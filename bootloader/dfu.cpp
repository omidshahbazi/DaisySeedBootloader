#include <cstdint>

#include "dfu.h"
#include "daisy_seed.h"

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
    void enable_jump();
}

class DFUHandle::Impl {
    public:
        Impl() {}
        ~Impl() {}

        Result Init(DaisySeed* seed);

        Result MemoryInit();
        Result MemoryDeinit();
        Result MemoryErase(uint32_t Add);
        Result MemoryWrite(uint8_t *src, uint8_t *dest, uint32_t Len);
        Result MemoryRead(uint8_t *src, uint8_t *dest, uint32_t Len);
        Result MemoryStatus(uint32_t Add, uint8_t Cmd, uint8_t *buffer);

        void LoadProgram();
        bool dfu_complete;
        bool dfu_initiated;

    private:

        Result Deinit();
        void VerifyQspiMode(QSPIHandle::Config::Mode mode);
        static constexpr uint32_t addr_offset_ = 0x90000000U;
        static constexpr uint32_t sector_size_ = 0x10000U;
        static constexpr uint32_t expected_stack_ = 0x20020000U;
        size_t data_written_;
        DaisySeed* hw_;
        
};

// Global dfu handle
DFUHandle::Impl dfu_impl;
uint8_t DSY_QSPI_BSS qspi_buffer[PROGRAM_SPACE];
uint8_t DSY_SRAM_EXEC sram_program[SRAM_SPACE];
uint8_t DSY_ITCMRAM_EXEC itcmram_program[ITCMRAM_SPACE];

DFUHandle::Result DFUHandle::Impl::Init(DaisySeed* seed)
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
    HAL_PWREx_EnableUSBVoltageDetector();

    hw_ = seed;

    VerifyQspiMode(QSPIHandle::Config::Mode::INDIRECT_POLLING);
    data_written_ = 0;
    dfu_complete = false;
    dfu_initiated = false;

    return Result::OK;
}

DFUHandle::Result DFUHandle::Impl::Deinit()
{
    if (USBD_DeInit(&hUsbDeviceFS) != USBD_OK)
        return Result::ERR;
    HAL_PWREx_DisableUSBVoltageDetector();

    __DSB();

    // TODO -- create mechanism to ensure the
    // deinit happens after USB disconnect so
    // the disconnect can happen without hanging
    System::Delay(50);

    hw_->Deinit();

    return Result::OK;
}

void DFUHandle::Impl::VerifyQspiMode(QSPIHandle::Config::Mode mode)
{
    if (hw_->qspi.GetConfig().mode != mode)
    {
        auto config = hw_->qspi.GetConfig();
        config.mode = mode;
        hw_->qspi.Init(config);
    }
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
    // DFU download has begun, so we shouldn't allow a jump
    // to happen before it completes
    dfu_initiated = true;
    VerifyQspiMode(QSPIHandle::Config::Mode::INDIRECT_POLLING);
    Add -= addr_offset_;
    hw_->qspi.Erase(Add, Add + sector_size_);
    return Result::OK;
}

DFUHandle::Result DFUHandle::Impl::MemoryWrite(uint8_t *src, uint8_t *dest, uint32_t Len)
{
    VerifyQspiMode(QSPIHandle::Config::Mode::INDIRECT_POLLING);
    uint32_t write_addr = (uint32_t) dest - addr_offset_;
    hw_->qspi.Write(write_addr, Len, src);
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
    // The data caching can cause issues if we've recently
    // read QSPI and found no program in there, and then
    // actually write a program there and try to load it.
    SCB_InvalidateDCache();
    __DSB();

    VerifyQspiMode(QSPIHandle::Config::Mode::DSY_MEMORY_MAPPED);
    
    // If the stack pointer isn't here, then either the
    // download failed or was invalid, or a program
    // has never been written to flash
    uint32_t* stack_ptr = (uint32_t*) qspi_buffer;
    if (*stack_ptr != expected_stack_)
    {
        if (dfu_complete)
        {
            // this means the DFU transaction occurred, but we downloaded
            // bad data. This requires a restart to reset the DFU state machine
            HAL_NVIC_SystemReset();
        }
        return;
    }

    // TODO -- integrate itcmram
    for (size_t i = 0; i < SRAM_SPACE; i++)
    {
        // if (i < SRAM_SPACE)
        sram_program[i] = qspi_buffer[i];
        // else
        //     itcmram_program[i - SRAM_SPACE] = qspi_buffer[i];
    }
    
    Deinit();

    // disable interupts NOTE -- These two seem to cause
    // errors for the target application
    // __set_PRIMASK(1);
    // __disable_irq();
    RCC->CIER = 0x00000000;
    
    typedef void (*EntryPoint)(void);

    uint32_t application_address = *(__IO uint32_t*)(EXEC_START + 4);
    EntryPoint application = (EntryPoint)(application_address);
    SCB->VTOR = EXEC_START;
    __set_MSP(*(__IO uint32_t*)EXEC_START);
    application();
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

    void enable_jump()
    {
        dfu_impl.dfu_complete = true;
    }
}

/////////////////////////////////////////////////
// DFUHandle::Impl -> DFUHandle                 
/////////////////////////////////////////////////

DFUHandle::Result DFUHandle::Init(DaisySeed* seed)
{
    state_ = State::WAITING_ON_TIMEOUT;
    timeout_start_ = System::GetNow();

    pwm_tick_ = timeout_start_;
    angle_ = 0;

    hw_ = seed;
    pimpl_ = &dfu_impl;
    return pimpl_->Init(seed);
}

void DFUHandle::PollJump()
{
    // Prevents a jump during DFU download
    if (pimpl_->dfu_initiated)
        state_ = State::WAITING_ON_DFU;

    bool timeout_elapsed = System::GetNow() - timeout_start_ > timeout_;
    if (pimpl_->dfu_complete || (timeout_elapsed && state_ == State::WAITING_ON_TIMEOUT))
    {
        pimpl_->LoadProgram();
        // If we get here, then the program failed to load, meaning
        // there's likely no program in flash
        state_ = State::WAITING_ON_DFU;
    }
        
    SineLed();
}

void DFUHandle::SineLed()
{
    uint32_t time = System::GetNow();
    if (time > pwm_tick_)
    {
        pwm_tick_ = time;
        angle_ += (2 * M_PI / 1000.f) / sine_hz_;
        bool led = sin(angle_) * sine_fid_ + sine_fid_ - 1 > time % (sine_fid_ * 2);
        hw_->SetLed(led);   
    }
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