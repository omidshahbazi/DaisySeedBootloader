#ifndef DSY_DFU
#define DSY_DFU /**< Macro */

#include <cstdint>
#include "per/qspi.h"

namespace daisy
{
/** @addtogroup serial
@{
*/

#define TEST_LEN 8192

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

    Result Init(QSPIHandle* qspi);

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
