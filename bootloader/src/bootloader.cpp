#include <cstdint>

#include "bootloader.h"
#include "daisy_seed.h"

#include "usbd_def.h"
#include "usbd_desc.h"
#include "usbd_dfu.h"
#include "usbd_dfu_if.h"
#include "system.h"

using namespace daisy;

uint8_t DSY_QSPI_BSS qspi_buffer[PROGRAM_SPACE];
uint8_t DSY_SRAM_EXEC sram_program[SRAM_SPACE];
uint8_t DSY_ITCMRAM_EXEC itcmram_program[ITCMRAM_SPACE];

Bootloader::Result Bootloader::Init(DaisySeed& seed)
{
    // dfu.Init(seed);
    hw_ = &seed;

    pwm_tick_ = System::GetNow();
    angle_ = 0;

    dfu_initialized_ = false;

    return Result::OK;
}

Bootloader::Result Bootloader::Deinit()
{
    dfu.Deinit();
    hw_->Deinit();
    return Result::OK;
}

void Bootloader::SosLed()
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

uint32_t Bootloader::FillTargetMemory()
{
    uint32_t entry_address = *(uint32_t*) (qspi_buffer + 4);
    auto mem = System::GetProgramMemory(entry_address);

    switch (mem)
    {
        case System::AXI_SRAM:
        {
            for (size_t i = 0; i < SRAM_SPACE; i++)
            {
                sram_program[i] = qspi_buffer[i];
            }
            hw_->qspi.Deinit();
            return System::sram_start;
        }
        case System::QSPI:
        {
            // WARNING -- this will need to change with multi-programs 
            // (should be the beginning of the program, not the memory)
            return System::qspi_start;
        }
        default:
        {
            // If we got here, then the stack address is valid, but the 
            // entry point is not, meaning the user should know their
            // build isn't going to work
            SosLed();
        }
    }

    return 0; // to ward off any compiler complaints
}

void _Noreturn Bootloader::LoadProgram()
{
    // The data caching can cause issues if we've recently
    // read QSPI and found no program in there, and then
    // actually write a program there and try to load it.
    SCB_InvalidateDCache();
    __DSB();
    
    // If the stack pointer isn't here, then either the
    // download failed or was invalid, or a program
    // has never been written to flash
    uint32_t* stack_ptr = (uint32_t*) qspi_buffer;
    if (*stack_ptr != System::expected_stack)
    {
        if (dfu.GetDfuComplete())
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

void Bootloader::AwaitDFU()
{
    if (!dfu_initialized_)
    {
        dfu_initialized_ = true;
        dfu.Init(hw_);
    }
    if (dfu.PollJump())
    {
        LoadProgram();
    }
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