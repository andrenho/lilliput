#ifndef DEBUGGER_VIDEO_HH_
#define DEBUGGER_VIDEO_HH_

#include "debuggerscreen.hh"

namespace luisavm {

class DebuggerVideo : public DebuggerScreen {
public:
    explicit DebuggerVideo(Video& video) : DebuggerScreen(video) {}

    void Update() override;
    void Keypressed(Keyboard::KeyPress const& kp) override;

private:
    size_t _current = 1;
};

}  // namespace luisavm

#endif
