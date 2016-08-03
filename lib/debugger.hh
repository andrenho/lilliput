#ifndef DEBUGGER_HH_
#define DEBUGGER_HH_

#include "device.hh"
#include "video.hh"
#include "keyboard.hh"
#include "debuggerscreen.hh"

#include <memory>
#include <vector>
using namespace std;

namespace luisavm {

class Debugger : public Device {
public:
    Debugger(Video& video, Keyboard& keyboard);
    void Step() override;

    bool Active = true;

private:
    Keyboard& _keyboard;
    vector<unique_ptr<DebuggerScreen>> _screens;
    uint8_t _selected = 0;

    void Keypressed(Keyboard::KeyPress const& kp);
};

}  // namespace luisavm

#endif
