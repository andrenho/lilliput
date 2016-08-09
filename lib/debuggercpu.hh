#ifndef DEBUGGER_CPU_HH_
#define DEBUGGER_CPU_HH_

#include "debuggerscreen.hh"

namespace luisavm {

class DebuggerCPU : public DebuggerScreen {
public:
    DebuggerCPU(class LuisaVM& comp, Video& video) : DebuggerScreen(video), _comp(comp) {}

    void Update() override;

private:
    class LuisaVM& _comp;
};

}  // namespace luisavm

#endif
