#include "luisavm.h"

#include <stdlib.h>
#include <syslog.h>

#include "device.h"

typedef struct Debugger {
    bool active;
} Debugger;

Debugger*
debugger_init(bool active)
{
    Debugger* dbg = calloc(sizeof(Debugger), 1);
    dbg->active = active;

    syslog(LOG_DEBUG, "Debugger created.");

    return dbg;
}


static void
debugger_free(void* ptr)
{
    Debugger* dbg = (Debugger*)ptr;
    free(dbg);
}


LVM_Device*
debugger_dev_init(bool active)
{
    return device_init(debugger_init(active), 
            NULL, NULL, NULL, debugger_free, DEV_DEBUGGER, 0);
}
