#include "video.hh"

#include <array>

#include "font.xbm"

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


void 
Video::DrawChar(char c, uint16_t x, uint16_t y, uint8_t fg, uint8_t bg) const
{
    if(x >= COLUMNS || y >= LINES) {
        return;
    }
    cb.draw_sprite(_char_bg[bg], (x * CHAR_W), (y * CHAR_H));
    cb.draw_sprite(LoadCharSprite(c, fg), (x * CHAR_W), (y * CHAR_H));
}


uint32_t
Video::LoadCharSprite(char c, uint8_t fg) const
{
    uint32_t key = (fg << 8) | c;
    auto it = _char_sprite.find(key);
    if(it != _char_sprite.end()) {
        return it->second;
    }

    array<uint8_t, CHAR_W * CHAR_H> data;
    size_t sx = (size_t)((static_cast<uint8_t>(c) / 16) * CHAR_W),
           sy = (size_t)((static_cast<uint8_t>(c) % 16) * CHAR_H);
    int i = 0;
    for(size_t y = sy; y < (sy + CHAR_H); ++y) {
        for(size_t x = sx; x < (sx + CHAR_W); ++x) {
            size_t f = x + (y * font_width);
            uint8_t bit = ((font_bits[f/8]) >> (f % 8)) & 1;
            data[i++] = (bit ? fg : TRANSPARENT);
        }
    }

    uint32_t idx = cb.upload_sprite(CHAR_W, CHAR_H, &data[0]);
    _char_sprite[key] = idx;
    return idx;
}

int 
Video::Print(uint16_t x, uint16_t y, uint8_t fg, uint8_t bg, string const& str) const
{
    int count = 0;
    for(char c: str) {
        DrawChar(c, x++, y, fg, bg);
        if(x == COLUMNS) {
            break;
        }
        ++count;
    }
    return count;
}


}  // namespace luisavm
