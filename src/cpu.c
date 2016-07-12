#include "cpu.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <syslog.h>

#include "memory.h"

#define rPC (reg[PC])

uint32_t reg[16] = { 0 };

void
cpu_init()
{
}


void
cpu_destroy()
{
}


// {{{ REGISTERS

inline uint32_t
cpu_register(Register n)
{
    return reg[n];
}


inline void 
cpu_setregister(Register n, uint32_t v)
{
    reg[n] = v;
}


inline bool 
cpu_flag(Flag f)
{
    return (bool)((cpu_register(FL) >> (int)f) & 1);
}


inline void 
cpu_setflag(Flag f, bool value)
{
    int64_t new_value = cpu_register(FL);
    new_value ^= (-value ^ new_value) & (1 << (int)f);
    cpu_setregister(FL, (uint32_t)new_value);
}

// }}}


// {{{ STEP

static inline uint32_t 
affect_flags(uint32_t value)
{
    cpu_setflag(Z, (value & 0xFFFFFFFF) == 0);
    cpu_setflag(P, (value % 2) == 0);
    cpu_setflag(S, (value >> 31) & 1);
    cpu_setflag(V, 0);
    cpu_setflag(Y, 0);
    cpu_setflag(GT, 0);
    cpu_setflag(LT, 0);
    return value;
}


