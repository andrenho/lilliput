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

#endif
