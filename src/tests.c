#include "tests.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "computer.h"
#include "cpu.h"
#include "memory.h"

static void test_memory();
static void test_cpu();

void 
tests_run()
{
    test_memory();
    test_cpu();
}


// {{{ TEST EXECUTION

static void
test(uint32_t t, uint32_t expected, const char* desc)
{
    if(t == expected) {
        syslog(LOG_NOTICE, "  [\x1b[32mok\x1b[0m] %s", desc);
    } else {
        syslog(LOG_NOTICE, "  [\x1b[31merror\x1b[0m] %s (expected 0x%X, got 0x%X)", desc, expected, t);
        abort();
    }
}

// }}}

//--------------------------------------------------------------

// {{{ TEST MEMORY

static void
test_memory()
{
    syslog(LOG_NOTICE, "[[[ MEMORY ]]]");

    memory_set(0x12, 0xAF);
    test(memory_get(0x12), 0xAF, "memory_set");

    memory_set32(0x0, 0x12345678);
    test(memory_get(0x0), 0x78, "memory_set32 (byte 0)");
    test(memory_get(0x1), 0x56, "memory_set32 (byte 1)");
    test(memory_get(0x2), 0x34, "memory_set32 (byte 2)");
    test(memory_get(0x3), 0x12, "memory_set32 (byte 3)");

    memory_set32(SYSTEM_AREA, 0x1000);
    test(memory_offset(), 0x1000, "offset memory location");
    test(memory_get32(SYSTEM_AREA), 0x1000, "offset from memory");

    memory_set(0x12, 0xAF);
    test(memory_get(0x12), 0xAF, "get from offset");
    test(memory_get_direct()[0x1012], 0xAF, "get direct from offset");
}

// }}}

//--------------------------------------------------------------

// {{{ TEST CPU

// {{{ opcode execution

typedef enum { NONE, REG, V8, V16, V32, INDREG, INDV32 } ParType;

