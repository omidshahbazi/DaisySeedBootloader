#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed  seed;
static Oscillator osc;

uint8_t DSY_SDRAM_BSS array[8];
uint32_t DSY_QSPI_BSS program[8];

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    float sig;
    for(size_t i = 0; i < size; i += 2)
    {
        sig = osc.Process();

        // left out
        out[i] = sig;

        // right out
        out[i + 1] = sig;
    }
}

int main(void)
{
    // initialize seed hardware and oscillator daisysp module
    float sample_rate;
    seed.Configure();
    seed.Init();
    sample_rate = seed.AudioSampleRate();
    osc.Init(sample_rate);

    // Set parameters for oscillator
    osc.SetWaveform(osc.WAVE_SIN);
    osc.SetFreq(440);
    osc.SetAmp(0.5);

    for (int i = 0; i < 8; i++)
    {
        array[i] = i;
    }

    bool working = true;
    for (int i = 0; i < 8; i++)
    {
        if (array[i] != i)
            working = false; 
    }

    auto config = seed.qspi.GetConfig();
    config.mode = QSPIHandle::Config::Mode::DSY_MEMORY_MAPPED;
    seed.qspi.Init(config);

    if (program[0] != 0x20020000)
        working = false;

    // start callback
    seed.StartAudio(AudioCallback);

    bool ledstate = false;
    while(1) {

        if (working)
        {
            seed.SetLed(ledstate);
            ledstate = !ledstate;
            seed.DelayMs(250);
        }

    }
}
