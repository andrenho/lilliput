#include <stdio.h>
#include <syslog.h>

#include "config.h"
#include "computer.h"
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
    computer_init(config);
    if(config->debugger) {
        debugger_init();
    }

    // main loop
    while(computer_active()) {
        computer_videoupdate();
        if(config->debugger) {
            debugger_serve();
        }
    }

    // free everything
    if(config->debugger) {
        debugger_destroy();
    }
    computer_destroy();
    config_free(config);
    closelog();
    return 0;
}
