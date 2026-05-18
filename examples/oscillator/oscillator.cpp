#include "daisy_seed.h"

using namespace daisy;

static DaisySeed hw;

static float render_osc() {
  static const float phs_inc = 440.f * (1.f / 48000.f);
  static float phs = 0.f;
  auto out = std::sin(phs * ((float)M_PI * 2.f));
  phs += phs_inc;
  if (phs > 1.f) {
    phs -= 1.f;
  }
  return out;
}

static void AudioCallback(AudioHandle::InputBuffer in,
                          AudioHandle::OutputBuffer out, size_t size) {
  for (size_t i = 0; i < size; i++) {
    out[0][i] = out[1][i] = render_osc();
  }
}

int main(void) {
  hw.Init(true);

  hw.StartAudio(AudioCallback);

  while (1) {
    // Blink quickly
    hw.SetLed((System::GetNow() & 255) < 127);
  }
}