static struct {
    uint8_t opcode;
    const char *name;
    ParType par1, par2;
} opcodes[] = {
    // opcode list {{{
    // movement
    { 0x01, "mov",  REG, REG    },
    { 0x02, "mov",  REG, V8     },
    { 0x03, "mov",  REG, V16    },
    { 0x04, "mov",  REG, V32    },
    { 0x05, "movb", REG, INDREG },
    { 0x06, "movb", REG, INDV32 },
    { 0x07, "movw", REG, INDREG },
    { 0x08, "movw", REG, INDV32 },
    { 0x09, "movd", REG, INDREG },
    { 0x0A, "movd", REG, INDV32 },

    { 0x0B, "movb", INDREG, REG    },
    { 0x0C, "movb", INDREG, V8     },
    { 0x0D, "movb", INDREG, INDREG },
    { 0x0E, "movb", INDREG, INDV32 },
    { 0x0F, "movw", INDREG, REG    },
    { 0x1A, "movw", INDREG, V16    },
    { 0x1B, "movw", INDREG, INDREG },
    { 0x1C, "movw", INDREG, INDV32 },
    { 0x1D, "movd", INDREG, REG    },
    { 0x1E, "movd", INDREG, V32    },
    { 0x1F, "movd", INDREG, INDREG },
    { 0x20, "movd", INDREG, INDV32 },

    { 0x21, "movb", INDV32, REG    },
    { 0x22, "movb", INDV32, V8     },
    { 0x23, "movb", INDV32, INDREG },
    { 0x24, "movb", INDV32, INDV32 },
    { 0x25, "movw", INDV32, REG    },
    { 0x26, "movw", INDV32, V16    },
    { 0x27, "movw", INDV32, INDREG },
    { 0x28, "movw", INDV32, INDV32 },
    { 0x29, "movd", INDV32, REG    },
    { 0x2A, "movd", INDV32, V32    },
    { 0x2B, "movd", INDV32, INDREG },
    { 0x2C, "movd", INDV32, INDV32 },

    { 0x8A, "swap", REG, REG },

    // logic
    { 0x2D, "or",  REG, REG  },
    { 0x2E, "or",  REG, V8   },
    { 0x2F, "or",  REG, V16  },
    { 0x30, "or",  REG, V32  },
    { 0x31, "xor", REG, REG  },
    { 0x32, "xor", REG, V8   },
    { 0x33, "xor", REG, V16  },
    { 0x34, "xor", REG, V32  },
    { 0x35, "and", REG, REG  },
    { 0x36, "and", REG, V8   },
    { 0x37, "and", REG, V16  },
    { 0x38, "and", REG, V32  },
    { 0x39, "shl", REG, REG  },
    { 0x3A, "shl", REG, V8   },
    { 0x3D, "shr", REG, REG  },
    { 0x3E, "shr", REG, V8   },
    { 0x41, "not", REG, NONE },

    // arithmetic
    { 0x42, "add",  REG, REG  },
    { 0x43, "add",  REG, V8   },
    { 0x44, "add",  REG, V16  },
    { 0x45, "add",  REG, V32  },
    { 0x46, "sub",  REG, REG  },
    { 0x47, "sub",  REG, V8   },
    { 0x48, "sub",  REG, V16  },
    { 0x49, "sub",  REG, V32  },
    { 0x4A, "cmp",  REG, REG  },
    { 0x4B, "cmp",  REG, V8   },
    { 0x4C, "cmp",  REG, V16  },
    { 0x4D, "cmp",  REG, V32  },
    { 0x8B, "cmp",  REG, NONE },
    { 0x4E, "mul",  REG, REG  },
    { 0x4F, "mul",  REG, V8   },
    { 0x50, "mul",  REG, V16  },
    { 0x51, "mul",  REG, V32  },
    { 0x52, "idiv", REG, REG  },
    { 0x53, "idiv", REG, V8   },
    { 0x54, "idiv", REG, V16  },
    { 0x55, "idiv", REG, V32  },
    { 0x56, "mod",  REG, REG  },
    { 0x57, "mod",  REG, V8   },
    { 0x58, "mod",  REG, V16  },
    { 0x59, "mod",  REG, V32  },
    { 0x5A, "inc",  REG, NONE },
    { 0x5B, "dec",  REG, NONE },

    // jumps
    { 0x5C, "bz",   REG,  NONE },
    { 0x5D, "bz",   V32,  NONE },
    { 0x5C, "beq",  REG,  NONE },
    { 0x5D, "beq",  V32,  NONE },
    { 0x5E, "bnz",  REG,  NONE },
    { 0x5F, "bnz",  V32,  NONE },
    { 0x60, "bneg", REG,  NONE },
    { 0x61, "bneg", V32,  NONE },
    { 0x62, "bpos", REG,  NONE },
    { 0x63, "bpos", V32,  NONE },
    { 0x64, "bgt",  REG,  NONE },
    { 0x65, "bgt",  V32,  NONE },
    { 0x66, "bgte", REG,  NONE },
    { 0x67, "bgte", V32,  NONE },
    { 0x68, "blt",  REG,  NONE },
    { 0x69, "blt",  V32,  NONE },
    { 0x6A, "blte", REG,  NONE },
    { 0x6B, "blte", V32,  NONE },
    { 0x6C, "bv",   REG,  NONE },
    { 0x6D, "bv",   V32,  NONE },
    { 0x6E, "bnv",  REG,  NONE },
    { 0x6F, "bnv",  V32,  NONE },
    { 0x70, "jmp",  REG,  NONE },
    { 0x71, "jmp",  V32,  NONE },
    { 0x72, "jsr",  REG,  NONE },
    { 0x73, "jsr",  V32,  NONE },
    { 0x74, "ret",  NONE, NONE },
    { 0x75, "sys",  REG,  NONE },
    { 0x76, "sys",  V8,   NONE },
    { 0x77, "iret", NONE, NONE },
    { 0x86, "sret", NONE, NONE },

    // stack
    { 0x78, "pushb",  REG,  NONE },
    { 0x79, "pushb",  V8,   NONE },
    { 0x7A, "pushw",  REG,  NONE },
    { 0x7B, "pushw",  V16,  NONE },
    { 0x7C, "pushd",  REG,  NONE },
    { 0x7D, "pushd",  V32,  NONE },
    { 0x7E, "push.a", NONE, NONE },
    { 0x7F, "popb",   REG,  NONE },
    { 0x80, "popw",   REG,  NONE },
    { 0x81, "popd",   REG,  NONE },
    { 0x82, "pop.a",  NONE, NONE },
    { 0x83, "popx",   REG,  NONE },
    { 0x84, "popx",   V8,   NONE },
    { 0x85, "popx",   V16,  NONE },

    // other
    { 0x87, "nop",  NONE, NONE },
    { 0x88, "halt", NONE, NONE },
    { 0x89, "dbg",  NONE, NONE },
    // }}}
};

static const char* registers[] = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "FP", "SP", "PC", "FL" };

