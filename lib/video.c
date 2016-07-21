#include "luisavm.h"

#include <string.h>
#include <syslog.h>

#include "device.h"
#include "font.xbm"

#define CHAR_WIDTH  6
#define CHAR_HEIGHT 9

typedef struct Video {
    VideoCallbacks cb;
    uint32_t char_sprite[16][256];  // TODO - too wasteful, use map
    uint32_t char_bg[16];
} Video;

static void video_draw_char(Video* video, char c, uint16_t x, uint16_t y, uint8_t fg, uint8_t bg);

// 
// CONSTRUCTORS
//

Video*
video_init(VideoCallbacks cbs)
{
    Video* video = calloc(sizeof(Video), 1);
    video->cb = cbs;
    
    // initialize palette (TODO)
    video->cb.setpal(0, 0, 0, 0);
    video->cb.setpal(1, 255, 255, 255);
    video->cb.setpal(2, 64, 0, 0);
    video->cb.clrscr(0);

    // create backgrounds
    uint8_t bg[CHAR_WIDTH * CHAR_HEIGHT] = { 0 };
    for(int i=0; i<16; ++i) {
        memset(bg, i, CHAR_WIDTH * CHAR_HEIGHT);
        video->char_bg[i] = video->cb.upload_sprite(CHAR_WIDTH, CHAR_HEIGHT, bg);
    }

    video_draw_char(video, 'A', 0, 0, 1, 2);

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

static void
video_draw_char(Video* video, char c, uint16_t x, uint16_t y, uint8_t fg, uint8_t bg)
{
    video->cb.draw_sprite(video->char_bg[bg], x * CHAR_WIDTH, y * CHAR_HEIGHT);
}
