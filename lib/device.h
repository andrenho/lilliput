#ifndef DEVICE_H_
#define DEVICE_H_

#include <stdint.h>
#include <stdlib.h>

typedef enum { DEV_ROM, DEV_VIDEO } LVM_DeviceType;

typedef struct LVM_Device {
    void     *ptr;
    void    (*step)(void*, size_t);
    uint8_t (*get)(void*, uint32_t);
    void    (*set)(void*, uint32_t, uint8_t);
    void    (*free)(void*);
    LVM_DeviceType type;
    uint32_t sz;
} LVM_Device;

LVM_Device* device_init(
        void* ptr,
        void    (*step)(void*, size_t),
        uint8_t (*get)(void*, uint32_t),
        void    (*set)(void*, uint32_t, uint8_t),
        void    (*free)(void*),
        LVM_DeviceType type,
        uint32_t sz);

void device_free(LVM_Device* dev);

#endif
