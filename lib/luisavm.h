#ifndef LUISAVM_H_
#define LUISAVM_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

//
// LOGS
// 

void lvm_debuglog(bool active);

// 
// COMPUTER
//

typedef struct LVM_Computer LVM_Computer;
typedef struct LVM_CPU LVM_CPU;

LVM_Computer* lvm_computercreate(uint32_t physical_memory_size);
void          lvm_computerdestroy(LVM_Computer* comp);

uint8_t lvm_get(LVM_Computer* comp, uint32_t pos);
void    lvm_set(LVM_Computer* comp, uint32_t pos, uint8_t data);

uint16_t lvm_get16(LVM_Computer* comp, uint32_t pos);
void     lvm_set16(LVM_Computer* comp, uint32_t pos, uint16_t data);
uint32_t lvm_get32(LVM_Computer* comp, uint32_t pos);
void     lvm_set32(LVM_Computer* comp, uint32_t pos, uint32_t data);

uint8_t* lvm_physicalmemory(LVM_Computer* comp);
uint32_t lvm_physicalmemorysz(LVM_Computer* comp);

void     lvm_setoffset(LVM_Computer* comp, uint32_t offset);
uint32_t lvm_offset(LVM_Computer* comp);

LVM_CPU* lvm_addcpu(LVM_Computer* comp);
LVM_CPU* lvm_cpu(LVM_Computer* comp, size_t n);

// 
// CPU
//

typedef enum LVM_CPURegister {
    A, B, C, D, E, F, G, H, I, J, K, L, FP, SP, PC, FL
} LVM_CPURegister;

uint32_t lvm_cpuregister(LVM_CPU* cpu, LVM_CPURegister r);
void     lvm_cpusetregister(LVM_CPU* cpu, LVM_CPURegister r, uint32_t data);

typedef enum LVM_CPUFlag {
    Y, V, Z, S, GT, LT,
} LVM_CPUFlag;

bool lvm_cpuflag(LVM_CPU* cpu, LVM_CPUFlag f);
void lvm_cpusetflag(LVM_CPU* cpu, LVM_CPUFlag f, bool value);

#endif
