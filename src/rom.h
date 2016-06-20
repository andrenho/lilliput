#ifndef ROM_H_
#define ROM_H_

#include <stdint.h>

void rom_init(const char* filename);
void rom_destroy();

uint32_t rom_size();

uint8_t rom_get(uint32_t pos);

#endif
