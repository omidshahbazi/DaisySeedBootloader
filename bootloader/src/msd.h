#ifndef DSY_MSD
#define DSY_MSD

#include <cstdint>
#include "daisy_seed.h"

namespace daisy
{

/** 
   @author Gabriel Ball
   @date September 16, 2021

   @brief Presents a USB Mass Storage Device host interface
*/
class MSDHandle
{
  public:
    enum Result
    {
        OK = 0,
        ERR
    };

    /** Configuration structure for interfacing with MSD Driver */
    struct Config
    {

    };

    /** Initializes the USB drivers and starts timeout.
     * 
     *  \param seed Pointer to initialized seed hardware class
     */
    Result Init(DaisySeed& seed);

    /** Deinitializes MSD-related peripherals
     * 
     */
    Result Deinit();

    MSDHandle() : pimpl_(nullptr) {}
    MSDHandle(const MSDHandle& other) = default;
    MSDHandle& operator=(const MSDHandle& other) = default;

    class Impl; /**< & */

  private:

    DaisySeed* hw_;
    Impl* pimpl_;

    Switch boot_button_;

};

} // namespace daisy

#endif // DSY_MSD