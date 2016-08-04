#include "debuggermemory.hh"

namespace luisavm {

DebuggerMemory::DebuggerMemory(LuisaVM& comp, Video& video)
    : DebuggerScreen(video), _comp(comp)
{
}


void DebuggerMemory::Keypressed(Keyboard::KeyPress const& kp)
{
}


void DebuggerMemory::Update()
{
    _video.ClearScreen(0);

    _video.Print(0, 0, 10, 0, "Memory");
    _video.Print(40, 0, 10, 8, "[F1]"); _video.Print(45, 0, 10, 0, "- help");
    _video.Print(0, 25, 10, 8, "[G]"); _video.Print(4, 25, 10, 0, "- go to");

    _video.DrawBox(1, 1, 50, 24, 10, 0, true, false);

    // addresses
    for(uint32_t i=0; i<22; ++i) {
        uint32_t addr = _top_addr + (i*0x8);
        _video.Printf(3, i+2, 10, 0, "%08X:", addr);
        for(int j=0; j<8; ++j) {
            uint8_t data = _comp.Get(addr+j);
            _video.Printf(15 + (j*3), i+2, 10, 0, "%02X", data);
            string printable = (data >= 32 && data < 127) ? string(1, static_cast<char>(data)) : ".";
            _video.Print(41 + j, i+2, 10, 0, printable);
        }
    }

    _video.UpdateScreen();
}

}
