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

//
// shortcuts
//

#define GET(pos) (lvm_get(cpu->computer, pos))
#define GET16(pos) (lvm_get16(cpu->computer, pos))
#define GET32(pos) (lvm_get32(cpu->computer, pos))
#define rPC (cpu->reg[PC])

//
// parameters
//

typedef struct Parameter {
    enum { REG, INDREG, V8, V16, V32, INDV32 } type;
    uint32_t value;
} Parameter;

inline static void
_twin_registers_rr(Parameter pars[2], uint8_t pos)
{
    pars[0] = (Parameter) { REG, (pos >> 4) };
    pars[1] = (Parameter) { REG, (pos & 0xF) };
}

inline static void
_twin_registers_ri(Parameter pars[2], uint8_t pos)
{
    pars[0] = (Parameter) { REG, (pos >> 4) };
    pars[1] = (Parameter) { INDREG, (pos & 0xF) };
}

inline static void
_twin_registers_ir(Parameter pars[2], uint8_t pos)
{
    pars[0] = (Parameter) { INDREG, (pos >> 4) };
    pars[1] = (Parameter) { REG, (pos & 0xF) };
}

inline static void
_twin_registers_ii(Parameter pars[2], uint8_t pos)
{
    pars[0] = (Parameter) { INDREG, (pos >> 4) };
    pars[1] = (Parameter) { INDREG, (pos & 0xF) };
}

inline static Parameter _reg(LVM_CPU* cpu, uint32_t offset) { return (Parameter) { REG, GET(rPC+offset) }; }
inline static Parameter _indreg(LVM_CPU* cpu, uint32_t offset) { return (Parameter) { INDREG, GET(rPC+offset) }; }
inline static Parameter _v8(LVM_CPU* cpu, uint32_t offset) { return (Parameter) { V8, GET(rPC+offset) }; }
inline static Parameter _v16(LVM_CPU* cpu, uint32_t offset) { return (Parameter) { V16, GET16(rPC+offset) }; }
inline static Parameter _v32(LVM_CPU* cpu, uint32_t offset) { return (Parameter) { V32, GET32(rPC+offset) }; }
inline static Parameter _indv32(LVM_CPU* cpu, uint32_t offset) { return (Parameter) { INDV32, GET32(rPC+offset) }; }


//
// operation constructors
//

inline static uint32_t
_take(LVM_CPU* cpu, Parameter par)
{
    switch(par.type) {
        case REG: return cpu->reg[par.value];
        case V8: case V16: case V32: return par.value;
        case INDV32: return lvm_get32(cpu->computer, par.value);
        case INDREG: return lvm_get32(cpu->computer, cpu->reg[par.value]);
    }
    abort();
}
#define TAKE(par) (_take(cpu, par))


inline static void
_apply(LVM_CPU* cpu, Parameter par, uint32_t value, uint32_t sz)
{
    lvm_cpusetflag(cpu, Z, value == 0);
    lvm_cpusetflag(cpu, S, (value >> 31) & 1);
    lvm_cpusetflag(cpu, V, false);
    lvm_cpusetflag(cpu, Y, false);
    lvm_cpusetflag(cpu, GT, false);
    lvm_cpusetflag(cpu, LT, false);

    switch(par.type) {
        case REG:
            assert(par.value < 16);
            cpu->reg[par.value] = value;
            break;
        case INDREG:
            switch(sz) {
                case  8: lvm_set(cpu->computer, cpu->reg[par.value], value); return;
                case 16: lvm_set16(cpu->computer, cpu->reg[par.value], value); return;
                case 32: lvm_set32(cpu->computer, cpu->reg[par.value], value); return;
                default: abort();
            }
        case INDV32:
            switch(sz) {
                case  8: lvm_set(cpu->computer, par.value, value); return;
                case 16: lvm_set16(cpu->computer, par.value, value); return;
                case 32: lvm_set32(cpu->computer, par.value, value); return;
                default: abort();
            }
        case V8: case V16: case V32: default:
            abort();
    }
}
#define APPLY(par, value, sz) (_apply(cpu, par, value, sz))


inline static void
_advance_pc(LVM_CPU* cpu, Parameter pars[2])
{
    if(((pars[0].type == REG) || (pars[0].type == INDREG)) && ((pars[1].type == REG) || (pars[1].type == INDREG))) {
        cpu->reg[PC] += 2;
    } else {
        for(int i=0; i<2; ++i) {
            switch(pars[i].type) {
                case REG: case INDREG: case V8:
                    cpu->reg[PC] += 1;
                    break;
                case V16:
                    cpu->reg[PC] += 2;
                    break;
                case V32: case INDV32:
                    cpu->reg[PC] += 4;
                    break;
                default:
                    abort();
            }
        }
    }
}
#define ADVANCE_PC(pars) (_advance_pc(cpu, pars))


// 
// instruction parser
//

typedef enum Instruction { 
    MOV, MOVB, MOVW, MOVD, SWAP,
    INVALID 
} Instruction;


