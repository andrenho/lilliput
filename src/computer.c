#include "computer.h"

#include <stdlib.h>
#include <syslog.h>

#include "video.h"
#include "memory.h"
#include "rom.h"
#include "cpu.h"

static Config* config = NULL;

void
computer_init(Config* config_)
{
    config = config_;
    memory_init(config);
    if(config->rom_file) {
        rom_init(config->rom_file);
        memory_addmap(0xF8001000, rom_size(), rom_get, NULL);
    }
    cpu_init();
    video_init(config);
}


void
computer_destroy()
{
    video_destroy();
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
    video_reset();
    syslog(LOG_DEBUG, "Computer reset.");
}
