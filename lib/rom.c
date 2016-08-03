#include "luisavm.h"

#include <stdlib.h>
#include <syslog.h>

#include "device.h"

typedef struct ROM {
    Device   device;
    uint32_t sz;
    uint8_t* data;
} ROM;


static uint8_t rom_get(Device* dev, uint32_t pos);
static void rom_free(Device* dev);


ROM*
rom_init(uint32_t sz, uint8_t* data_new_ownership)
{
    ROM* rom = calloc(sizeof(ROM), 1);
    
    rom->device.get = rom_get;
    rom->device.free = rom_free;
    rom->device.type = DEV_ROM;
    rom->device.sz = sz;

    rom->sz = sz;
    rom->data = data_new_ownership;

    syslog(LOG_DEBUG, "A ROM device was created.");

    return rom;
}


static void
rom_free(Device* dev)
{
    ROM* rom = (ROM*)dev;
    if(rom->data) {
        free(rom->data);
    }
    free(rom);
}


static uint8_t
rom_get(Device* dev, uint32_t pos)
{
    ROM* rom = (ROM*)dev;
    if(pos < rom->sz) {
        return rom->data[pos];
    } else {
        return 0;
    }
}
