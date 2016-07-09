#include <stdio.h>
#include <syslog.h>

#include "config.h"
#include "computer.h"
#include "tests.h"

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
        setlogmask(LOG_UPTO(LOG_NOTICE));
    config_log(config);

    // initialize things
    computer_init(config);

    // main loop
#ifdef DEBUG
    if(config->run_tests) {
        tests_run();
    } else 
#else
    {
        while(computer_active()) {
            // TODO (step)
            computer_videoupdate();
        }
    }
#endif

    // free everything
    computer_destroy();
    config_free(config);
    closelog();
    return 0;
}
