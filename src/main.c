#include <stdio.h>
#include <syslog.h>

#include "config.h"
#include "video.h"
#include "memory.h"
#include "rom.h"
#include "cpu.h"
#include "debugger.h"

int 
main(int argc, char** argv)
{
    // open syslog
#ifdef DEBUG
    setlogmask(LOG_UPTO(LOG_DEBUG));
#else
    setlogmask(LOG_UPTO(LOG_NOTICE));
#endif
    openlog("lilliput", LOG_CONS | LOG_PERROR | LOG_PID, LOG_LOCAL1);

    // read config file
    Config* config = config_init(argc, argv);
    if(config->quiet)
        setlogmask(LOG_UPTO(LOG_ERR));
    config_log(config);

    // initialize things
    memory_init(config);
    if(config->rom_file) {
        rom_init(config->rom_file);
        memory_addmap(0xF8001000, rom_size(), rom_get, NULL);
    }
    cpu_init();
    video_init(config);
    if(config->debugger) {
        debugger_init();
    }

    // main loop
    while(video_active()) {
        video_doevents();
        video_draw();
        if(config->debugger) {
            debugger_serve();
        }
    }

    // free everything
    if(config->debugger) {
        debugger_destroy();
    }
    video_destroy();
    cpu_destroy();
    if(config->rom_file) {
        rom_destroy();
    }
    memory_destroy();
    config_free(config);
    closelog();
    return 0;
}
