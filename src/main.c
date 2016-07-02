#include <stdio.h>
#include <syslog.h>

#include "config.h"

int 
main(int argc, char** argv)
{
#ifdef DEBUG
    setlogmask(LOG_UPTO(LOG_DEBUG));
#else
    setlogmask(LOG_UPTO(LOG_NOTICE));
#endif
    openlog("lilliput", LOG_CONS | LOG_PERROR | LOG_PID, LOG_LOCAL1);

    Config* config = config_init(argc, argv);
    config_log(config);

    config_free(config);

    closelog();
    return 0;
}
