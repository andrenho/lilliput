#include "debuggerkeyboard.hh"

namespace luisavm {

void DebuggerKeyboard::Keypressed(Keyboard::KeyPress const& kp)
{
}


void DebuggerKeyboard::Update()
{
    _video.ClearScreen(0);

    _video.Print(0, 0, 10, 0, "Keyboard");
    _video.Print(40, 0, 10, 8, "[F1]"); _video.Print(45, 0, 10, 0, "- help");

    _video.Print(0, 25, 10, 8, "[P]"); _video.Print(4, 25, 10, 0, "- add keypress event");
    _video.Print(27, 25, 10, 8, "[R]"); _video.Print(31, 25, 10, 0, "- add keyrelease event");

    _video.UpdateScreen();
}


}
