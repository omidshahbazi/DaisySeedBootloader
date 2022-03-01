#ifndef DSY_DFU
#define DSY_DFU

#include <cstdint>
#include "daisy_seed.h"

namespace daisy
{
/** @addtogroup serial
@{
*/

/** 
   @author Gabriel Ball
   @date 14 July, 2021

   @brief Presents a DFU device and endpoints to the USB host,
   and downloads an application if initiated within 5 seconds.
   Jumps to existing program after if present. Breathing LED indicates
   idle bootloader. An SOS pattern indicates the application is not
   correctly configured to run from the bootloader.
*/
class DFUHandle
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

    /** Initializes the USB drivers and starts timeout.
     * 
     *  \param seed Pointer to initialized seed hardware class
     */
    Result Init(DaisySeed* seed);

    /** Waits for the appropriate condition, then
     *  loads the target application and jumps to it.
     *  If no program is present, a DFU event is awaited.
     *  Attempts a jump immediately upon DFU completion.
     * 
     *  \returns True if the bootloader should jump
     */
    bool   PollJump(bool delay_timeout = false);

    void ResetPoll() { timeout_start_ = System::GetNow(); }

    /** Deinitializes DFU-related peripherals
     * 
     */
    Result DeInit();

    /** Retrieves the current state.
     * 
     */
    State  GetState() { return state_; }
    bool GetDfuComplete();

    DFUHandle() : pimpl_(nullptr) {}
    DFUHandle(const DFUHandle& other) = default;
    DFUHandle& operator=(const DFUHandle& other) = default;

    class Impl; /**< & */

  private:

    void HappyBlink();

    DaisySeed* hw_;
    Impl* pimpl_;

    Switch boot_button_;

    static constexpr uint32_t timeout_ = 2500; // 2.5 seconds
    uint32_t timeout_start_;
    State state_;

    static constexpr uint32_t sine_fid_ = 10;
    static constexpr uint32_t sine_hz_ = 1.f;
    float angle_;
    uint32_t pwm_tick_;

};

/** @} */

} // namespace daisy

#endif // DSY_DFU
