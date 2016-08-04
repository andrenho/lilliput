#ifndef DEBUGGER_HH_
#define DEBUGGER_HH_

#include "luisavm.hh"
#include "debuggerscreen.hh"

#include <memory>
#include <vector>
using namespace std;

namespace luisavm {

class Debugger : public Device {
public:
    Debugger(LuisaVM& comp, Video& video);
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