inline static Instruction
_parse_opcode(LVM_CPU* cpu, Parameter pars[2])
{
    uint8_t opcode = GET(rPC);
    switch(opcode) {

        case 0x01: _twin_registers_rr(pars, GET(rPC+1));                            return MOV;
        case 0x02: pars[0] = _reg(cpu, rPC+1); pars[1] = _v8(cpu, rPC+2);           return MOV;
        case 0x03: pars[0] = _reg(cpu, rPC+1); pars[1] = _v16(cpu, rPC+2);          return MOV;
        case 0x04: pars[0] = _reg(cpu, rPC+1); pars[1] = _v32(cpu, rPC+2);          return MOV;

        case 0x05: _twin_registers_ri(pars, GET(rPC+1));                            return MOVB;
        case 0x06: pars[0] = _reg(cpu, rPC+1); pars[1] = _indv32(cpu, rPC+2);       return MOVB;
        case 0x07: _twin_registers_ir(pars, GET(rPC+1));                            return MOVB;
        case 0x08: pars[0] = _indreg(cpu, rPC+1); pars[1] = _v8(cpu, rPC+2);        return MOVB;
        case 0x09: _twin_registers_ii(pars, GET(rPC+1));                            return MOVB;
        case 0x0A: pars[0] = _indreg(cpu, rPC+1); pars[1] = _indv32(cpu, rPC+2);    return MOVB;
        case 0x0B: pars[0] = _indv32(cpu, rPC+1); pars[1] = _reg(cpu, rPC+5);       return MOVB;
        case 0x0C: pars[0] = _indv32(cpu, rPC+1); pars[1] = _v8(cpu, rPC+5);        return MOVB;
        case 0x0D: pars[0] = _indv32(cpu, rPC+1); pars[1] = _indreg(cpu, rPC+5);    return MOVB;
        case 0x0E: pars[0] = _indv32(cpu, rPC+1); pars[1] = _indv32(cpu, rPC+5);    return MOVB;

        case 0x0F: _twin_registers_ri(pars, GET(rPC+1));                            return MOVW;
        case 0x10: pars[0] = _reg(cpu, rPC+1); pars[1] = _indv32(cpu, rPC+2);       return MOVW;
        case 0x11: _twin_registers_ir(pars, GET(rPC+1));                            return MOVW;
        case 0x12: pars[0] = _indreg(cpu, rPC+1); pars[1] = _v16(cpu, rPC+2);       return MOVW;
        case 0x13: _twin_registers_ii(pars, GET(rPC+1));                            return MOVW;
        case 0x14: pars[0] = _indreg(cpu, rPC+1); pars[1] = _indv32(cpu, rPC+2);    return MOVW;
        case 0x15: pars[0] = _indv32(cpu, rPC+1); pars[1] = _reg(cpu, rPC+5);       return MOVW;
        case 0x16: pars[0] = _indv32(cpu, rPC+1); pars[1] = _v16(cpu, rPC+5);       return MOVW;
        case 0x17: pars[0] = _indv32(cpu, rPC+1); pars[1] = _indreg(cpu, rPC+5);    return MOVW;
        case 0x18: pars[0] = _indv32(cpu, rPC+1); pars[1] = _indv32(cpu, rPC+5);    return MOVW;

        case 0x19: _twin_registers_ri(pars, GET(rPC+1));                            return MOVD;
        case 0x1A: pars[0] = _reg(cpu, rPC+1); pars[1] = _indv32(cpu, rPC+2);       return MOVD;
        case 0x1B: _twin_registers_ir(pars, GET(rPC+1));                            return MOVD;
        case 0x1C: pars[0] = _indreg(cpu, rPC+1); pars[1] = _v32(cpu, rPC+2);       return MOVD;
        case 0x1D: _twin_registers_ii(pars, GET(rPC+1));                            return MOVD;
        case 0x1E: pars[0] = _indreg(cpu, rPC+1); pars[1] = _indv32(cpu, rPC+2);    return MOVD;
        case 0x1F: pars[0] = _indv32(cpu, rPC+1); pars[1] = _reg(cpu, rPC+5);       return MOVD;
        case 0x20: pars[0] = _indv32(cpu, rPC+1); pars[1] = _v32(cpu, rPC+5);       return MOVD;
        case 0x21: pars[0] = _indv32(cpu, rPC+1); pars[1] = _indreg(cpu, rPC+5);    return MOVD;
        case 0x22: pars[0] = _indv32(cpu, rPC+1); pars[1] = _indv32(cpu, rPC+5);    return MOVD;

        case 0x23: _twin_registers_rr(pars, GET(rPC+1));                            return SWAP;

        default:
            return INVALID;
    }
}


//
// operator execution
//

void 
lvm_cpustep(LVM_CPU* cpu)
{
    Parameter pars[2];
    switch(_parse_opcode(cpu, pars)) {
        case MOV:
            APPLY(pars[0], TAKE(pars[1]), 0);
            break;
        case MOVB:
            APPLY(pars[0], (uint8_t)TAKE(pars[1]), 8);
            break;
        case MOVW:
            APPLY(pars[0], (uint16_t)TAKE(pars[1]), 16);
            break;
        case MOVD:
            APPLY(pars[0], TAKE(pars[1]), 32);
            break;
        case SWAP: {
                uint32_t tmp = TAKE(pars[1]);
                APPLY(pars[1], TAKE(pars[0]), 0);
                APPLY(pars[0], tmp, 0);
            }
            break;
        case INVALID:
        default:
            syslog(LOG_ERR, "Invalid opcode 0x%02X", GET(cpu->reg[PC]));
            abort();
    }
    ADVANCE_PC(pars);
}

// }}}

#pragma GCC diagnostic pop

// {{{ REGS & FLAGS

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
