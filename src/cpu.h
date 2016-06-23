#ifndef CPU_H_
#define CPU_H_

#include <stdint.h>

void cpu_init();
void cpu_destroy();

uint32_t cpu_register(uint8_t n);
void     cpu_setregister(uint8_t n, uint32_t v);

void cpu_step();

#endif
