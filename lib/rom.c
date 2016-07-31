#include "luisavm.h"

#include <stdlib.h>
#include <syslog.h>

#include "device.h"

typedef struct ROM {
    uint32_t sz;
    uint8_t* data;
} ROM;


ROM*
rom_init(uint32_t sz, uint8_t* data_new_ownership)
{
    ROM* rom = calloc(sizeof(ROM), 1);
    rom->sz = sz;
    rom->data = data_new_ownership;

    syslog(LOG_DEBUG, "A ROM device was created.");

    return rom;
}


static void
rom_free(void* ptr)
{
    ROM* rom = (ROM*)ptr;

    if(rom->data) {
        free(rom->data);
    }
    free(rom);
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


LVM_Device* rom_dev_init(uint32_t sz, uint8_t* data_new_ownership)
{
    return device_init(rom_init(sz, data_new_ownership),
            NULL, rom_get, NULL, rom_free, DEV_ROM, sz);
}
