#include "luisavm.h"

#include <assert.h>
#include <stdlib.h>
#include <syslog.h>

typedef struct LVM_CPU {
    LVM_Computer* computer;
    uint32_t reg[16];
} LVM_CPU;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-declarations"

// {{{ CONSTRUCTOR/DESTRUCTOR

LVM_CPU* 
lvm_createcpu(LVM_Computer* comp)
{
    LVM_CPU* cpu = calloc(1, sizeof(LVM_CPU));
    cpu->computer = comp;
    syslog(LOG_DEBUG, "New CPU created.");
    return cpu;
}


void
lvm_destroycpu(LVM_CPU* cpu)
{
    free(cpu);
    syslog(LOG_DEBUG, "CPU destroyed.");
}

// }}}

// {{{ STEP / RESET

void 
lvm_cpureset(LVM_CPU* cpu)
{
    for(int i=0; i<16; ++i) {
        cpu->reg[i] = 0;
    }
}


typedef enum Instruction { MOV, INVALID } Instruction;

typedef struct Parameter {
    enum { REGISTER } type;
    uint32_t value;
} Parameter;


#define GET(pos) (lvm_get(cpu->computer, pos))
#define rPC (cpu->reg[PC])


inline static void
_twin_registers(Parameter pars[2], uint8_t pos)
{
    pars[0] = (Parameter) { REGISTER, (pos >> 4) };
    pars[1] = (Parameter) { REGISTER, (pos & 0xF) };
}


inline static Instruction
_parse_opcode(LVM_CPU* cpu, Parameter pars[2])
{
    uint8_t opcode = GET(rPC);
    switch(opcode) {
        case 0x01:
            _twin_registers(pars, GET(rPC+1));
            return MOV;
        default:
            return INVALID;
    }
}


inline static void
_apply(LVM_CPU* cpu, Parameter par, uint32_t value)
{
    switch(par.type) {
        case REGISTER:
            assert(par.value < 16);
            cpu->reg[par.value] = value;
            break;
        default:
            abort();
    }
}
#define APPLY(par, value) (_apply(cpu, par, value))


inline static uint32_t
_take(LVM_CPU* cpu, Parameter par)
{
    switch(par.type) {
        case REGISTER:
            return cpu->reg[par.value];
    }
    abort();
}
#define TAKE(par) (_take(cpu, par))


void 
lvm_cpustep(LVM_CPU* cpu)
{
    Parameter pars[2];
    switch(_parse_opcode(cpu, pars)) {
        case MOV:
            APPLY(pars[0], TAKE(pars[1]));
            break;
        case INVALID:
        default:
            syslog(LOG_ERR, "Invalid opcode 0x%02X", GET(cpu->reg[PC]));
            abort();
    }
}

// }}}

#pragma GCC diagnostic pop

// {{{ REGISTERS & FLAGS

uint32_t 
lvm_cpuregister(LVM_CPU* cpu, LVM_CPURegister r)
{
    if(r > 15) {
        syslog(LOG_ERR, "Invalid register %u, exiting...", r);
        abort();
    }
    return cpu->reg[r];
}


void
lvm_cpusetregister(LVM_CPU* cpu, LVM_CPURegister r, uint32_t data)
{
    if(r > 15) {
        syslog(LOG_ERR, "Invalid register %u, exiting...", r);
        abort();
    }
    cpu->reg[r] = data;
}


bool 
lvm_cpuflag(LVM_CPU* cpu, LVM_CPUFlag f)
{
    return (bool)((lvm_cpuregister(cpu, FL) >> (int)f) & 1);
}


void 
lvm_cpusetflag(LVM_CPU* cpu, LVM_CPUFlag f, bool value)
{
    int64_t new_value = lvm_cpuregister(cpu, FL);
    new_value ^= (-value ^ new_value) & (1 << (int)f);
    lvm_cpusetregister(cpu, FL, (uint32_t)new_value);
}

// }}}
