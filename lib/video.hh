#ifndef VIDEO_HH_
#define VIDEO_HH_

#include <cstdint>
#include <array>
#include <functional>
#include <map>
using namespace std;

#include "device.hh"

namespace luisavm {

class Video : public Device {
    friend class DebuggerVideo;
public:
    static const int COLUMNS = 53,
                     LINES   = 26;

    struct Callbacks {
        function<void(uint8_t, uint8_t, uint8_t, uint8_t)> setpal;
        function<void(uint8_t)>                            clrscr;
        function<void(uint8_t)>                            change_border_color;
        function<uint32_t(uint16_t, uint16_t, uint8_t*)>   upload_sprite;
        function<void(uint32_t, uint16_t, uint16_t)>       draw_sprite;
        function<void(uint32_t, uint16_t&, uint16_t&)>     sprite_size;
        function<void()>                                   update_screen;
    };

    explicit Video(Callbacks const& cb);

    void DrawChar(char c, uint16_t x, uint16_t y, uint8_t fg, uint8_t bg) const;
    void DrawBox(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t fg, uint8_t bg, bool clear=true, bool shadow=false) const;
    int  Print(uint16_t x, uint16_t y, uint8_t fg, uint8_t bg, string const& str) const;

    template <typename ...Args>
    int Printf(uint16_t x, uint16_t y, uint8_t fg, uint8_t bg, Args&&... args) const {
        static char buf[512];
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
        snprintf(buf, sizeof buf, args...);
#pragma GCC diagnostic pop
        return Print(x, y, fg, bg, string(buf));
    }

    void ClearScreen(uint8_t color) const { cb.clrscr(color); }
    void UpdateScreen() const { cb.update_screen(); }

private:
    uint32_t LoadCharSprite(char c, uint8_t fg) const;

    Callbacks cb;
    array<uint8_t,16> _char_bg;
    mutable map<uint32_t, uint32_t> _char_sprite;
};

}  // namespace luisavm

#endif
