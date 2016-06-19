#ifndef VIDEO_H_
#define VIDEO_H_

#include <stdbool.h>
#include <stdint.h>

#include "config.h"

// constructor / destructor
void video_init(Config* config);
void video_destroy();

// host commands
bool video_active();
void video_doevents();
void video_draw();

// guest commands
void video_clrscr(uint8_t idx);
void video_setbordercolor(uint8_t idx);
void video_drawpoint(uint16_t x, uint16_t y, uint8_t idx);

#endif
