#ifndef DEVICE_HH_
#define DEVICE_HH_

namespace luisavm {

class Device {
public:
    virtual void Step() {}
    virtual void Reset() {}
};

}  // namespace luisavm

#endif
