#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

struct State
{
  uint32_t boot_cnt;

  State() : boot_cnt(0) {}

  bool operator==(const State rhs)
  {
    return boot_cnt == rhs.boot_cnt;
  }

  bool operator!=(const State rhs)
  {
    return !operator==(rhs);
  }
};

DaisySeed hardware;
Oscillator DSY_SDRAM_BSS osc;
PersistentStorage<State> storage(hardware.qspi);

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
  for (size_t i = 0; i < size; i++)
  {
    out[0][i] = in[0][i];
    out[1][i] = osc.Process();
  }
}

int main(void)
{
  bool led_state;
  led_state = true;

  hardware.Init(true);
  hardware.StartLog(false);

  // Load, and Increment Boot Count
  State default_state;
  storage.Init(default_state);
  auto &state_data = storage.GetSettings();
  state_data.boot_cnt += 1;
  storage.Save();

  // Synth
  osc.Init(hardware.AudioSampleRate());
  osc.SetFreq(220.f);
  osc.SetAmp(1.f);
  hardware.StartAudio(AudioCallback);

  for (;;)
  {
    hardware.SetLed(led_state);
    hardware.Print("Boot Count: %d\t", state_data.boot_cnt);
    hardware.PrintLine("LED %s", led_state ? "ON" : "OFF");

    // Quick check to see if qspi is accessible
    // (expected value: 537001984 = 0x20020000)
    // uint32_t stack = *(__IO uint32_t *)0x90040000;
    led_state = !led_state;
    hardware.DelayMs(500);
  }
}
