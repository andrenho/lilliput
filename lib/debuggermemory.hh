#ifndef DEBUGGER_MEMORY_HH_
#define DEBUGGER_MEMORY_HH_

#include "debuggerscreen.hh"
#include "luisavm.hh"

namespace luisavm {

class DebuggerMemory : public DebuggerScreen {
public:
    DebuggerMemory(LuisaVM& comp, Video& video);

    void Keypressed(Keyboard::KeyPress const& kp) override;
    void Update() override;

private:
    LuisaVM& _comp;
    uint32_t _top_addr = 0x0;
    bool     _ask_goto = false;
    string   _goto_temp = "";
};

}   // namespace luisavm

#endif
