
#include "msd.h"

using namespace daisy;

class MSDHandle::Impl {
    public:
        Impl() {}
        ~Impl() {}

        Result Init(DaisySeed* seed);
        Result Deinit();

    private:

        DaisySeed* hw_;
        
};

// Global dfu handle
MSDHandle::Impl msd_impl;

MSDHandle::Result MSDHandle::Impl::Init(DaisySeed* seed)
{

}

MSDHandle::Result MSDHandle::Impl::Deinit()
{
  
}