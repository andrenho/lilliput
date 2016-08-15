#include "debuggercpu.hh"

#include "luisavm.hh"

namespace luisavm {

void DebuggerCPU::Update() {
    _video.ClearScreen(0);
    
    _video.Print(0, 0, 10, 0, "CPU");
    _video.Print(40, 0, 10, 8, "[F1]"); _video.Print(45, 0, 10, 0, "- help");
    
    _video.DrawBox(0, 1, 50, 21, 10, 0, true, false);

    // registers
    for(size_t i=0; i<16; ++i) {
        _video.Printf((i%4)*13, 22+(i/4), 10, 0, "%s%s: %08X", (i<12) ? " " : "", _regs[i].c_str(), _comp.cpu().Register[i]);
    }

    // flags
    for(size_t i=0; i<6; ++i) {
        _video.Printf(i+47, 19, 10, 0, "%s", _flags[i].c_str());
        _video.Printf(i+47, 20, 10, 0, "%d", _comp.cpu().Flag(static_cast<Flag>(i)));
    }

    _video.UpdateScreen();
}

const vector<string> DebuggerCPU::_regs = {
    "A", "B", "C", "D", "E", "F", "G", "H", 
    "I", "J", "K", "L", "FP", "SP", "PC", "FL"
};

const vector<string> DebuggerCPU::_flags = {
    "Y", "V", "Z", "S", "G", "L",
};

}  // namespace luisavm