static ParType cpu_op_type(const char* par) {{{
    if(par == NULL) {
        return NONE;
    } else if(par[0] == '[') {
        if(isalpha(par[1])) {
            return INDREG;
        } else {
            return INDV32;
        }
    } else if(isalpha(par[0])) {
        return REG;
    } else {
        uint32_t v = (uint32_t)strtoll(par, NULL, 0);
        if(v <= 0xFF) {
            return V8;
        } else if(v <= 0xFFFF) {
            return V16;
        } else {
            return V32;
        }
    }
}}}

static size_t cpu_op_add_value(char* par, ParType tp, uint8_t* data, size_t pos) {{{
    if(!par) {
        return 0;
    }

    // remove brackets
    if(par[0] == '[') {
        ++par;
        par[strlen(par)-1] = '\0';
    }

    // choose according to type
    switch(tp) {
        case NONE:
            return 0;
        case REG:
        case INDREG:
            for(uint8_t i=0; i<sizeof(registers)/sizeof(registers[0]); ++i) {
                if(strcmp(registers[i], par) == 0) {
                    data[pos] = i;
                    return pos + 1;
                }
            }
            abort();
        case V8:
            data[pos] = (uint8_t)strtol(par, NULL, 0);
            return pos + 1;
        case V16: {
                uint16_t v = (uint16_t)strtoll(par, NULL, 0);
                data[pos++] = (uint8_t)(v & 0xFF);
                data[pos++] = (uint8_t)(v >> 8);
            }
            return pos;
        case INDV32:
        case V32: {
                uint32_t v = (uint32_t)strtoll(par, NULL, 0);
                data[pos++] = (uint8_t)(v & 0xFF);
                data[pos++] = (uint8_t)((v >> 8) & 0xFF);
                data[pos++] = (uint8_t)((v >> 16) & 0xFF);
                data[pos++] = (uint8_t)((v >> 24) & 0xFF);
            }
            return pos;
        default:
            abort();
    }
}}}

static void
cpu_add(const char* code)
{
    char *tmp = strdup(code);
    char *opcode = strtok(tmp, " "),
         *par1 = strtok(NULL, ", "),
         *par2 = strtok(NULL, ", ");

    // find value bytes
    uint8_t ram[25] = { 0 };
    size_t  pos = 1;
    ParType par1type = cpu_op_type(par1),
            par2type = cpu_op_type(par2);
    pos = cpu_op_add_value(par1, par1type, ram, pos);
    pos = cpu_op_add_value(par2, par2type, ram, pos);

    // if both parameters are registers, join them in one byte
    if((par1type == REG || par1type == INDREG) && (par2type == REG || par2type == INDREG)) {
        ram[1] |= (uint8_t)(ram[2] << 4);
        ram[2] = 0;
    }

    // find opcode
    for(size_t i=0; i < sizeof(opcodes) / sizeof(opcodes[0]); ++i) {
        if(strcmp(opcodes[i].name, opcode) == 0 
                && opcodes[i].par1 == par1type && opcodes[i].par2 == par2type) {
            ram[0] = opcodes[i].opcode;
            for(size_t j=0; j<sizeof(ram); ++j) {
                memory_set((uint32_t)j, ram[j]);
            }
            goto end;
        }
    }

    syslog(LOG_ERR, "Opcode not found");
    abort();

end:
    free(tmp);
}

#define EXEC(precode, assembly, tst, expected)  \
    computer_reset();                           \
    { precode; }                                \
    cpu_add(assembly);                          \
    cpu_step();                                 \
    { char buf[255]; snprintf(buf, sizeof(buf), "(op 0x%02X) %s", memory_get(0), assembly); \
    test(tst, expected, buf); }

// }}}

static void
test_cpu_basic()
{
    syslog(LOG_NOTICE, "* BASIC TESTS");

    computer_reset();
    cpu_setregister(A, 0x42);
    test(cpu_register(A), 0x42, "A = 0x42");
    test(cpu_register(B), 0x00, "B = 0x00");

    cpu_setflag(S, true);
    test(cpu_flag(S), true, "S = true");
    test(cpu_register(FL), 0b1000, "FL = 0b1000");

    computer_reset();
    cpu_add("mov A, B");
    test(memory_get(0), 0x1, "[0] = 0x1");
    test(memory_get(1), 0x10, "[1] = 0x10");
}


