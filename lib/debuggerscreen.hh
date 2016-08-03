#ifndef DEBUGGER_SCREEN_HH_
#define DEBUGGER_SCREEN_HH_

#include "video.hh"
#include "keyboard.hh"

namespace luisavm {

class DebuggerScreen {
public:
    DebuggerScreen(Video& video) : _video(video) {}

    virtual void Keypressed(Keyboard::KeyPress const& kp) { (void) kp; }

    virtual void Update() = 0;
    bool dirty = true;

protected:
    Video& _video;
};

}  // namespace luisavm

#endif
