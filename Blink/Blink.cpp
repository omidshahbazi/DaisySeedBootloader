#include "daisy_seed.h"
#include "daisysp.h"
#include "stm32h7xx_hal.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hardware;

int main(void) {
  // asm("bkpt 255");

  bool led_state;
  led_state = true;

  hardware.Init();

  for (;;) {

    hardware.SetLed(led_state);

    // Quick check to see if qspi is accessible
    // (expected value: 537001984 = 0x20020000)
    // uint32_t stack = *(__IO uint32_t *)0x90040000;
    led_state = !led_state;
    hardware.DelayMs(500);
  }
}