static void
test_cpu_MOV()
{
    syslog(LOG_NOTICE, "* simple movement (MOV)");

    EXEC(cpu_setregister(B, 0x42), "mov A, B", cpu_register(A), 0x42);
    test(cpu_register(PC), 2, "PC = 2");

    EXEC({}, "mov A, 0x34", cpu_register(A), 0x34);
    EXEC({}, "mov A, 0x1234", cpu_register(A), 0x1234);
    EXEC({}, "mov A, 0xFABC1234", cpu_register(A), 0xFABC1234);

    EXEC({}, "mov A, 0", cpu_flag(Z), true);
    test(cpu_flag(P), true, "P=1");
    test(cpu_flag(S), false, "P=0");

    EXEC({}, "mov A, 0xF0000001", cpu_flag(Z), false);
    test(cpu_flag(P), false, "P=1");
    test(cpu_flag(S), true, "P=0");
}


static void
test_cpu_MOVB()
{
    syslog(LOG_NOTICE, "* 8-bit movement (MOVB)");

    EXEC({ cpu_setregister(B, 0x100); memory_set(cpu_register(B), 0xAB); },
            "movb A, [B]", cpu_register(A), 0xAB);
    EXEC({ memory_set(0x1000, 0xAB); },
            "movb A, [0x1000]", cpu_register(A), 0xAB);
    EXEC({ cpu_setregister(A, 0x64); cpu_setregister(C, 0x32); },
            "movb [C], A", memory_get(0x32), 0x64);
    EXEC({ cpu_setregister(A, 0x64); },
            "movb [A], 0xFA", memory_get(0x64), 0xFA);
    EXEC({ cpu_setregister(A, 0x32); cpu_setregister(B, 0x64); memory_set(0x64, 0xFF); },
            "movb [A], [B]", memory_get(0x32), 0xFF);
    EXEC({ cpu_setregister(A, 0x32); memory_set(0x6420, 0xFF); },
            "movb [A], [0x6420]", memory_get(0x32), 0xFF);
    EXEC({ cpu_setregister(A, 0xAC32); },
            "movb [0x64], A", memory_get(0x64), 0x32);
    EXEC({},
            "movb [0x64], 0xF0", memory_get(0x64), 0xF0);
    EXEC({ cpu_setregister(A, 0xF000); memory_set(0xF000, 0x42); },
            "movb [0xCC64], [A]", memory_get(0xCC64), 0x42);
    EXEC({ memory_set32(0xABF0, 0x1234); memory_set(0x1234, 0x3F); },
            "movb [0x64], [0xABF0]", memory_get(0x64), 0x3F);
}


static void
test_cpu_MOVW()
{
    syslog(LOG_NOTICE, "* 16-bit movement (MOVW)");

    EXEC({ cpu_setregister(B, 0x1000); memory_set16(cpu_register(B), 0xABCD); },
            "movw A, [B]", cpu_register(A), 0xABCD);
    EXEC({ memory_set16(0x1000, 0xABCD); },
            "movw A, [0x1000]", cpu_register(A), 0xABCD);
    EXEC({ cpu_setregister(A, 0x6402); },
            "movw [A], A", memory_get16(0x6402), 0x6402);
    EXEC({ cpu_setregister(A, 0x64); },
            "movw [A], 0xFABA", memory_get16(0x64), 0xFABA);
    EXEC({ cpu_setregister(A, 0x32CC); cpu_setregister(B, 0x64); memory_set16(0x64, 0xFFAB); },
            "movw [A], [B]", memory_get16(0x32CC), 0xFFAB);
    EXEC({ cpu_setregister(A, 0x32); memory_set16(0x6420, 0xFFAB); },
            "movw [A], [0x6420]", memory_get16(0x32), 0xFFAB);
    EXEC({ cpu_setregister(A, 0xAB32AC); },
            "movw [0x64], A", memory_get16(0x64), 0x32AC);
    EXEC({},
            "movw [0x64], 0xF0FA", memory_get16(0x64), 0xF0FA);
    EXEC({ cpu_setregister(A, 0xF000); memory_set16(0xF000, 0x4245); },
            "movw [0xCC64], [A]", memory_get16(0xCC64), 0x4245);
    EXEC({ memory_set32(0xABF0, 0x1234); memory_set16(0x1234, 0x3F54); },
            "movw [0x64], [0xABF0]", memory_get16(0x64), 0x3F54);
}


