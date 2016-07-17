#include "luisavm.h"

#include <stdlib.h>
#include <syslog.h>

typedef struct LVM_CPU {
    LVM_Computer* computer;
} LVM_CPU;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-declarations"

LVM_CPU* 
lvm_createcpu(LVM_Computer* comp)
{
    LVM_CPU* cpu = calloc(1, sizeof(LVM_CPU));
    cpu->computer = comp;
    syslog(LOG_DEBUG, "New CPU created.");
    return cpu;
}


void
lvm_destroycpu(LVM_CPU* cpu)
{
    free(cpu);
    syslog(LOG_DEBUG, "CPU destroyed.");
}

#pragma GCC diagnostic pop
