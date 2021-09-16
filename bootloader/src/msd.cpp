
#include "msd.h"
#include "usb_host.h"
#include "fatfs.h"

using namespace daisy;

class MSDHandle::Impl {
  public:
    Impl() {}
    ~Impl() {}

    Result Init(DaisySeed& seed);
    Result Deinit();

    void Process();

  private:

    DaisySeed* hw_;
        
};

// Global dfu handle
MSDHandle::Impl msd_impl;

MSDHandle::Result MSDHandle::Impl::Init(DaisySeed& seed)
{
  hw_ = &seed; 
  MX_USB_HOST_Init();
  MX_FATFS_Init();
  return Result::OK;
}

MSDHandle::Result MSDHandle::Impl::Deinit()
{
  
}

void MSDHandle::Impl::Process()
{
    MX_USB_HOST_Process();
}

// MSDHandle -> Impl

MSDHandle::Result MSDHandle::Init(DaisySeed& seed)
{
    return pimpl_->Init(seed);
}