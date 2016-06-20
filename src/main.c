#include <stdio.h>
#include <syslog.h>

#include "config.h"
#include "video.h"
#include "memory.h"

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
    config_log(config);

    // initialize things
    memory_init(config);

    if(config->test_only) {
        memory_test();
    } else {
        // main loop
        video_init(config);
        while(video_active()) {
            video_doevents();
            video_draw();
        }
        video_destroy();
    }

    // free everything
    memory_destroy();
    config_free(config);
    closelog();
    return 0;
}
