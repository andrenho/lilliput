#ifndef DEBUGGER_HH_
#define DEBUGGER_HH_

#include "device.hh"
#include "video.hh"

namespace luisavm {

class Debugger : public Device {
public:
    Debugger(Video& video);

private:
    Video& video;
};

}  // namespace luisavm

#endif
