#include "debuggernotimplemented.hh"

namespace luisavm {

void DebuggerNotImplemented::Update() {
    _video.ClearScreen(0);
    _video.Print(13, 12, 10, 0, "*** NOT IMPLEMENTED :-( ***");
    _video.UpdateScreen();
}

}  // namespace luisavm
