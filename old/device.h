#ifndef DEVICE_H_
#define DEVICE_H_

#include <stdint.h>
#include <stdlib.h>

typedef enum { DEV_ROM, DEV_VIDEO } DeviceType;

typedef struct Device {
    void        (*step)(struct Device*, size_t);
    uint8_t     (*get)(struct Device*, uint32_t);
    void        (*set)(struct Device*, uint32_t, uint8_t);
    void        (*free)(struct Device*);
    DeviceType  type;
    uint32_t    sz;
    uint32_t    pos;
} Device;

#endif
