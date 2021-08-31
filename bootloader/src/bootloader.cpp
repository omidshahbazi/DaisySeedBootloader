#include <cstdint>

#include "bootloader.h"
#include "daisy_seed.h"

#include "usbd_def.h"
#include "usbd_desc.h"
#include "usbd_dfu.h"
#include "usbd_dfu_if.h"
#include "system.h"

using namespace daisy;

class Bootloader::Impl {
    public:
        Impl() {}
        ~Impl() {}

        Result Init(DaisySeed* seed);

        void LoadProgram();

        // TODO -- use System vars when qspi_cpp is merged!
        static constexpr uint32_t addr_offset = 0x90000000U;
        static constexpr uint32_t sector_size = 0x10000U;
        static constexpr uint32_t expected_stack = 0x20020000U;

        static constexpr uint32_t sram_start = 0x24000000U;
        static constexpr uint32_t sram_end_ = sram_start + 0x80000;
        static constexpr uint32_t qspi_start = 0x90040000U;

        // TODO -- this is a bit too large:
        static constexpr uint32_t qspi_end_ = qspi_start + 0x800000;

    private:

        Result Deinit();
        uint32_t FillTargetMemory();
        void SosLed();
              
        DaisySeed* hw_;
        
};

// Global dfu handle
Bootloader::Impl dfu_impl;
uint8_t DSY_QSPI_BSS qspi_buffer[PROGRAM_SPACE];
uint8_t DSY_SRAM_EXEC sram_program[SRAM_SPACE];
uint8_t DSY_ITCMRAM_EXEC itcmram_program[ITCMRAM_SPACE];

Bootloader::Result Bootloader::Impl::Init(DaisySeed* seed)
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

Bootloader::Result Bootloader::Impl::Deinit()
{
    if (USBD_DeInit(&hUsbDeviceFS) != USBD_OK)
        return Result::ERR;
    HAL_PWREx_DisableUSBVoltageDetector();

    __DSB();

    // TODO -- create mechanism to ensure the
    // deinit happens after USB disconnect so
    // the disconnect can happen without hanging
    System::Delay(50);

    dfu.Deinit();

    hw_->Deinit();

    return Result::OK;
}

void Bootloader::Impl::SosLed()
{
    // If we get here, the program should block until given a manual reset
    uint8_t delays[] = {
        1, 1,
        1, 1,
        1, 2,
        3, 1,
        3, 1,
        3, 2,
        1, 1,
        1, 1,
        1, 5,
    };
    bool led = true;

    for (;;) 
    {
        for (unsigned int i = 0; i < sizeof(delays) / sizeof(delays[0]); i++) 
        {
            hw_->SetLed(led);
            led = !led;
            hw_->DelayMs(delays[i] * 100);
        }
    }
}

uint32_t Bootloader::Impl::FillTargetMemory()
{
    uint32_t entry_address = *(uint32_t*) (qspi_buffer + 4);

    if (entry_address >= sram_start_ && entry_address < sram_end_)
    {
        for (size_t i = 0; i < SRAM_SPACE; i++)
        {
            sram_program[i] = qspi_buffer[i];
        }
        hw_->qspi.Deinit();
        return sram_start_;
    }
    else if (entry_address >= qspi_start_ && entry_address < qspi_end_)
    {
        // WARNING -- this will need to change with multi-programs 
        // (should be the beginning of the program, not the memory)
        return qspi_start_;
    }
    else
    {
        // If we got here, then the stack address is valid, but the 
        // entry point is not, meaning the user should know their
        // build isn't going to work
        SosLed();
    }
    return 0; // to ward off any compiler complaints
}

void _Noreturn Bootloader::Impl::LoadProgram()
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
            SosLed();
        }
        return;
    }

    uint32_t program_start = FillTargetMemory();
    
    hw_->SetLed(false);
    Deinit();

    // disable interupts NOTE -- These two seem to cause
    // errors for the target application
    // __set_PRIMASK(1);
    // __disable_irq();
    RCC->CIER = 0x00000000;
    
    typedef volatile void (*EntryPoint)(void);

    volatile uint32_t application_address = *(__IO uint32_t*)(program_start + 4);
    EntryPoint application = (EntryPoint)(application_address);
    SCB->VTOR = program_start;
    __set_MSP(*(__IO uint32_t*)program_start);
    application();
}

/////////////////////////////////////////////////
// Bootloader::Impl -> DFUHandle                 
/////////////////////////////////////////////////

Bootloader::Result Bootloader::Init(DaisySeed* seed)
{
    state_ = State::WAITING_ON_TIMEOUT;
    timeout_start_ = System::GetNow();

    dsy_gpio_pin button{dsy_gpio_port::DSY_GPIOG, 3};
    boot_button_.Init(button, 1000, Switch::TYPE_MOMENTARY, Switch::POLARITY_NORMAL, Switch::PULL_NONE);

    pwm_tick_ = timeout_start_;
    angle_ = 0;

    hw_ = seed;
    pimpl_ = &dfu_impl;
    return pimpl_->Init(seed);
}

void Bootloader::AwaitDFU()
{
    dfu.PollJump();
    SineLed();
}

void Bootloader::SineLed()
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

void Bootloader::HappyBlink()
{
    unsigned int time = 50;
    for (int i = 0; i < 2; i++)
    {
        hw_->SetLed(true);
        System::Delay(time);
        hw_->SetLed(false);
        System::Delay(time);
    }
}