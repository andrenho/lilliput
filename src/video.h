#ifndef VIDEO_H_
#define VIDEO_H_

#include <stdbool.h>

#include "config.h"

void video_init(Config* config);
void video_destroy();

bool video_active();
void video_doevents();
void video_draw();

#endif
