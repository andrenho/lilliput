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

#ifdef DEBUG
    computer_init(config, false);
    if(config->run_tests) {
        tests_run();
    } else
#endif
    {
        computer_init(config, true);
        while(computer_active()) {
            // TODO (step)
            computer_videoupdate();
        }
    }

    // free everything
    computer_destroy();
    config_free(config);
    closelog();
    return 0;
}
