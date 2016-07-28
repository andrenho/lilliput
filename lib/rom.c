#include "luisavm.h"

#include <stdlib.h>
#include <syslog.h>

#include "device.h"

typedef struct ROM {
    uint32_t sz;
    uint8_t* data;
} ROM;


static void
rom_free(LVM_Device* dev)
{
    ROM* rom = (ROM*)dev->ptr;

    if(rom->data) {
        free(rom->data);
    }
    free(rom);
    free(dev);
}


static uint8_t
rom_get(void* ptr, uint32_t pos)
{
    ROM* rom = (ROM*)ptr;

    if(pos < rom->sz) {
        return rom->data[pos];
    } else {
        return 0;
    }
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-declarations"

LVM_Device*
rom_init(uint32_t sz, uint8_t* data_new_ownership)
{
    ROM* rom = calloc(sizeof(ROM), 1);
    rom->sz = sz;
    rom->data = data_new_ownership;

    LVM_Device* dev = calloc(sizeof(LVM_Device), 1);
    dev->ptr = rom;
    dev->step = NULL;
    dev->get = rom_get;
    dev->set = NULL;
    dev->free = rom_free;
    dev->type = DEV_ROM;
    dev->sz = sz;

    syslog(LOG_DEBUG, "A ROM device was created.");

    return dev;
}

#pragma GCC diagnostic pop
