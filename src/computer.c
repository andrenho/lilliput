#include "computer.h"

#include <stdlib.h>
#include <syslog.h>

#include "video.h"
#include "memory.h"
#include "rom.h"
#include "cpu.h"

static Config* config = NULL;
static bool with_video = true;

void
computer_init(Config* config_, bool with_video_)
{
    config = config_;
    with_video = with_video_;
    memory_init(config);
    if(config->rom_file) {
        rom_init(config->rom_file);
        memory_addmap(0xF8001000, rom_size(), rom_get, NULL);
    }
    cpu_init();
    if(with_video) {
        video_init(config);
    }
}


void
computer_destroy()
{
    if(with_video) {
        video_destroy();
    }
    cpu_destroy();
    if(config->rom_file) {
        rom_destroy();
    }
    memory_destroy();
}


bool
computer_active()
{
    return video_active();
}


void
computer_step()
{
    cpu_step();
}


void
computer_videoupdate()
{
    video_doevents();
    video_draw();
}


void
computer_reset()
{
    memory_reset();
    cpu_reset();
    if(with_video) {
        video_reset();
    }
    syslog(LOG_DEBUG, "Computer reset.");
}
