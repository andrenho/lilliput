#ifndef DEBUGGER_KEYBOARD_HH_
#define DEBUGGER_KEYBOARD_HH_

#include "debuggerscreen.hh"
#include "luisavm.hh"

namespace luisavm {

class DebuggerKeyboard : public DebuggerScreen {
public:
    DebuggerKeyboard(LuisaVM& comp, Video& video) 
        : DebuggerScreen(video), _comp(comp) {}

    void Keypressed(Keyboard::KeyPress const& kp) override;
    void Update() override;

private:
    string KeyDescription(Keyboard::KeyPress const& kp) const;

    LuisaVM& _comp;
    bool _waiting_press = false;
    KeyState _waiting_state = PRESSED;
};

}   // namespace luisavm

#endif
