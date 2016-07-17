#include "luisavm.h"

#include <stdlib.h>
#include <syslog.h>


extern LVM_CPU* lvm_createcpu(LVM_Computer* comp);
extern void lvm_destroycpu(LVM_CPU* cpu);

typedef struct LVM_Computer {
    uint8_t*  physical_memory;
    uint32_t  physical_memory_size;
    uint32_t  offset;
    LVM_CPU** cpu;
} LVM_Computer;


LVM_Computer*
lvm_computercreate(uint32_t physical_memory_size)
{
    LVM_Computer* comp = calloc(1, sizeof(LVM_Computer));
    comp->physical_memory = calloc(physical_memory_size, 1);
    comp->physical_memory_size = physical_memory_size;
    comp->cpu = calloc(1, sizeof(LVM_CPU*));
    syslog(LOG_DEBUG, "Computer created with %d kB of physical memory.", physical_memory_size / 1024);
    return comp;
}


void
lvm_computerdestroy(LVM_Computer* comp)
{
    size_t i = 0;
    while(comp->cpu[i]) {
        lvm_destroycpu(comp->cpu[i++]);
    }
    free(comp->physical_memory);
    free(comp);
    syslog(LOG_DEBUG, "Computer destroyed.");
}


uint8_t 
lvm_get(LVM_Computer* comp, uint32_t pos)
{
    pos += comp->offset;
    if(pos < comp->physical_memory_size) {
        return comp->physical_memory[pos];
    } else {
        return 0;
    }
}


void 
lvm_set(LVM_Computer* comp, uint32_t pos, uint8_t data)
{
    pos += comp->offset;
    if(pos < comp->physical_memory_size) {
        comp->physical_memory[pos] = data;
    }
}


uint16_t 
lvm_get16(LVM_Computer* comp, uint32_t pos)
{
    uint16_t b1 = lvm_get(comp, pos),
             b2 = lvm_get(comp, pos+1);
    return b1 | (b2 >> 8);
}


void
lvm_set16(LVM_Computer* comp, uint32_t pos, uint16_t data)
{
    lvm_set(comp, pos, (uint8_t)data);
    lvm_set(comp, pos+1, (uint8_t)(data >> 8));
}


uint32_t 
lvm_get32(LVM_Computer* comp, uint32_t pos)
{
    uint32_t b1 = lvm_get(comp, pos),
             b2 = lvm_get(comp, pos+1),
             b3 = lvm_get(comp, pos+2),
             b4 = lvm_get(comp, pos+3);
    return b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
}


void
lvm_set32(LVM_Computer* comp, uint32_t pos, uint32_t data)
{
    lvm_set(comp, pos, (uint8_t)data);
    lvm_set(comp, pos+1, (uint8_t)(data >> 8));
    lvm_set(comp, pos+2, (uint8_t)(data >> 16));
    lvm_set(comp, pos+3, (uint8_t)(data >> 24));
}


uint8_t*
lvm_physicalmemory(LVM_Computer* comp)
{
    return comp->physical_memory;
}


uint32_t
lvm_physicalmemorysz(LVM_Computer* comp)
{
    return comp->physical_memory_size;
}


void
lvm_setoffset(LVM_Computer* comp, uint32_t offset)
{
    if(offset >= 0xF0000000) {
        syslog(LOG_ERR, "Offset can't be higher than 0xF0000000");
        abort();
    }
    comp->offset = offset;
}


uint32_t
lvm_offset(LVM_Computer* comp)
{
    return comp->offset;
}


LVM_CPU* 
lvm_addcpu(LVM_Computer* comp)
{
    size_t i = 0;
    while(comp->cpu[i++]);
    
    // create space for one extra CPU
    comp->cpu = realloc(comp->cpu, sizeof(LVM_CPU*) * (i+1));
    comp->cpu[i] = NULL;

    // add CPU
    comp->cpu[i-1] = lvm_createcpu(comp);
    syslog(LOG_DEBUG, "New CPU added to computer.");
    return comp->cpu[i-1];
}


LVM_CPU* 
lvm_cpu(LVM_Computer* comp, size_t n)
{
    return comp->cpu[n];
}
