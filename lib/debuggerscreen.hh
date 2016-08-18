#ifndef DEBUGGER_SCREEN_HH_
#define DEBUGGER_SCREEN_HH_

#include "video.hh"

namespace luisavm {

class DebuggerScreen {
public:
    DebuggerScreen(Video& video) : _video(video) {}

    virtual void Update() = 0;
    bool dirty = true;

protected:
    Video& _video;
};

}  // namespace luisavm

#endif
