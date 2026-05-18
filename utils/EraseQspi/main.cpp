/** Daisy Blink Example
 *
 *  This example blinks the Daisy Seed's LED
 *  once per second, as explained in the
 *  C++ Getting Started guide for the Daisy.
 */
#include <cstring>
#include "daisy_seed.h"
#include "dma.h"

using namespace daisy;

DaisySeed hardware;
bool blinkState;

/** Create a FIFO for receiving messages to echo out. */
FIFO<FixedCapStr<128>, 16> msg_fifo;

/** Callback that fires whenever new data is sent from the serial port */
void UsbCallback(uint8_t *buff, uint32_t *length)
{
    if (buff && length) /**< Check that our inputs are not null */
    {
        // Create a new string, and push it to the FIFO
        FixedCapStr<128> rx((const char *)buff, *length);
        msg_fifo.PushBack(rx);
    }
}

int main()
{
    hardware.Init();
    blinkState = false;

    // Start the log, and wait for connection
    hardware.StartLog(true);

    // Set USB callback
    hardware.usb_handle.SetReceiveCallback(UsbCallback,
                                           UsbHandle::UsbPeriph::FS_INTERNAL);

    /** Print an initial message once the connection occurs */
    hardware.PrintLine("Send 'ERASE' to erase QSPI!");

    bool erase_toggle = false;

    while (true)
    {
        while (!msg_fifo.IsEmpty())
        {
            auto msg = msg_fifo.PopFront();
            if (strstr(msg.Cstr(), "ERASE") != nullptr)
            {
                hardware.PrintLine("Message received, erasing. . . ");
                erase_toggle = true;
            }
            else
            {
                hardware.PrintLine("Unknown message received. Send 'ERASE' to erase.");
            }
        }

        /** Erase QSPI based on command */
        if (erase_toggle)
        {
            hardware.SetLed(true); /**< force led high when erasing */
            /** Trigger erase */
            // hardware.qspi.Erase(0, (1024 * 1024)); /**< just 1MB for now... */
            hardware.qspi.Erase(0, 0x800000);
            erase_toggle = false;
            hardware.PrintLine("done erasing");
            /** Verify erasure */
            hardware.PrintLine("verifying...");
            size_t chunk_size = 0x800000 / 32;
            auto percentage = 1.f / 32.f;
            for (size_t chnk = 0; chnk < 32; chnk++)
            {
                FixedCapStr<32> str = "Veryifying ";
                str.AppendFloat(percentage * chnk);
                str.Append("\% ... ");
                hardware.Print(str);
                uint8_t *buf = (uint8_t *)(0x90000000 + (chunk_size * chnk));
                dsy_dma_invalidate_cache_for_buffer(buf, chunk_size);
                bool good = true;
                for (size_t i = 0; i < chunk_size; i++)
                {
                    if (buf[i] != 0xff)
                        good = false;
                }
                hardware.Print("%s\n", good ? "Pass" : "Fail");
            }
        }

        /** Blink while idling */
        hardware.SetLed((daisy::System::GetNow() & 511) > 255);
    }
}
