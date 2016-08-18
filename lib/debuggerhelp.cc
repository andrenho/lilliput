#include "debuggerhelp.hh"

namespace luisavm {

void DebuggerHelp::Update() {
    _video.Print(13, 0, 10, 0, "*** KEYBOARD COMMANDS ***");
    _video.Print(1, 2, 10, 8, "F1");
    _video.Print(4, 2, 10, 0, "- this help");
    _video.Print(1, 4, 10, 8, "F2");
    _video.Print(4, 4, 10, 0, "- current screen");
    _video.Print(1, 6, 10, 8, "F3");
    _video.Print(4, 6, 10, 0, "- CPU (source code)");
    _video.Print(1, 8, 10, 8, "F4");
    _video.Print(4, 8, 10, 0, "- CPU (disassembly)");
    _video.Print(1, 10, 10, 8, "F5");
    _video.Print(4, 10, 10, 0, "- Memory");
    _video.Print(1, 12, 10, 8, "F6");
    _video.Print(4, 12, 10, 0, "- Keyboard");
    _video.Print(1, 14, 10, 8, "F7");
    _video.Print(4, 14, 10, 0, "- Video");
    _video.Print(1, 16, 10, 8, "F8");
    _video.Print(4, 16, 10, 0, "- Timers");
    _video.UpdateScreen();
}

}  // namespace luisavm
