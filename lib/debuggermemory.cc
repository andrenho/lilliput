#include "debuggermemory.hh"

#include <cctype>

namespace luisavm {

DebuggerMemory::DebuggerMemory(LuisaVM& comp, Video& video)
    : DebuggerScreen(video), _comp(comp)
{
}


void DebuggerMemory::Keypressed(Keyboard::KeyPress const& kp)
{
    auto ph_size = _comp.PhysicalMemory().size();

    if(_ask_goto) {
        if(isxdigit(kp.key) != 0 && _goto_temp.size() < 8) {
            _goto_temp += toupper(kp.key);
        }
        switch(kp.key) {
            case ESC:
                _ask_goto = false;
                break;
            case ENTER:
                _top_addr = static_cast<uint32_t>(stoll(_goto_temp, nullptr, 16));
                _top_addr /= 8; _top_addr *= 8;
                if(_top_addr > ph_size) {
                    _top_addr = ph_size - 0xB0;
                }
                _ask_goto = false;
                break;
            case BACKSPACE:
                if(!_goto_temp.empty()) {
                    _goto_temp = _goto_temp.substr(0, _goto_temp.size()-1);
                }
                break;
        }
    } else {
        switch(kp.key) {
            case DOWN:
                if(_top_addr < ph_size - 0xB0) {
                    _top_addr += 0x8;
                }
                break;
            case UP:
                if(_top_addr > 0) {
                    _top_addr -= 0x8;
                }
                break;
            case PGDOWN:
                if(_top_addr > (ph_size - 0xB0 * 2)) {
                    _top_addr = ph_size - 0xB0;
                } else {
                    _top_addr += 0xB0;
                }
                break;
            case PGUP:
                if(_top_addr > 0xB0) {
                    _top_addr -= 0xB0;
                } else if(_top_addr > 0x0) {
                    _top_addr = 0x0;
                }
                break;
            case HOME:
                _top_addr = 0x0;
                break;
            case END:
                _top_addr = ph_size - 0xB0;
                break;
            case 'g':
                _goto_temp = "";
                _ask_goto = true;
                break;
        }
    }

    dirty = true;
}


void DebuggerMemory::Update()
{
    _video.ClearScreen(0);

    _video.Print(0, 0, 10, 0, "Memory");
    _video.Print(40, 0, 10, 8, "[F1]"); _video.Print(45, 0, 10, 0, "- help");

    _video.DrawBox(1, 1, 50, 24, 10, 0, true, false);

    // addresses
    auto ph_size = _comp.PhysicalMemory().size();
    for(uint32_t i=0; i<22; ++i) {
        uint32_t addr = _top_addr + (i*0x8);
        _video.Printf(3, i+2, 10, 0, "%08X:", addr);
        for(int j=0; j<8; ++j) {
            if(addr+j < ph_size) {
                uint8_t data = _comp.Get(addr+j);
                _video.Printf(15 + (j*3), i+2, 10, 0, "%02X", data);
                string printable = (data >= 32 && data < 127) ? string(1, static_cast<char>(data)) : ".";
                _video.Print(41 + j, i+2, 10, 0, printable);
            }
        }
    }
    
    if(_ask_goto) {
        _video.Print(0, 25, 10, 0, "Go to:");
        _video.Print(9, 25, 10, 8, "        ");
        _video.Printf(7, 25, 10, 8, "0x%s%c", _goto_temp.c_str(), 255);
    } else {
        _video.Print(0, 25, 10, 8, "[G]"); _video.Print(4, 25, 10, 0, "- go to");
    }

    _video.UpdateScreen();
}

}  // namespace luisavm
