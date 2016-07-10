#ifndef CPU_H_
#define CPU_H_

#include <stdbool.h>
#include <stdint.h>

void cpu_init();
void cpu_destroy();

typedef enum {
    A, B, C, D, E, F, G, H, I, J, K, L, FP, SP, PC, FL,
} Register;
uint32_t cpu_register(Register n);
void     cpu_setregister(Register n, uint32_t v);

typedef enum {
    Y, V, Z, S, GT, LT, P, T,
} Flag;
bool cpu_flag(Flag f);
void cpu_setflag(Flag f, bool value);

void cpu_step();
void cpu_reset();

#endif
