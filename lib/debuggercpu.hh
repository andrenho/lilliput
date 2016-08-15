#ifndef DEBUGGER_CPU_HH_
#define DEBUGGER_CPU_HH_

#include <string>
#include <vector>
using namespace std;

#include "debuggerscreen.hh"

namespace luisavm {

class DebuggerCPU : public DebuggerScreen {
public:
    DebuggerCPU(class LuisaVM& comp, Video& video) : DebuggerScreen(video), _comp(comp) {}

    void Update() override;

private:
    class LuisaVM& _comp;

    static const vector<string> _regs, _flags;
};

}  // namespace luisavm

#endif
