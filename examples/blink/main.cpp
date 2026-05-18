#include "daisy_seed.h"

using namespace daisy;

DaisySeed hardware;

int main(void) {
  hardware.Init();
  bool led_state = true;

  for (;;) {
    hardware.SetLed(led_state);
    led_state = !led_state;
    hardware.DelayMs(500);
  }
}
