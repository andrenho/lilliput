#ifndef LUISAVM_H_
#define LUISAVM_H_

#include <stdint.h>

typedef struct LVM_Computer LVM_Computer;

LVM_Computer* lvm_computercreate(uint32_t physical_memory_size);
void lvm_computerdestroy(LVM_Computer* comp);

#endif
