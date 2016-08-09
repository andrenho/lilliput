#include "debuggerkeyboard.hh"

#include <sstream>

namespace luisavm {

void DebuggerKeyboard::Keypressed(Keyboard::KeyPress const& kp)
{
    auto& q = _comp.keyboard().Queue;

    if(_waiting_press) {
        q.push_front({ kp.key, kp.mod, _waiting_state });
        dirty = true;
        Update();
        _waiting_press = false;
    } else {
        switch(kp.key) {
            case 'x':
                if(!q.empty()) {
                    q.pop_front();
                }
                dirty = true;
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
    stringstream s;
    s << ((kp.state == PRESSED) ? "[PRESSED]" : "[RELEASED]") << " - ";
    if((kp.mod & CONTROL) != 0) { s << "Ctrl + "; }
    if((kp.mod & SHIFT) != 0) { s << "Shift + "; }
    if((kp.mod & ALT) != 0) { s << "Alt + "; }
    switch(kp.key) {
        case F1: s << "F1"; break; 
        case F2: s << "F2"; break; 
        case F3: s << "F3"; break; 
        case F4: s << "F4"; break; 
        case F5: s << "F5"; break; 
        case F6: s << "F6"; break; 
        case F7: s << "F7"; break; 
        case F8: s << "F8"; break; 
        case F9: s << "F9"; break; 
        case F10: s << "F10"; break; 
        case F11: s << "F11"; break; 
        case F12: s << "F12"; break;
        case INSERT: s << "INSERT"; break; 
        case HOME: s << "HOME"; break; 
        case DELETE: s << "DELETE"; break; 
        case END: s << "END"; break; 
        case PGUP: s << "PGUP"; break; 
        case PGDOWN: s << "PGDOWN"; break;
        case LEFT: s << "LEFT"; break; 
        case RIGHT: s << "RIGHT"; break; 
        case UP: s << "UP"; break; 
        case DOWN: s << "DOWN"; break;
        case ESC: s << "ESC"; break; 
        case TAB: s << "TAB"; break; 
        case BACKSPACE: s << "BACKSPACE"; break; 
        case ENTER: s << "ENTER"; break;
        default: s << static_cast<char>(kp.key);
    }

    return s.str();
}


}  // namespace luisavm
