#include "debuggerkeyboard.hh"

namespace luisavm {

void DebuggerKeyboard::Keypressed(Keyboard::KeyPress const& kp)
{
    auto& q = _comp.keyboard().Queue;

    if(_waiting_press) {
        q.push_front({ kp.key, kp.mod, _waiting_state, true });
        dirty = true;
        Update();
        _waiting_press = false;
    } else {
        switch(kp.key) {
            case 'x':
                if(q.size() > 1) {   // the 'x' is also in the queue!
                    q.pop_front();
                }
                break;
            case 'p':
                _waiting_press = true;
                _waiting_state = PRESSED;
                break;
            case 'r':
                _waiting_press = true;
                _waiting_state = RELEASED;
                break;
        }
    }
}


void DebuggerKeyboard::Update()
{
    _video.ClearScreen(0);

    _video.Print(0, 0, 10, 0, "Keyboard");
    _video.Print(40, 0, 10, 8, "[F1]");
    _video.Print(45, 0, 10, 0, "- help");

    _video.Print( 0, 24, 10, 8, "[P]"); 
    _video.Print( 4, 24, 10, 0, "- add keypress event");
    _video.Print(27, 24, 10, 8, "[R]");
    _video.Print(31, 24, 10, 0, "- add keyrelease event");
    _video.Print( 0, 25, 10, 8, "[X]"); 
    _video.Print( 4, 25, 10, 0, "- unqueue event");

    if(_comp.keyboard().Queue.empty()) {
        _video.Print(2, 2, 10, 0, "The queue is empty.");
    } else {
        _video.Print(2, 2, 10, 0, "Front --->");
        uint16_t y = 2;
        for(auto const& kp: _comp.keyboard().Queue) {
            _video.Print(13, y++, 10, 0, KeyDescription(kp));
        }
    }

    _video.UpdateScreen();
}


string DebuggerKeyboard::KeyDescription(Keyboard::KeyPress const& kp) const
{
    return "TODO...";  // TODO
}


}
