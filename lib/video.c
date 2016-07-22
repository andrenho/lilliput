#include "luisavm.h"

#include <string.h>
#include <syslog.h>

#include "device.h"
#include "font.xbm"

#define CHAR_W 6
#define CHAR_H 9
#define TRANSPARENT 0xFF

typedef struct Video {
    VideoCallbacks cb;
    uint32_t char_sprite[16][256];  // TODO - too wasteful, use map
    uint32_t char_bg[16];
} Video;

extern uint32_t default_palette[255];

// 
// CONSTRUCTORS
//

Video*
video_init(VideoCallbacks cbs)
{
    Video* video = calloc(sizeof(Video), 1);
    video->cb = cbs;

    // initialize palette
    for(uint8_t i=0; i<255; ++i) {
        video->cb.setpal(i, 
                (uint8_t)(default_palette[i] >> 16),
                (uint8_t)((default_palette[i] >> 8) & 0xFF),
                (uint8_t)(default_palette[i] & 0xFF));
    }
    video->cb.change_border_color(0);
    video->cb.clrscr(0);

    // create backgrounds
    uint8_t bg[CHAR_W * CHAR_H] = { 0 };
    for(int i=0; i<16; ++i) {
        memset(bg, i, CHAR_W * CHAR_H);
        video->char_bg[i] = video->cb.upload_sprite(CHAR_W, CHAR_H, bg);
    }

    syslog(LOG_DEBUG, "Video created.");

    return video;
}


static void
video_free(void* ptr)
{
    Video* video = (Video*)ptr;
    free(video);
}


LVM_Device* video_dev_init(VideoCallbacks cbs)
{
    return device_init(video_init(cbs), NULL, NULL, NULL, video_free, DEV_VIDEO, 0);
}


//
// CHAR MANAGEMENT
//

static uint32_t
load_char_sprite(Video* video, uint8_t c, uint8_t fg)
{
    if(video->char_sprite[fg][c] != 0) {
        return video->char_sprite[fg][c];
    }

    uint8_t data[CHAR_W * CHAR_H] = { 0 };

    size_t sx = (size_t)((c / 16) * CHAR_W),
           sy = (size_t)((c % 16) * CHAR_H);
    int i = 0;
    for(size_t y = sy; y < (sy + CHAR_H); ++y) {
        for(size_t x = sx; x < (sx + CHAR_W); ++x) {
            size_t f = x + (y * font_width);
            uint8_t bit = ((font_bits[f/8]) >> (f % 8)) & 1;
            data[i++] = (bit ? fg : TRANSPARENT);
        }
    }

    uint32_t idx = video->cb.upload_sprite(CHAR_W, CHAR_H, data);
    video->char_sprite[fg][c] = idx;
    return idx;
}


void
video_draw_char(Video* video, uint8_t c, uint16_t x, uint16_t y, uint8_t fg, uint8_t bg)
{
    video->cb.draw_sprite(video->char_bg[bg], (uint16_t)(x * CHAR_W), (uint16_t)(y * CHAR_H));
    video->cb.draw_sprite(load_char_sprite(video, c, fg), (uint16_t)(x * CHAR_W), (uint16_t)(y * CHAR_H));
}
