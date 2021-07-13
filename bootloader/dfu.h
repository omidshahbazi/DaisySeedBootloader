#ifndef DSY_DFU
#define DSY_DFU

#include <cstdint>
#include "daisy_seed.h"

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
   Driver for DFU interactions. 
*/
class DFUHandle
{
  public:
    enum Result
    {
        OK = 0,
        ERR
    };

    /** Configuration structure for interfacing with QSPI Driver */
    struct Config
    {

    };

    Result Init(DaisySeed* seed);
    void   PollJump();

    DFUHandle() : pimpl_(nullptr) {}
    DFUHandle(const DFUHandle& other) = default;
    DFUHandle& operator=(const DFUHandle& other) = default;

    class Impl; /**< & */

  private:
    Impl* pimpl_;
};

/** @} */

} // namespace daisy

#endif // DSY_DFU