void cpu_step()
{
    uint8_t opcode = memory_get(rPC);

    switch(opcode) {

        // 
        // MOV
        //

        case 0x01: {  // mov R, R
                uint32_t value = reg[memory_get(rPC+1) >> 4];
                reg[memory_get(rPC+1) & 0xF] = affect_flags(value);
                rPC += 2;
            }
            break;

        case 0x02: {  // mov R, v8
                uint8_t value = memory_get(rPC+2);
                reg[memory_getreg(rPC+1)] = affect_flags(value);
                rPC += 3;
            }
            break;

        case 0x03: {  // mov R, v16
                uint16_t value = memory_get16(rPC+2);
                reg[memory_getreg(rPC+1)] = affect_flags(value);
                rPC += 4;
            }
            break;

        case 0x04: {  // mov R, v32
                uint32_t value = memory_get32(rPC+2);
                reg[memory_getreg(rPC+1)] = affect_flags(value);
                rPC += 6;
            }
            break;

        // 
        // MOVB
        //
        case 0x05: {  // movb R, [R]
                uint8_t value = memory_get(reg[memory_get(rPC+1) >> 4]);
                reg[memory_get(rPC+1) & 0xF] = affect_flags(value);
                rPC += 2;
            }
            break;

        case 0x06: {  // movb R, [v32]
                uint8_t value = memory_get(memory_get32(rPC+2));
                reg[memory_get(rPC+1)] = affect_flags(value);
                rPC += 6;
            }
            break;

        case 0x0B: {  // movb [R], R
                uint8_t value = (uint8_t)reg[memory_get(rPC+1) >> 4];
                memory_set(reg[memory_get(rPC+1) & 0xF], (uint8_t)affect_flags(value));
                rPC += 2;
            }
            break;

        case 0x0C: {  // movb [R], v8
                uint8_t value = memory_get(rPC+2);
                memory_set(reg[memory_get(rPC+1)], (uint8_t)affect_flags(value));
                rPC += 3;
            }
            break;

        case 0x0D: {  // movb [R], [R]
                uint8_t value = memory_get(reg[memory_get(rPC+1) >> 4]);
                memory_set(reg[memory_get(rPC+1) & 0xF], (uint8_t)affect_flags(value));
                rPC += 2;
            }
            break;

        case 0x0E: {  // movb [R], [v32]
                uint8_t value = memory_get(memory_get32(rPC+2));
                memory_set(reg[memory_get(rPC+1)], (uint8_t)affect_flags(value));
                rPC += 6;
            }
            break;

        case 0x21: {  // movb [v32], R
                uint8_t value = (uint8_t)reg[memory_get(rPC+5)];
                memory_set(memory_get32(rPC+1), (uint8_t)affect_flags(value));
                rPC += 6;
            }
            break;

        case 0x22: {  // movb [v32], v8
                uint8_t value = memory_get(rPC+5);
                memory_set(memory_get32(rPC+1), (uint8_t)affect_flags(value));
                rPC += 6;
            }
            break;

        case 0x23: {  // movb [v32], [R]
                uint8_t value = memory_get(reg[memory_get(rPC+5)]);
                memory_set(memory_get32(rPC+1), (uint8_t)affect_flags(value));
                rPC += 6;
            }
            break;

        case 0x24: {  // movb [v32], [v32]
                uint8_t value = memory_get(memory_get32(memory_get32(rPC+5)));
                memory_set(memory_get32(rPC+1), (uint8_t)affect_flags(value));
                rPC += 9;
            }
            break;

        // 
        // MOVW
        //

        case 0x07: {  // movw R, [R]
                uint16_t value = memory_get16(reg[memory_get(rPC+1) >> 4]);
                reg[memory_get(rPC+1) & 0xF] = affect_flags(value);
                rPC += 2;
            }
            break;

        case 0x08: {  // movw R, [v32]
                uint16_t value = memory_get16(memory_get32(rPC+2));
                reg[memory_get(rPC+1)] = affect_flags(value);
                rPC += 6;
            }
            break;

        case 0x0F: {  // movw [R], R
                uint16_t value = (uint16_t)reg[memory_get(rPC+1) >> 4];
                memory_set16(reg[memory_get(rPC+1) & 0xF], (uint16_t)affect_flags(value));
                rPC += 2;
            }
            break;

        case 0x1A: {  // movw [R], v16
                uint16_t value = memory_get16(rPC+2);
                memory_set16(reg[memory_get(rPC+1)], (uint16_t)affect_flags(value));
                rPC += 4;
            }
            break;

        case 0x1B: {  // movw [R], [R]
                uint16_t value = memory_get16(reg[memory_get(rPC+1) >> 4]);
                memory_set16(reg[memory_get(rPC+1) & 0xF], (uint16_t)affect_flags(value));
                rPC += 2;
            }
            break;

        case 0x1C: {  // movw [R], [v32]
                uint16_t value = memory_get16(memory_get32(rPC+2));
                memory_set16(reg[memory_get(rPC+1)], (uint16_t)affect_flags(value));
                rPC += 6;
            }
            break;

        case 0x25: {  // movw [v32], R
                uint16_t value = (uint16_t)reg[memory_get(rPC+5)];
                memory_set16(memory_get32(rPC+1), (uint16_t)affect_flags(value));
                rPC += 6;
            }
            break;

        case 0x26: {  // movw [v32], v16
                uint16_t value = memory_get16(rPC+5);
                memory_set16(memory_get32(rPC+1), (uint16_t)affect_flags(value));
                rPC += 7;
            }
            break;

        case 0x27: {  // movw [v32], [R]
                uint16_t value = memory_get16(reg[memory_get(rPC+5)]);
                memory_set16(memory_get32(rPC+1), (uint16_t)affect_flags(value));
                rPC += 6;
            }
            break;

        case 0x28: {  // movw [v32], [v32]
                uint16_t value = memory_get16(memory_get32(memory_get32(rPC+5)));
                memory_set16(memory_get32(rPC+1), (uint16_t)affect_flags(value));
                rPC += 9;
            }
            break;

        // 
        // MOVD
        //

        case 0x09: {  // movd R, [R]
                uint32_t value = memory_get32(reg[memory_get(rPC+1) >> 4]);
                reg[memory_get(rPC+1) & 0xF] = affect_flags(value);
                rPC += 2;
            }
            break;

        case 0x0A: {  // movd R, [v32]
                uint32_t value = memory_get32(memory_get32(rPC+2));
                reg[memory_get(rPC+1)] = affect_flags(value);
                rPC += 6;
            }
            break;

        case 0x1D: {  // movd [R], R
                uint32_t value = reg[memory_get(rPC+1) >> 4];
                memory_set32(reg[memory_get(rPC+1) & 0xF], affect_flags(value));
                rPC += 2;
            }
            break;

        case 0x1E: {  // movd [R], v32
                uint32_t value = memory_get32(rPC+2);
                memory_set32(reg[memory_get(rPC+1)], affect_flags(value));
                rPC += 6;
            }
            break;

        case 0x1F: {  // movd [R], [R]
                uint32_t value = memory_get32(reg[memory_get(rPC+1) >> 4]);
                memory_set32(reg[memory_get(rPC+1) & 0xF], affect_flags(value));
                rPC += 2;
            }
            break;

        case 0x20: {  // movd [R], [v32]
                uint32_t value = memory_get32(memory_get32(rPC+2));
                memory_set32(reg[memory_get(rPC+1)], affect_flags(value));
                rPC += 6;
            }
            break;

        case 0x29: {  // movd [v32], R
                uint32_t value = reg[memory_get(rPC+5)];
                memory_set32(memory_get32(rPC+1), affect_flags(value));
                rPC += 6;
            }
            break;

        case 0x2A: {  // movd [v32], v32
                uint32_t value = memory_get32(rPC+5);
                memory_set32(memory_get32(rPC+1), affect_flags(value));
                rPC += 9;
            }
            break;

        case 0x2B: {  // movd [v32], [R]
                uint32_t value = memory_get32(reg[memory_get(rPC+5)]);
                memory_set32(memory_get32(rPC+1), affect_flags(value));
                rPC += 6;
            }
            break;

        case 0x2C: {  // movd [v32], [v32]
                uint32_t value = memory_get32(memory_get32(memory_get32(rPC+5)));
                memory_set32(memory_get32(rPC+1), affect_flags(value));
                rPC += 9;
            }
            break;

        //
        // SWAP
        //
        
        case 0x8A: {  // swap R, R
                uint32_t tmp = reg[memory_get(rPC+1) >> 4];
                reg[memory_get(rPC+1) >> 4] = reg[memory_get(rPC+1) & 0xF];
                reg[memory_get(rPC+1) & 0xF] = tmp;
                rPC += 2;
            }
            break;

        // 
        // OR
        //

        case 0x2D: {  // or R, R
                uint32_t value = reg[memory_get(rPC+1) & 0xF] | reg[memory_get(rPC+1) >> 4];
                reg[memory_get(rPC+1) & 0xF] = affect_flags(value);
                rPC += 2;
            }
            break;

        case 0x2E: {  // or R, v8
                uint32_t value = reg[memory_get(rPC+1)] | memory_get(rPC+2);
                reg[memory_get(rPC+1)] = affect_flags(value);
                rPC += 3;
            }
            break;

        case 0x2F: {  // or R, v16
                uint32_t value = reg[memory_get(rPC+1)] | memory_get16(rPC+2);
                reg[memory_get(rPC+1)] = affect_flags(value);
                rPC += 4;
            }
            break;

        case 0x30: {  // or R, v32
                uint32_t value = reg[memory_get(rPC+1)] | memory_get32(rPC+2);
                reg[memory_get(rPC+1)] = affect_flags(value);
                rPC += 6;
            }
            break;

        // 
        // XOR
        //

        case 0x31: {  // xor R, R
                uint32_t value = reg[memory_get(rPC+1) & 0xF] ^ reg[memory_get(rPC+1) >> 4];
                reg[memory_get(rPC+1) & 0xF] = affect_flags(value);
                rPC += 2;
            }
            break;

        case 0x32: {  // xor R, v8
                uint32_t value = reg[memory_get(rPC+1)] ^ memory_get(rPC+2);
                reg[memory_get(rPC+1)] = affect_flags(value);
                rPC += 3;
            }
            break;

        case 0x33: {  // xor R, v16
                uint32_t value = reg[memory_get(rPC+1)] ^ memory_get16(rPC+2);
                reg[memory_get(rPC+1)] = affect_flags(value);
                rPC += 4;
            }
            break;

        case 0x34: {  // xor R, v32
                uint32_t value = reg[memory_get(rPC+1)] ^ memory_get32(rPC+2);
                reg[memory_get(rPC+1)] = affect_flags(value);
                rPC += 6;
            }
            break;

        // 
        // AND
        //

        case 0x35: {  // and R, R
                uint32_t value = reg[memory_get(rPC+1) & 0xF] & reg[memory_get(rPC+1) >> 4];
                reg[memory_get(rPC+1) & 0xF] = affect_flags(value);
                rPC += 2;
            }
            break;

        case 0x36: {  // and R, v8
                uint32_t value = reg[memory_get(rPC+1)] & memory_get(rPC+2);
                reg[memory_get(rPC+1)] = affect_flags(value);
                rPC += 3;
            }
            break;

        case 0x37: {  // and R, v16
                uint32_t value = reg[memory_get(rPC+1)] & memory_get16(rPC+2);
                reg[memory_get(rPC+1)] = affect_flags(value);
                rPC += 4;
            }
            break;

        case 0x38: {  // and R, v32
                uint32_t value = reg[memory_get(rPC+1)] & memory_get32(rPC+2);
                reg[memory_get(rPC+1)] = affect_flags(value);
                rPC += 6;
            }
            break;

        // 
        // SHIFT
        //

        case 0x39: {  // shl R, R
                uint32_t value = reg[memory_get(rPC+1) & 0xF] << reg[memory_get(rPC+1) >> 4];
                reg[memory_get(rPC+1) & 0xF] = affect_flags(value);
                rPC += 2;
            }
            break;

        case 0x3A: {  // shl R, v8
                uint32_t value = reg[memory_get(rPC+1)] << memory_get(rPC+2);
                reg[memory_get(rPC+1)] = affect_flags(value);
                rPC += 3;
            }
            break;

        case 0x3D: {  // shr R, R
                uint32_t value = reg[memory_get(rPC+1) & 0xF] >> reg[memory_get(rPC+1) >> 4];
                reg[memory_get(rPC+1) & 0xF] = affect_flags(value);
                rPC += 2;
            }
            break;

        case 0x3E: {  // shr R, v8
                uint32_t value = reg[memory_get(rPC+1)] >> memory_get(rPC+2);
                reg[memory_get(rPC+1)] = affect_flags(value);
                rPC += 3;
            }
            break;

        // 
        // NOT
        //

        case 0x41: {  // not R
                uint32_t value = ~reg[memory_get(rPC+1)];
                reg[memory_get(rPC+1)] = affect_flags(value);
                rPC += 2;
            }
            break;

        // 
        // ADD
        //

        case 0x42: {  // add R, R
                uint64_t value = (uint64_t)reg[memory_get(rPC+1) & 0xF] + (uint64_t)reg[memory_get(rPC+1) >> 4] + (uint64_t)cpu_flag(Y);
                reg[memory_get(rPC+1) & 0xF] = affect_flags((uint32_t)value);
                cpu_setflag(Y, value > 0xFFFFFFFF);
                rPC += 2;
            }
            break;

        case 0x43: {  // add R, v8
                uint64_t value = (uint64_t)reg[memory_get(rPC+1)] + (uint64_t)memory_get(rPC+2) + (uint64_t)cpu_flag(Y);
                reg[memory_get(rPC+1)] = affect_flags((uint32_t)value);
                cpu_setflag(Y, value > 0xFFFFFFFF);
                rPC += 3;
            }
            break;

        case 0x44: {  // add R, v16
                uint64_t value = (uint64_t)reg[memory_get(rPC+1)] + (uint64_t)memory_get16(rPC+2) + (uint64_t)cpu_flag(Y);
                reg[memory_get(rPC+1)] = affect_flags((uint32_t)value);
                cpu_setflag(Y, value > 0xFFFFFFFF);
                rPC += 4;
            }
            break;

        case 0x45: {  // add R, v32
                uint64_t value = (uint64_t)reg[memory_get(rPC+1)] + (uint64_t)memory_get32(rPC+2) + (uint64_t)cpu_flag(Y);
                reg[memory_get(rPC+1)] = affect_flags((uint32_t)value);
                cpu_setflag(Y, value > 0xFFFFFFFF);
                rPC += 6;
            }
            break;

        //
        // SUB
        //

        case 0x46: {  // sub R, v8
                int64_t value = (int64_t)reg[memory_get(rPC+1) & 0xF] - (int64_t)reg[memory_get(rPC+1) >> 4] - (int64_t)cpu_flag(Y);
                reg[memory_get(rPC+1) & 0xF] = affect_flags((uint32_t)value);
                cpu_setflag(Y, value < 0);
                rPC += 2;
            }
            break;

        //
        // INVALID OPCODE
        //

        default:
            syslog(LOG_ERR, "Invalid opcode 0x%02X from memory position 0x%" PRIX32, opcode, reg[PC]);
            exit(EXIT_FAILURE);
    }

/*
    f[0x47] = pos => {  // sub R, v8
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] - p2 - (this.Y ? 1 : 0);
      reg[p1] = this._affectFlags(r);
      this.Y = (r < 0);
      return 2;
    };
    
    f[0x48] = pos => {  // sub R, v16
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get16(pos + 1)];
      const r = reg[p1] - p2 - (this.Y ? 1 : 0);
      reg[p1] = this._affectFlags(r);
      this.Y = (r < 0);
      return 3;
    };
    
    f[0x49] = pos => {  // sub R, v32
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get32(pos + 1)];
      const r = reg[p1] - p2 - (this.Y ? 1 : 0);
      reg[p1] = this._affectFlags(r);
      this.Y = (r < 0);
      return 5;
    };
    
    f[0x4A] = pos => {  // cmp R, R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      this._affectFlags(reg[p1] - reg[p2], 32);
      this.LT = reg[p1] < reg[p2];
      this.GT = reg[p1] > reg[p2];
      this.Y = (reg[p1] - reg[p2]) < 0;
      return 2;
    };
    
    f[0x4B] = pos => {  // cmp R, v8
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      this._affectFlags(reg[p1] - p2, 8);
      this.LT = reg[p1] < p2;
      this.GT = reg[p1] > p2;
      this.Y = (reg[p1] - p2) < 0;
      return 2;
    };
    
    f[0x4C] = pos => {  // cmp R, v16
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get16(pos + 1)];
      this._affectFlags(reg[p1] - p2, 16);
      this.LT = reg[p1] < p2;
      this.GT = reg[p1] > p2;
      this.Y = (reg[p1] - p2) < 0;
      return 3;
    };
    
    f[0x4D] = pos => {  // cmp R, v32
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get32(pos + 1)];
      this._affectFlags(reg[p1] - p2, 32);
      this.LT = reg[p1] < p2;
      this.GT = reg[p1] > p2;
      this.Y = (reg[p1] - p2) < 0;
      return 5;
    };

    f[0x8B] = pos => {  // cmp R
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      this._affectFlags(reg[p1]);
      this.Y = reg[p1] < 0;
      return 1;
    };
    
    f[0x4E] = pos => {  // mul R, R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] * reg[p2];
      reg[p1] = this._affectFlags(r);
      this.V = (r > 0xFFFFFFFF);
      return 2;
    };
    
    f[0x4F] = pos => {  // mul R, v8
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] * p2;
      reg[p1] = this._affectFlags(r);
      this.V = (r > 0xFFFFFFFF);
      return 2;
    };
    
    f[0x50] = pos => {  // mul R, v16
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get16(pos + 1)];
      const r = reg[p1] * p2;
      reg[p1] = this._affectFlags(r);
      this.V = (r > 0xFFFFFFFF);
      return 3;
    };
    
    f[0x51] = pos => {  // mul R, v32
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get32(pos + 1)];
      const r = reg[p1] * p2;
      reg[p1] = this._affectFlags(r);
      this.V = (r > 0xFFFFFFFF);
      return 5;
    };
    
    f[0x52] = pos => {  // idiv R, R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = Math.floor(reg[p1] / reg[p2]);
      reg[p1] = this._affectFlags(r);
      return 2;
    };
    
    f[0x53] = pos => {  // idiv R, v8
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = Math.floor(reg[p1] / p2);
      reg[p1] = this._affectFlags(r);
      return 2;
    };
    
    f[0x54] = pos => {  // idiv R, v16
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get16(pos + 1)];
      const r = Math.floor(reg[p1] / p2);
      reg[p1] = this._affectFlags(r);
      return 3;
    };
    
    f[0x55] = pos => {  // idiv R, v32
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get32(pos + 1)];
      const r = Math.floor(reg[p1] / p2);
      reg[p1] = this._affectFlags(r);
      return 5;
    };
    
    f[0x56] = pos => {  // mod R, R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] % reg[p2];
      reg[p1] = this._affectFlags(r);
      return 2;
    };
    
    f[0x57] = pos => {  // mod R, v8
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] % p2;
      reg[p1] = this._affectFlags(r);
      return 2;
    };
    
    f[0x58] = pos => {  // mod R, v16
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get16(pos + 1)];
      const r = reg[p1] % p2;
      reg[p1] = this._affectFlags(r);
      return 3;
    };
    
    f[0x59] = pos => {  // mod R, v32
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get32(pos + 1)];
      const r = reg[p1] % p2;
      reg[p1] = this._affectFlags(r);
      return 5;
    };

    f[0x5A] = pos => {  // inc R
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      const r = reg[p1] + 1;
      reg[p1] = this._affectFlags(r);
      this.Y = (r > 0xFFFFFFFF);
      return 1;
    };
    
    f[0x5B] = pos => {  // dec R
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      const r = reg[p1] - 1;
      reg[p1] = this._affectFlags(r);
      this.Y = (r < 0);
      return 1;
    };

    //
    // BRANCHES
    //

    f[0x5C] = pos => {  // bz A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      if (this.Z) {
        this.PC = reg[p1];
        return 0;
      }
      return 1;
    };

    f[0x5D] = pos => {  // bz v32
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get32(pos);
      if (this.Z) {
        this.PC = p1;
        return 0;
      }
      return 4;
    };

    f[0x5E] = pos => {  // bz A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      if (!this.Z) {
        this.PC = reg[p1];
        return 0;
      }
      return 1;
    };

    f[0x5F] = pos => {  // bz v32
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get32(pos);
      if (!this.Z) {
        this.PC = p1;
        return 0;
      }
      return 4;
    };

    f[0x60] = pos => {  // bneg A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      if (this.S) {
        this.PC = reg[p1];
        return 0;
      }
      return 1;
    };

    f[0x61] = pos => {  // bneg v32
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get32(pos);
      if (this.S) {
        this.PC = p1;
        return 0;
      }
      return 4;
    };

    f[0x62] = pos => {  // bpos A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      if (!this.S) {
        this.PC = reg[p1];
        return 0;
      }
      return 1;
    };

    f[0x63] = pos => {  // bpos v32
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get32(pos);
      if (!this.S) {
        this.PC = p1;
        return 0;
      }
      return 4;
    };

    f[0x64] = pos => {  // bgt A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      if (this.GT) {
        this.PC = reg[p1];
        return 0;
      }
      return 1;
    };

    f[0x65] = pos => {  // bgt v32
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get32(pos);
      if (this.GT) {
        this.PC = p1;
        return 0;
      }
      return 4;
    };

    f[0x66] = pos => {  // bgte A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      if (this.GT && this.Z) {
        this.PC = reg[p1];
        return 0;
      }
      return 1;
    };

    f[0x67] = pos => {  // bgte v32
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get32(pos);
      if (this.GT && this.Z) {
        this.PC = p1;
        return 0;
      }
      return 4;
    };

    f[0x68] = pos => {  // blt A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      if (this.LT) {
        this.PC = reg[p1];
        return 0;
      }
      return 1;
    };

    f[0x69] = pos => {  // blt v32
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get32(pos);
      if (this.LT) {
        this.PC = p1;
        return 0;
      }
      return 4;
    };

    f[0x6A] = pos => {  // blte A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      if (this.LT && this.Z) {
        this.PC = reg[p1];
        return 0;
      }
      return 1;
    };

    f[0x6B] = pos => {  // blte v32
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get32(pos);
      if (this.LT && this.Z) {
        this.PC = p1;
        return 0;
      }
      return 4;
    };

    f[0x6C] = pos => {  // bv A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      if (this.V) {
        this.PC = reg[p1];
        return 0;
      }
      return 1;
    };

    f[0x6D] = pos => {  // bv v32
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get32(pos);
      if (this.V) {
        this.PC = p1;
        return 0;
      }
      return 4;
    };

    f[0x6E] = pos => {  // bv A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      if (!this.V) {
        this.PC = reg[p1];
        return 0;
      }
      return 1;
    };

    f[0x6F] = pos => {  // bv v32
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get32(pos);
      if (!this.V) {
        this.PC = p1;
        return 0;
      }
      return 4;
    };

    f[0x70] = pos => {  // jmp A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = this._mb.get(pos);
      this.PC = this._reg[p1];
      return 0;
    };

    f[0x71] = pos => {  // jmp v32
      const p1 = this._mb.get32(pos);
      this.PC = p1;
      return 0;
    };

    f[0x72] = pos => {  // jsr A
      const p1 = this._mb.get(pos);
      this._push32(this.PC + 2);
      this.PC = this._reg[p1];
      return 0;
    };

    f[0x73] = pos => {  // jsr v32
      const p1 = this._mb.get32(pos);
      this._push32(this.PC + 5);
      this.PC = p1;
      return 0;
    };

    f[0x74] = pos => {  // ret
      this.PC = this._pop32();
      return 0;
    };

    f[0x75] = pos => {  // sys R
      const p1 = this._mb.get(pos);
      // TODO - enter supervisor mode
      this._push32(this.PC + 2);
      this.PC = this._syscallVector[this._reg[p1] & 0xFF];
      return 0;
    };
      
    f[0x76] = pos => {  // sys v8
      const p1 = this._mb.get(pos);
      // TODO - enter supervisor mode
      this._push32(this.PC + 2);
      this.PC = this._syscallVector[p1];
      return 0;
    };
      
    f[0x77] = pos => {  // iret
      this.PC = this._pop32();
      this.T = true;
      return 0;
    };

    f[0x86] = pos => {  // sret
      this.PC = this._pop32();
      // TODO - leave supervisor mode
      return 0;
    };

    f[0x78] = pos => {  // pushb R
      const p1 = this._mb.get(pos);
      this._push(this._reg[p1] & 0xFF);
      return 1;
    };

    f[0x79] = pos => {  // pushb v8
      const p1 = this._mb.get(pos);
      this._push(p1);
      return 1;
    };

    f[0x7A] = pos => {  // pushw R
      const p1 = this._mb.get(pos);
      this._push16(this._reg[p1] & 0xFFFF);
      return 1;
    };

    f[0x7B] = pos => {  // pushw v16
      const p1 = this._mb.get16(pos);
      this._push16(p1);
      return 2;
    };

    f[0x7C] = pos => {  // pushw R
      const p1 = this._mb.get(pos);
      this._push32(this._reg[p1]);
      return 1;
    };

    f[0x7D] = pos => {  // pushw v16
      const p1 = this._mb.get32(pos);
      this._push32(p1);
      return 4;
    };

    f[0x7E] = pos => {  // push.a
      for (let i = 0x0; i <= 0xB; ++i) {
        this._push32(this._reg[i]);
      }
      return 0;
    };

    f[0x7F] = pos => {  // popb R
      const p1 = this._mb.get(pos);
      this._reg[p1] = this._pop();
      return 1;
    };

    f[0x80] = pos => {  // popw R
      const p1 = this._mb.get(pos);
      this._reg[p1] = this._pop16();
      return 1;
    };

    f[0x81] = pos => {  // popw R
      const p1 = this._mb.get(pos);
      this._reg[p1] = this._pop32();
      return 1;
    };

    f[0x82] = pos => {  // pop.a
      for (let i = 0xB; i >= 0x0; --i) {
        this._reg[i] = this._pop32();
      }
      return 0;
    };

    f[0x83] = pos => {  // popx R
      const p1 = this._mb.get(pos);
      for (let i = 0; i < this._reg[p1]; ++i) {
        this._pop();
      }
      return 1;
    };
      
    f[0x84] = pos => {  // popx v8
      const p1 = this._mb.get(pos);
      for (let i = 0; i < p1; ++i) {
        this._pop();
      }
      return 1;
    };
      
    f[0x85] = pos => {  // popx v16
      const p1 = this._mb.get16(pos);
      for (let i = 0; i < p1; ++i) {
        this._pop();
      }
      return 2;
    };

    f[0x87] = pos => {  // nop
      return 1;
    };

    f[0x88] = pos => {  // halt
      this.systemHalted = true;
      return 1;
    };

    f[0x89] = pos => {  // dbg
      this.activateDebugger = true;
      return 1;
    };

    return f;
  }

  step() {
    const n = this._stepFunction[this._mb.get(this.PC)](this.PC + 1);
    if (n) {
      this.PC += n + 1;
    }
  }

  
  checkInterrupts() {
    if (this.T && this._interruptsPending.length > 0) {
      let n = this._interruptsPending.shift();
      this._push32(this.PC);
      this.T = false;
      this.PC = this._interruptVector[n];
      this.systemHalted = false;
    }
  }


  _affectFlags(value) {
    this.Z = ((value & 0xFFFFFFFF) === 0);
    this.P = ((value % 2) === 0);
    this.S = ((value >> 31) & 0x1 ? true : false);
    this.V = false;
    this.Y = false;
    this.GT = false;
    this.LT = false;
    return value & 0xFFFFFFFF;
  }


  // 
  // STACK
  //

  _push(value) {
    this._mb.set(this.SP, value);
    this.SP -= 1;
  }


  _push16(value) {
    this.SP -= 1;
    this._mb.set16(this.SP, value);
    this.SP -= 1;
  }


  _push32(value) {
    this.SP -= 3;
    this._mb.set32(this.SP, value);
    this.SP -= 1;
  }
  

  _pop() {
    this.SP += 1;
    return this._mb.get(this.SP);
  }


  _pop16() {
    this.SP += 1;
    const r = this._mb.get16(this.SP);
    this.SP += 1;
    return r;
  }


  _pop32() {
    this.SP += 1;
    const r = this._mb.get32(this.SP);
    this.SP += 3;
    return r;
  }


  //
  // GETTERS / SETTERS
  //

  get A() { return this._reg[0]; }
  get B() { return this._reg[1]; }
  get C() { return this._reg[2]; }
  get D() { return this._reg[3]; }
  get E() { return this._reg[4]; }
  get F() { return this._reg[5]; }
  get G() { return this._reg[6]; }
  get H() { return this._reg[7]; }
  get I() { return this._reg[8]; }
  get J() { return this._reg[9]; }
  get K() { return this._reg[10]; }
  get L() { return this._reg[11]; }
  get FP() { return this._reg[12]; }
  get SP() { return this._reg[13]; }
  get PC() { return this._reg[14]; }
  get FL() { return this._reg[15]; }

  set A(v) { this._reg[0] = v; }
  set B(v) { this._reg[1] = v; }
  set C(v) { this._reg[2] = v; }
  set D(v) { this._reg[3] = v; }
  set E(v) { this._reg[4] = v; }
  set F(v) { this._reg[5] = v; }
  set G(v) { this._reg[6] = v; }
  set H(v) { this._reg[7] = v; }
  set I(v) { this._reg[8] = v; }
  set J(v) { this._reg[9] = v; }
  set K(v) { this._reg[10] = v; }
  set L(v) { this._reg[11] = v; }
  set FP(v) { this._reg[12] = v; }
  set SP(v) { this._reg[13] = v & 0xFFFFFFFF; }
  set PC(v) { this._reg[14] = v; }
  set FL(v) { this._reg[15] = v; }

  get Y() { return (this._reg[15] & 0x1) ? true : false; }
  get V() { return ((this._reg[15] >> 1) & 0x1) ? true : false; }
  get Z() { return ((this._reg[15] >> 2) & 0x1) ? true : false; }
  get S() { return ((this._reg[15] >> 3) & 0x1) ? true : false; }
  get GT() { return ((this._reg[15] >> 4) & 0x1) ? true : false; }
  get LT() { return ((this._reg[15] >> 5) & 0x1) ? true : false; }
  get P() { return ((this._reg[15] >> 6) & 0x1) ? true : false; }
  get T() { return ((this._reg[15] >> 7) & 0x1) ? true : false; }

  // jscs:disable validateIndentation
  set Y(v) { if (v) this._reg[15] |= (1 << 0); else this._reg[15] &= ~(1 << 0); }
  set V(v) { if (v) this._reg[15] |= (1 << 1); else this._reg[15] &= ~(1 << 1); }
  set Z(v) { if (v) this._reg[15] |= (1 << 2); else this._reg[15] &= ~(1 << 2); }
  set S(v) { if (v) this._reg[15] |= (1 << 3); else this._reg[15] &= ~(1 << 3); }
  set GT(v) { if (v) this._reg[15] |= (1 << 4); else this._reg[15] &= ~(1 << 4); }
  set LT(v) { if (v) { this._reg[15] |= (1 << 5); } else { this._reg[15] &= ~(1 << 5); } }
  set P(v) { if (v) { this._reg[15] |= (1 << 6); } else { this._reg[15] &= ~(1 << 6); } }
  set T(v) { if (v) { this._reg[15] |= (1 << 7); } else { this._reg[15] &= ~(1 << 6); } }
  // jscs:enable validateIndentation


 */
}

// }}}


void
cpu_reset()
{
    for(size_t i=0; i<16; ++i) {
        reg[i] = 0;
    }
}
