#ifndef DSY_BOOTLOADER
#define DSY_BOOTLOADER

#include <cstdint>
#include "daisy_seed.h"
#include "dfu.h"

namespace daisy
{
/** @addtogroup serial
@{
*/

#define SRAM_SPACE    0x80000U
#define ITCMRAM_SPACE 0x10000U
#define PROGRAM_SPACE SRAM_SPACE + ITCMRAM_SPACE
#define EXEC_START    0x24000000U

#define DSY_SRAM_EXEC    __attribute__((section( ".sram_exec")))
#define DSY_ITCMRAM_EXEC __attribute__((section( ".itcmram_exec")))

/** 
   @author Gabriel Ball
   @date 14 July, 2021

   @brief Presents a DFU device and endpoints to the USB host,
   and downloads an application if initiated within 5 seconds.
   Jumps to existing program after if present. Breathing LED indicates
   idle bootloader. An SOS pattern indicates the application is not
   correctly configured to run from the bootloader.
*/
class Bootloader
{
  public:
    enum Result
    {
        OK = 0,
        ERR
    };

    /** Configuration structure for interfacing with DFU Driver */
    struct Config
    {

    };

    /** Describes the class state. 
     */
    enum State {
      WAITING_ON_TIMEOUT = 0,
      WAITING_ON_DFU
    };

    Bootloader() {}
    ~Bootloader() {}

    /** Initializes the USB drivers and starts timeout.
     * 
     *  \param seed Pointer to initialized seed hardware class
     */
    Result Init(DaisySeed& seed);

    /** Waits for the appropriate condition, then
     *  loads the target application and jumps to it.
     *  If no program is present, a DFU event is awaited.
     *  Attempts a jump immediately upon DFU completion.
     */
    void   AwaitDFU();

    /** Retrieves the current state.
     * 
     */
    State  GetState() { return state_; }

    Bootloader(const Bootloader& other) = default;
    Bootloader& operator=(const Bootloader& other) = default;

    void LoadProgram();

  private:


    void SineLed();
    void HappyBlink();

    Result Deinit();
    uint32_t FillTargetMemory();
    void SosLed();

    DaisySeed* hw_;

    DFUHandle dfu;

    // Switch boot_button_; // handled by the dfu class at the moment
    bool dfu_initialized_;

    static constexpr uint32_t timeout_ = 5000; // 5 seconds
    uint32_t timeout_start_;
    State state_;

    static constexpr uint32_t sine_fid_ = 10;
    static constexpr uint32_t sine_hz_ = 1.f;
    float angle_;
    uint32_t pwm_tick_;

};

/** @} */

} // namespace daisy

#endif // DSY_BOOTLOADER
