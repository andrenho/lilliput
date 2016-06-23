#ifndef MEMORY_H_
#define MEMORY_H_

#include <stdint.h>

#include "config.h"

#define SYSTEM_AREA 0xF8000000

typedef uint8_t (*get_t)(uint32_t);
typedef void    (*set_t)(uint32_t, uint8_t);

void memory_init(Config* config);
void memory_destroy();

uint32_t memory_addmap(uint32_t pos, uint32_t sz, get_t get, set_t set);

uint8_t memory_get(uint32_t pos);
void    memory_set(uint32_t pos, uint8_t data);

uint16_t memory_get16(uint32_t pos);
uint32_t memory_get32(uint32_t pos);
void     memory_set16(uint32_t pos, uint16_t data);
void     memory_set32(uint32_t pos, uint32_t data);

uint32_t memory_offset();
uint32_t memory_set_offset(uint32_t offset);

#endif
