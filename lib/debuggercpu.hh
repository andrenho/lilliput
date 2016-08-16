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
    void DrawInstructions() const;
    uint8_t InstructionSize(uint32_t addr) const;
    string Instruction(uint32_t addr) const;

    class LuisaVM& _comp;
    static const vector<string> _regs, _flags;
    uint32_t _code_start = 0x0,
             _top_addr = 0x0;
};

}  // namespace luisavm

#endif