static void
test_cpu_MOVD()
{
    syslog(LOG_NOTICE, "* 32-bit movement (MOVD)");

    EXEC({ cpu_setregister(B, 0x1000); memory_set32(cpu_register(B), 0xABCDEF01); },
            "movd A, [B]", cpu_register(A), 0xABCDEF01);
    EXEC({ memory_set32(0x1000, 0xABCDEF01); },
            "movd A, [0x1000]", cpu_register(A), 0xABCDEF01);
    EXEC({ cpu_setregister(A, 0x16402); },
            "movd [A], A", memory_get32(0x16402), 0x16402);
    EXEC({ cpu_setregister(A, 0x64); },
            "movd [A], 0xFABA1122", memory_get32(0x64), 0xFABA1122);
    EXEC({ cpu_setregister(A, 0x32CC); cpu_setregister(B, 0x64); memory_set32(0x64, 0xFFAB5678); },
            "movd [A], [B]", memory_get32(0x32CC), 0xFFAB5678);
    EXEC({ cpu_setregister(A, 0x32); memory_set32(0x6420, 0xFFAC9876); },
            "movd [A], [0x6420]", memory_get32(0x32), 0xFFAC9876);
    EXEC({ cpu_setregister(A, 0xAB32AC44); },
            "movd [0x64], A", memory_get32(0x64), 0xAB32AC44);
    EXEC({},
            "movd [0x64], 0xF0FA1234", memory_get32(0x64), 0xF0FA1234);
    EXEC({ cpu_setregister(A, 0xF000); memory_set32(0xF000, 0x4245AABB); },
            "movd [0xCC64], [A]", memory_get32(0xCC64), 0x4245AABB);
    EXEC({ memory_set32(0xABF0, 0x1234); memory_set32(0x1234, 0x3F54FABC); },
            "movd [0x64], [0xABF0]", memory_get32(0x64), 0x3F54FABC);
}


static void
test_cpu_SWAP()
{
    syslog(LOG_NOTICE, "* swap registers (SWAP)");
    
    EXEC({ cpu_setregister(A, 0xA); cpu_setregister(B, 0xB); }, 
            "swap A, B", cpu_register(A), 0xB);
    test(cpu_register(B), 0xA, "B = 0xA");
}


static void
test_cpu_logical()
{
    syslog(LOG_NOTICE, "* logical operations");

    EXEC({ cpu_setregister(A, 0b1010); cpu_setregister(B, 0b1100); }, "or A, B", cpu_register(A), 0b1110);
    test(cpu_flag(S), false, "S = 0");
    test(cpu_flag(P), true , "P = 1");
    test(cpu_flag(Z), false, "Z = 0");
    test(cpu_flag(Y), false, "Y = 0");
    test(cpu_flag(V), false, "V = 0");

    EXEC({ cpu_setregister(A, 0b11); }, "or A, 0x4", cpu_register(A), 0b111);
    EXEC({ cpu_setregister(A, 0b111); }, "or A, 0x4000", cpu_register(A), 0x4007);
    EXEC({ cpu_setregister(A, 0x10800000); }, "or A, 0x2A426653", cpu_register(A), 0x3AC26653);

    EXEC({ cpu_setregister(A, 0b1010); cpu_setregister(B, 0b1100); }, "xor A, B", cpu_register(A), 0b110);
    EXEC({ cpu_setregister(A, 0b11); }, "xor A, 0x4", cpu_register(A), 0b111);
    EXEC({ cpu_setregister(A, 0xFF0); }, "xor A, 0xFF00", cpu_register(A), 0xF0F0);
    EXEC({ cpu_setregister(A, 0x148ABD12); }, "xor A, 0x2A426653", cpu_register(A), 0x3EC8DB41);

    EXEC({ cpu_setregister(A, 0b11); cpu_setregister(B, 0b1100); }, "and A, B", cpu_register(A), 0);
    test(cpu_flag(Z), true, "Z = 1");
    EXEC({ cpu_setregister(A, 0b11); }, "and A, 0x7", cpu_register(A), 0b11);
    EXEC({ cpu_setregister(A, 0xFF0); }, "and A, 0xFF00", cpu_register(A), 0xF00);
    EXEC({ cpu_setregister(A, 0x148ABD12); }, "and A, 0x2A426653", cpu_register(A), 0x22412);

    EXEC({ cpu_setregister(A, 0b10101010); cpu_setregister(B, 4); }, "shl A, B", cpu_register(A), 0b101010100000);
    EXEC({ cpu_setregister(A, 0b10101010); }, "shl A, 4", cpu_register(A), 0b101010100000);

    EXEC({ cpu_setregister(A, 0b10101010); cpu_setregister(B, 4); }, "shr A, B", cpu_register(A), 0b1010);
    EXEC({ cpu_setregister(A, 0b10101010); }, "shr A, 4", cpu_register(A), 0b1010);

    EXEC({ cpu_setregister(A, 0b11001010); }, "not A", cpu_register(A), 0b11111111111111111111111100110101);
}

