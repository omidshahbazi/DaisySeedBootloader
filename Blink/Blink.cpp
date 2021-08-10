#include "daisy_seed.h"
#include "stm32h7xx_hal.h"

// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
using namespace daisy;

// Declare a DaisySeed object called hardware
DaisySeed hardware;

int main(void)
{
    // asm("bkpt 255");
    // Declare a variable to store the state we want to set for the LED.
    bool led_state;
    led_state = true;

    // Configure and Initialize the Daisy Seed
    // These are separate to allow reconfiguration of any of the internal
    // components before initialization.
    hardware.Configure();
    hardware.Init();

    // Loop forever
    for(;;)
    {

        // Set the onboard LED
        hardware.SetLed(led_state);

        // Quick check to see if qspi is accessible
        // (expected value: 537001984 = 0x20020000)
        uint32_t stack = *(__IO uint32_t*)0x90040000;

        // Toggle the LED state for the next time around.
        led_state = !led_state;

        
        // // Wait 500ms
        System::Delay(500);
        
    }
}
