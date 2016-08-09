#include "debuggercpu.hh"

namespace luisavm {

void DebuggerCPU::Update() {
    _video.ClearScreen(0);
    _video.Print(13, 12, 10, 0, "CPU");
    _video.UpdateScreen();
}

}  // namespace luisavm
