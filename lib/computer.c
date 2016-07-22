#include "luisavm.h"

#include <stdlib.h>

typedef struct LVM_Computer {
    uint8_t* physical_memory;
    uint32_t physical_memory_size;
} LVM_Computer;


LVM_Computer* lvm_computercreate(uint32_t physical_memory_size)
{
    LVM_Computer* comp = calloc(1, sizeof(LVM_Computer));
    comp->physical_memory = calloc(physical_memory_size, 1);
    comp->physical_memory_size = physical_memory_size;
    return comp;
}


void lvm_computerdestroy(LVM_Computer* comp)
{
    free(comp->physical_memory);
    free(comp);
}
