#include "luisavm.h"

#ifdef __WIN32

#include <stdarg.h>
#include <stdio.h>

void syslog(int priority, const char* fmt, ...)
{
    (void) priority;
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    fflush(stderr);
    va_end(ap);
}

void 
lvm_debuglog(bool active)
{
    (void) active;
}

#else

#include <syslog.h>

__attribute__((constructor)) static void 
init()
{
    openlog("luisavm", LOG_CONS | LOG_PERROR | LOG_PID, LOG_LOCAL1);
    setlogmask(LOG_UPTO(LOG_NOTICE));
}


void 
lvm_debuglog(bool active)
{
    if(active) {
        setlogmask(LOG_UPTO(LOG_DEBUG));
        syslog(LOG_DEBUG, "Debugging level log activated.");
    } else {
        setlogmask(LOG_UPTO(LOG_NOTICE));
    }
}


__attribute__((destructor)) static void 
fini()
{
    closelog();
}

#endif
