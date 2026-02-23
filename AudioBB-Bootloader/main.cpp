#include <cstdint>
#include "bootloader.h"
#include "AudioBB.h"

using namespace daisy;

static AudioBB hw;
static Bootloader boot;

void AudioCallback(AudioHandle::InputBuffer  in,
				   AudioHandle::OutputBuffer out,
				   size_t                    size)
{
	for (size_t i = 0; i < size; i++)
	{
		out[0][i] = out[1][i] = 0;
	}

	boot.AudioProcess(in, out, size);
}

void DeInitCallback(void* context) {
  AudioBB *hardware = reinterpret_cast<AudioBB*>(context);
  if (hardware) {
    hardware->DeInit();
  }
}

int main(void) {

  // AudioBB QSPI Pinout..
  // (easiest way to hack this in atm without
  // the config data being available prior to init..)
  QSPIHandle::Config qspi_config;
  qspi_config.pin_config.io0 = dsy_pin(DSY_GPIOF, 8);
  qspi_config.pin_config.io1 = dsy_pin(DSY_GPIOF, 9);
  qspi_config.pin_config.io2 = dsy_pin(DSY_GPIOE, 2);
  qspi_config.pin_config.io3 = dsy_pin(DSY_GPIOF, 6);
  qspi_config.pin_config.clk = dsy_pin(DSY_GPIOF, 10);
  qspi_config.pin_config.ncs = dsy_pin(DSY_GPIOG, 6);

  // I'm not 100% sure this works prior to hw Init ... but I think it should..
  // I'd like to "stay" in the bootloader manually prior to attempting to jump
  // if this button is held..
  const Pin kButtonPin = Pin(PORTD, 7);
  GPIO btn;
  btn.Init(kButtonPin, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  const bool stay_in_bootloader = !btn.Read();
  uint32_t timeout_ms = 0;
  if (stay_in_bootloader) {
    timeout_ms = UINT32_MAX;
  } else {
	  timeout_ms = startup_process(&qspi_config);
  }

	hw.Init(true, true);

	SCB_DisableDCache();

  // This LED will _eventually_ work since it's on rev11, but there is no led on <=rev10
	boot.Init(hw.qspi, Pin(PORTC, 7), kButtonPin, timeout_ms, DeInitCallback, (void*)&hw);

	hw.StartAudio(AudioCallback);

	boot.IoInit();

	while(1)
	{
		boot.LoopProcess();
	}
}
