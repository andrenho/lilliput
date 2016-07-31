#include "luisavm.h"

#include <syslog.h>

#include "device.h"

typedef struct Video {
    VideoCallbacks cb;
} Video;

static void
video_free(LVM_Device* dev)
{
    Video* video = (Video*)dev->ptr;
    free(video);
    free(dev);
}

LVM_Device*
video_init(VideoCallbacks cbs)
{
    Video* video = calloc(sizeof(Video), 1);
    video->cb = cbs;

    LVM_Device* dev = calloc(sizeof(LVM_Device), 1);
    dev->ptr = video;
    dev->step = NULL;
    dev->get = NULL;
    dev->set = NULL;
    dev->free = video_free;
    dev->type = DEV_VIDEO;
    dev->sz = 0;

    syslog(LOG_DEBUG, "Video created.");

    return dev;
}