/*{{{

  //
  // integer math
  //

  t.comment('Integer arithmetic');
  
  s = opc('add A, B', () => { cpu.A = 0x12; cpu.B = 0x20; });
  t.equal(cpu.A, 0x32, s);
  
  s = opc('add A, 0x20', () => cpu.A = 0x12);
  t.equal(cpu.A, 0x32, s);

  s = opc('add A, 0x20', () => { cpu.A = 0x12, cpu.Y = true; });
  t.equal(cpu.A, 0x33, 'add A, 0x20 (with carry)');

  s = opc('add A, 0x2000', () => cpu.A = 0x12);
  t.equal(cpu.A, 0x2012, s);

  s = opc('add A, 0xF0000000', () => cpu.A = 0x10000012);
  t.equal(cpu.A, 0x12, s);
  t.true(cpu.Y, "cpu.Y == 1");

  s = opc('sub A, B', () => { cpu.A = 0x30; cpu.B = 0x20; });
  t.equal(cpu.A, 0x10, s);
  t.false(cpu.S, 'cpu.S == 0');

  s = opc('sub A, B', () => { cpu.A = 0x20; cpu.B = 0x30; });
  t.equal(cpu.A, 0xFFFFFFF0, 'sub A, B (negative)');
  t.true(cpu.S, 'cpu.S == 1');

  s = opc('sub A, 0x20', () => cpu.A = 0x22);
  t.equal(cpu.A, 0x2, s);

  s = opc('sub A, 0x20', () => { cpu.A = 0x22; cpu.Y = true; });
  t.equal(cpu.A, 0x1, 'sub A, 0x20 (with carry)');

  s = opc('sub A, 0x2000', () => cpu.A = 0x12);
  t.equal(cpu.A, 0xFFFFE012, s);
  t.true(cpu.S, 'cpu.S == 1');
  t.true(cpu.Y, 'cpu.Y == 1');

  s = opc('sub A, 0xF0000000', () => cpu.A = 0x10000012);
  t.equal(cpu.A, 0x20000012, s);
  t.true(cpu.Y, 'cpu.Y == 1');

  s = opc('cmp A, B');
  t.true(cpu.Z, s);

  s = opc('cmp A, 0x12');
  t.true(cpu.LT && !cpu.GT, s);

  s = opc('cmp A, 0x1234', () => cpu.A = 0x6000);
  t.true(!cpu.LT && cpu.GT, s);

  s = opc('cmp A, 0x12345678', () => cpu.A = 0xF0000000);
  t.true(!cpu.LT && cpu.GT, s);  // because of the signal!

  s = opc('cmp A', () => cpu.A = 0x0);
  t.true(cpu.Z, s);

  s = opc('mul A, B', () => { cpu.A = 0xF0; cpu.B = 0xF000; });
  t.equal(cpu.A, 0xE10000, s);

  s = opc('mul A, 0x12', () => cpu.A = 0x1234);
  t.equal(cpu.A, 0x147A8, s);

  s = opc('mul A, 0x12AF', () => cpu.A = 0x1234);
  t.equal(cpu.A, 0x154198C, s);
  t.false(cpu.V, 'cpu.V == 0');

  s = opc('mul A, 0x12AF87AB', () => cpu.A = 0x1234);
  t.equal(cpu.A, 0x233194BC, s);
  t.true(cpu.V, 'cpu.V == 1');

  s = opc('idiv A, B', () => { cpu.A = 0xF000; cpu.B = 0xF0; });
  t.equal(cpu.A, 0x100, s);

  s = opc('idiv A, 0x12', () => cpu.A = 0x1234);
  t.equal(cpu.A, 0x102, s);

  s = opc('idiv A, 0x2AF', () => cpu.A = 0x1234);
  t.equal(cpu.A, 0x6, s);

  s = opc('idiv A, 0x12AF', () => cpu.A = 0x123487AB);
  t.equal(cpu.A, 0xF971, s);

  s = opc('mod A, B', () => { cpu.A = 0xF000; cpu.B = 0xF0; });
  t.equal(cpu.A, 0x0, s);
  t.true(cpu.Z, 'cpu.Z == 1');

  s = opc('mod A, 0x12', () => cpu.A = 0x1234);
  t.equal(cpu.A, 0x10, s);

  s = opc('mod A, 0x2AF', () => cpu.A = 0x1234);
  t.equal(cpu.A, 0x21A, s);

  s = opc('mod A, 0x12AF', () => cpu.A = 0x123487AB);
  t.equal(cpu.A, 0x116C, s);

  s = opc('inc A', () => cpu.A = 0x42);
  t.equal(cpu.A, 0x43, s);

  s = opc('inc A', () => cpu.A = 0xFFFFFFFF);
  t.equal(cpu.A, 0x0, 'inc A (overflow)');
  t.true(cpu.Y, 'cpu.Y == 1');
  t.true(cpu.Z, 'cpu.Z == 1');

  s = opc('dec A', () => cpu.A = 0x42);
  t.equal(cpu.A, 0x41, s);

  s = opc('dec A', () => cpu.A = 0x0);
  t.equal(cpu.A, 0xFFFFFFFF, 'dec A (underflow)');
  t.false(cpu.Z, 'cpu.Z == 0');

  // 
  // branches
  //

  t.comment('Branch operations');

  s = opc('bz A', () => { cpu.Z = true; cpu.A = 0x1000; });
  t.equal(cpu.PC, 0x1000, s);

  s = opc('bz A', () => { cpu.A = 0x1000; });
  t.equal(cpu.PC, 0x2, 'bz A (false)');

  s = opc('bz 0x1000', () => cpu.Z = true);
  t.equal(cpu.PC, 0x1000, s);

  s = opc('bnz A', () => cpu.A = 0x1000);
  t.equal(cpu.PC, 0x1000, s);

  s = opc('bnz 0x1000');
  t.equal(cpu.PC, 0x1000, s);

  s = opc('bneg A', () => { cpu.S = true; cpu.A = 0x1000; });
  t.equal(cpu.PC, 0x1000, s);

  s = opc('bneg A', () => { cpu.A = 0x1000; });
  t.equal(cpu.PC, 0x2, 'bneg A (false)');

  s = opc('bneg 0x1000', () => cpu.S = true);
  t.equal(cpu.PC, 0x1000, s);

  s = opc('bpos A', () => cpu.A = 0x1000);
  t.equal(cpu.PC, 0x1000, s);

  s = opc('bpos 0x1000');
  t.equal(cpu.PC, 0x1000, s);

  s = opc('jmp 0x12345678');
  t.equal(cpu.PC, 0x12345678, s);
  

  // 
  // stack
  //

  t.comment('Stack operations');

  mb.reset();
  cpu.SP = 0xFFF; 
  cpu.A = 0xABCDEF12;

  mb.setArray(0x0, Debugger.encode('pushb A'));
  mb.setArray(0x2, Debugger.encode('pushb 0x12'));
  mb.setArray(0x4, Debugger.encode('pushw A'));
  mb.setArray(0x6, Debugger.encode('pushd A'));

  mb.setArray(0x8, Debugger.encode('popd B'));
  mb.setArray(0xA, Debugger.encode('popw B'));
  mb.setArray(0xC, Debugger.encode('popb B'));

  mb.setArray(0xE, Debugger.encode('popx 1'));

  mb.step();
  t.equal(mb.get(0xFFF), 0x12, 'pushb A');
  t.equal(cpu.SP, 0xFFE, 'SP = 0xFFE');

  mb.step();
  t.equal(mb.get(0xFFE), 0x12, 'pushb 0x12');
  t.equal(cpu.SP, 0xFFD, 'SP = 0xFFD');

  mb.step();
  t.equal(mb.get16(0xFFC), 0xEF12, s);
  t.equal(mb.get(0xFFD), 0xEF, s);
  t.equal(mb.get(0xFFC), 0x12, s);
  t.equal(cpu.SP, 0xFFB, 'SP = 0xFFB');

  mb.step();
  t.equal(mb.get32(0xFF8), 0xABCDEF12);
  t.equal(cpu.SP, 0xFF7, 'SP = 0xFF7');

  mb.step();
  t.equal(cpu.B, 0xABCDEF12, 'popd B');

  mb.step();
  t.equal(cpu.B, 0xEF12, 'popw B');

  mb.step();
  t.equal(cpu.B, 0x12, 'popb B');

  mb.step();
  t.equal(cpu.SP, 0xFFF, 'popx 1');

  // all registers
  s = opc('push.a', () => {
    cpu.SP = 0xFFF;
    cpu.A = 0xA1B2C3E4;
    cpu.B = 0xFFFFFFFF;
  });
  t.equal(cpu.SP, 0xFCF, s);
  t.equal(mb.get32(0xFFC), 0xA1B2C3E4, 'A is saved');
  t.equal(mb.get32(0xFF8), 0xFFFFFFFF, 'B is saved');
  
  s = opc('pop.a', () => {
    cpu.SP = 0xFCF;
    mb.set32(0xFFC, 0xA1B2C3E4);
    mb.set32(0xFF8, 0xFFFFFFFF);
  });
  t.equal(cpu.SP, 0xFFF, s);
  t.equal(cpu.A, 0xA1B2C3E4, 'A is restored');
  t.equal(cpu.B, 0xFFFFFFFF, 'B is restored');

  // others
  t.comment('Others');

  opc('nop');
  
  s = opc('dbg');
  t.true(cpu.activateDebugger, s);

  s = opc('halt');
  t.true(cpu.systemHalted, s);

  s = opc('swap A, B', () => {
    cpu.A = 0xA;
    cpu.B = 0xB;
  });
  t.true(cpu.A == 0xB && cpu.B == 0xA, s);

  t.end();

});


test('CPU: subroutines and system calls', t => {

  let [mb, cpu] = makeCPU();

  // jsr
  mb.reset();
  mb.setArray(0x200, Debugger.encode('jsr 0x1234'));
  mb.setArray(0x1234, Debugger.encode('ret'));
  cpu.PC = 0x200;
  cpu.SP = 0xFFF;
  mb.step();
  t.equal(cpu.PC, 0x1234, 'jsr 0x1234');
  t.equal(mb.get(0xFFC), 0x5, '[FFC] = 0x5');
  t.equal(mb.get(0xFFD), 0x2, '[FFD] = 0x2');
  t.equal(cpu.SP, 0xFFB, 'SP = 0xFFB');
  t.equal(mb.get32(0xFFC), 0x200 + 5, 'address in stack'); 

  mb.step();
  t.equal(cpu.PC, 0x205, 'ret');
  t.equal(cpu.SP, 0xFFF, 'SP = 0xFFF');

  // sys
  mb.reset();
  cpu.SP = 0xFFF;
  mb.setArray(0, Debugger.encode('sys 2'));
  mb.set32(cpu.CPU_SYSCALL_VECT + 8, 0x1000);
  t.equal(cpu._syscallVector[2], 0x1000, 'syscall vector');
  mb.setArray(0x1000, Debugger.encode('sret'));

  mb.step();
  t.equal(cpu.PC, 0x1000, 'sys 2');
  t.equal(cpu.SP, 0xFFB, 'SP = 0xFFD');
  mb.step();
  t.equal(cpu.PC, 0x2, 'sret');
  t.equal(cpu.SP, 0xFFF, 'SP = 0xFFF');

  t.end();

});


test('CPU: interrupts', t => {

  let [mb, cpu] = makeCPU();
  cpu.T = true;
  cpu.SP = 0x800;
  mb.set32(cpu.CPU_INTERRUPT_VECT + 8, 0x1000);
  mb.setArray(0x0, Debugger.encode('movb A, [0xE0000000]'));
  mb.setArray(0x1000, Debugger.encode('iret'));

  mb.step();  // cause the exception
  t.equal(cpu.PC, 0x1000, 'interrupt called');
  t.true(cpu.T, 'interrupts disabled');

  mb.step();  // iret
  t.equal(cpu.PC, 0x6, 'iret');
  t.true(cpu.T, 'interrupts enabled');

  t.end();

});


test('CPU: invalid opcode', t => {

  let [mb, cpu] = makeCPU();
  cpu.T = true;
  mb.set32(cpu.CPU_INTERRUPT_VECT + 12, 0x1000);
  mb.set(0x0, 0xFF);
  mb.step();
  t.equal(cpu.PC, 0x1000, 'interrupt called');

  t.end();

});}}} */


static void
test_cpu()
{
    syslog(LOG_NOTICE, "[[[ CPU ]]]");

    test_cpu_basic();
    test_cpu_MOV();
    test_cpu_MOVB();
    test_cpu_MOVW();
    test_cpu_MOVD();
    test_cpu_SWAP();
    test_cpu_logical();
}

// }}}
