
#include "msd.h"
#include "usb_host.h"
#include "fatfs.h"

using namespace daisy;

extern "C" 
{
  extern PCD_HandleTypeDef hpcd_USB_OTG_HS;
}

class MSDHandle::Impl {
  public:
    Impl() {}
    ~Impl() {}

    Result Init(DaisySeed& seed);
    Result Deinit();

    void Process();
    bool GetReady();

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
  return Result::OK;
}

void MSDHandle::Impl::Process()
{
  MX_USB_HOST_Process();
}

bool MSDHandle::Impl::GetReady()
{
  return (bool) GetMscReady();
}

// MSDHandle -> Impl

MSDHandle::Result MSDHandle::Init(DaisySeed& seed)
{
    return pimpl_->Init(seed);
}

bool MSDHandle::GetReady()
{
  return pimpl_->GetReady();
}

void MSDHandle::Process()
{
  pimpl_->Process();
}

// IRQ Handler
extern "C"
{
    void OTG_HS_EP1_OUT_IRQHandler(void)
    {
        HAL_PCD_IRQHandler(&hpcd_USB_OTG_HS);
    }

    void OTG_HS_EP1_IN_IRQHandler(void)
    {
        HAL_PCD_IRQHandler(&hpcd_USB_OTG_HS);
    }

    void OTG_HS_IRQHandler(void) 
    { 
        HAL_PCD_IRQHandler(&hpcd_USB_OTG_HS); 
    }
}