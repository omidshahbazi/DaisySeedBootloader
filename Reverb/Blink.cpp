#include "daisy_seed.h"
#include "daisysp.h"
#include "stm32h7xx_hal.h"

// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
using namespace daisy;
using namespace daisysp;

// Declare a DaisySeed object called hardware
DaisySeed hardware;

/** Some objects for making some sound */
Oscillator osc;
AdEnv env;
Metro metro;
CpuLoadMeter meter;
float nn;

/** Reverb pointer for object that will be on the heap */
ReverbSc *verb;

float GetNewNote() {
  float t[] = {
      0.f, 4.f, 7.f, 9.f, 11.f, 12.f,
  };
  float val;
  int tidx = rand() % 6;
  val = t[tidx];
  return mtof(60.f + val);
}

void cb(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out,
        size_t size) {
  meter.OnBlockStart();
  for (size_t i = 0; i < size; i++) {
    float wetl, wetr;
    wetl = 0.f;
    wetr = 0.f;
    if (metro.Process()) {
      if (rand() % 4 == 0) {
        env.Trigger();
        nn = GetNewNote();
        osc.SetFreq(nn);
      }
    }
    float s = osc.Process() * env.Process();
    verb->Process(s, s, &wetl, &wetr);
    wetl += s;
    wetr += s;
    out[0][i] = wetl;
    out[1][i] = wetr;
  }
  meter.OnBlockEnd();
}

int main(void) {
  // asm("bkpt 255");
  // Declare a variable to store the state we want to set for the LED.
  bool led_state;
  led_state = true;
  nn = 12.f;

  // Configure and Initialize the Daisy Seed
  // These are separate to allow reconfiguration of any of the internal
  // components before initialization.
  hardware.Configure();
  hardware.Init(false);
  hardware.SetAudioBlockSize(4);
  meter.Init(hardware.AudioSampleRate(), hardware.AudioBlockSize());
  osc.Init(hardware.AudioSampleRate());
  osc.SetAmp(0.5f);
  metro.Init(4.f, hardware.AudioSampleRate());
  env.Init(hardware.AudioSampleRate());
  verb = new ReverbSc();
  verb->Init(hardware.AudioSampleRate());
  verb->SetFeedback(0.8f);
  verb->SetLpFreq(13000.f);

  env.SetTime(ADENV_SEG_ATTACK, 0.01f);
  env.SetTime(ADENV_SEG_DECAY, 0.40f);

  hardware.StartLog(false);
  System::Delay(100);

  hardware.StartAudio(cb);
  uint32_t now, ledt, usbt;
  now = ledt = usbt = System::GetNow();

  // Loop forever
  for (;;) {

    // Set the onboard LED
    hardware.SetLed(led_state);

    // Quick check to see if qspi is accessible
    // (expected value: 537001984 = 0x20020000)
    // uint32_t stack = *(__IO uint32_t *)0x90040000;

    // Toggle the LED state for the next time around.
    now = System::GetNow();
    if (now - ledt > 500) {
      led_state = !led_state;
      ledt = now;
    }
    if (now - usbt > 250) {
      hardware.PrintLine("CPU Time:");
      int tmin, tmax, tavg;
      tmax = (meter.GetMaxCpuLoad() * 100.f);
      tmin = (meter.GetMinCpuLoad() * 100.f);
      tavg = (meter.GetAvgCpuLoad() * 100.f);
      hardware.PrintLine("Min:\t:%d", tmin);
      hardware.PrintLine("Max:\t:%d", tmax);
      hardware.PrintLine("Avg:\t:%d", tavg);
      usbt = now;
    }
  }
}
