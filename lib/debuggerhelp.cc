#include "debuggerhelp.hh"

namespace luisavm {

void DebuggerHelp::Update() {
    _video.ClearScreen(0);

    _video.Print(13, 0, 10, 0, "*** KEYBOARD COMMANDS ***");
    _video.Print(0, 2, 10, 8, "F1");
    _video.Print(3, 2, 10, 0, "- this help");
    _video.Print(0, 4, 10, 8, "F2");
    _video.Print(3, 4, 10, 0, "- current screen");
    _video.Print(0, 6, 10, 8, "F3");
    _video.Print(3, 6, 10, 0, "- CPU (source code)");
    _video.Print(0, 8, 10, 8, "F4");
    _video.Print(3, 8, 10, 0, "- CPU (disassembly)");
    _video.Print(0, 10, 10, 8, "F5");
    _video.Print(3, 10, 10, 0, "- Memory");
    _video.Print(0, 12, 10, 8, "F6");
    _video.Print(3, 12, 10, 0, "- Keyboard");
    _video.Print(0, 14, 10, 8, "F7");
    _video.Print(3, 14, 10, 0, "- Video");
    _video.Print(0, 16, 10, 8, "F8");
    _video.Print(3, 16, 10, 0, "- Timers");

    _video.Print(26, 2, 10, 8, "S");
    _video.Print(28, 2, 10, 0, "- step through");
    _video.Print(26, 4, 10, 8, "N");
    _video.Print(28, 4, 10, 0, "- next (skip subroutines)");
    _video.Print(26, 6, 10, 8, "C");
    _video.Print(28, 6, 10, 0, "- continue");
    _video.Print(26, 8, 10, 8, "G");
    _video.Print(28, 8, 10, 0, "- go to");
    _video.Print(26, 10, 10, 8, "B");
    _video.Print(28, 10, 10, 0, "- add breakpoint");

    _video.UpdateScreen();
}

}  // namespace luisavm
