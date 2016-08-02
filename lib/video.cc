#include "video.hh"

#include <array>

static const int CHAR_W = 6;
static const int CHAR_H = 9;
static const int TRANSPARENT = 0xFF;

namespace luisavm {

static uint32_t default_palette[255] = {
    0x0d0f11,  // dark black
    0xa54242,  // dark red
    0x8c9440,  // dark green
    0xde935f,  // dark yellow (orange)
    0x5f819d,  // dark blue
    0x85678f,  // dark purple
    0x5e8d87,  // dark turquoise
    0x707880,  // dark white  (light gray)
    0x373b41,  // light black (dark gray)
    0xcc6666,  // light red
    0x25d048,  // light green
    0xf0c674,  // light yellow
    0x81a2be,  // light blue
    0xb294bb,  // light purple
    0x8abeb7,  // light turquoise
    0xc5c8c6,  // light white
};


Video::Video(Callbacks const& cb) : cb(cb)
{
    // initialize palette
    for(uint8_t i=0; i<255; ++i) {
        cb.setpal(i, 
            (uint8_t)(default_palette[i] >> 16),
            (uint8_t)((default_palette[i] >> 8) & 0xFF),
            (uint8_t)(default_palette[i] & 0xFF));
    }

    // initialize screen
    cb.change_border_color(0);
    cb.clrscr(0);
    cb.update_screen();

    // upload character backgrounds
    array<uint8_t, CHAR_W * CHAR_H> bg;
    for(uint8_t i=0; i<16; ++i) {
        bg.fill(i);
        _char_bg[i] = cb.upload_sprite(CHAR_W, CHAR_H, &bg[0]);
    }
}

}  // namespace luisavm
