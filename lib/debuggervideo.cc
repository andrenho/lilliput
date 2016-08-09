#include "debuggervideo.hh"

#include "video.hh"

namespace luisavm {

void DebuggerVideo::Update() 
{
    _video.ClearScreen(0);

    // text
    _video.Print(0, 0, 10, 0, "Width:  318 px (53 text columns)");
    _video.Print(0, 1, 10, 0, "Height: 234 px (26 text lines)");
    _video.Printf(21, 25, 10, 0, "<<< %06X >>>", _current);

    // draw selected sprite
    uint16_t w, h;
    _video.cb.sprite_size(_current, w, h);
    _video.cb.draw_sprite(_current, 318/2 - w/2, 234/2 - h/2);

    _video.UpdateScreen();
}

void DebuggerVideo::Keypressed(Keyboard::KeyPress const& kp)
{
    if(kp.key == RIGHT) {
        ++_current;
        dirty = true;
    } else if(kp.key == LEFT) {
        if(_current > 1) {
            --_current;
            dirty = true;
        }
    }
}


}  // namespace luisavm
