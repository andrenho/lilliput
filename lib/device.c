#include "device.h"

LVM_Device* device_init(
        void* ptr,
        void    (*step)(void*, size_t),
        uint8_t (*get)(void*, uint32_t),
        void    (*set)(void*, uint32_t, uint8_t),
        void    (*free)(void*),
        LVM_DeviceType type,
        uint32_t sz)
{
    LVM_Device* dev = calloc(sizeof(LVM_Device), 1);
    dev->ptr = ptr;
    dev->step = step;
    dev->get = get;
    dev->set = set;
    dev->free = free;
    dev->type = type;
    dev->sz = sz;
    return dev;
}


void device_free(LVM_Device* dev)
{
    if(dev->free)
        dev->free(dev->ptr);
    free(dev);
}
