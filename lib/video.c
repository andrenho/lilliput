#include "luisavm.h"

#include <syslog.h>

#include "device.h"

typedef struct Video {
    VideoCallbacks cb;
} Video;


Video*
video_init(VideoCallbacks cbs)
{
    Video* video = calloc(sizeof(Video), 1);
    video->cb = cbs;
    
    video->cb.setpal(0, 0, 0, 0);
    video->cb.setpal(1, 255, 255, 255);

    video->cb.clrscr(0);

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
    return device_init(video_init(cbs),
            NULL, NULL, NULL, video_free, DEV_VIDEO, 0);
}