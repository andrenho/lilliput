#include <stdio.h>
#include <syslog.h>

#include "config.h"
#include "video.h"

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

    // initialize video
    video_init(config);

    // main loop
    while(video_active()) {
        video_doevents();
        video_draw();
    }

    // free everything
    video_destroy();
    config_free(config);
    closelog();
    return 0;
}
