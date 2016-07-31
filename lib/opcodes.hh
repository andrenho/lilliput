#ifndef OPCODES_HH_
#define OPCODES_HH_

#include <array>
#include <string>
#include <vector>

enum ParameterType { REG, V8, V16, V32, INDREG, INDV32, LABEL };

struct Opcode {
    string instruction;
    vector<ParameterType> parameter;
};

static array<Opcode, 255> opcodes = {{
    // invalid
    { "", {} },                         // 0x00

    // movement
    { "mov", { REG, REG } },            // 0x01
    { "mov", { REG, V8 } },             // 0x02
    { "mov", { REG, V16 } },            // 0x03
    { "mov", { REG, V32 } },            // 0x04

    { "movb", { REG, INDREG } },        // 0x05
    { "movb", { REG, INDV32 } },        // 0x06
    { "movb", { INDREG, REG } },        // 0x07
    { "movb", { INDREG, V8 } },         // 0x08
    { "movb", { INDREG, INDREG } },     // 0x09
    { "movb", { INDREG, INDV32 } },     // 0x0A
    { "movb", { INDV32, REG } },        // 0x0B
    { "movb", { INDV32, V8 } },         // 0x0C
    { "movb", { INDV32, INDREG } },     // 0x0D
    { "movb", { INDV32, INDV32 } },     // 0x0E

    { "movw", { REG, INDREG } },        // 0x0F
    { "movw", { REG, INDV32 } },        // 0x10
    { "movw", { INDREG, REG } },        // 0x11
    { "movw", { INDREG, V16 } },        // 0x12
    { "movw", { INDREG, INDREG } },     // 0x13
    { "movw", { INDREG, INDV32 } },     // 0x14
    { "movw", { INDV32, REG } },        // 0x15
    { "movw", { INDV32, V16 } },        // 0x16
    { "movw", { INDV32, INDREG } },     // 0x17
    { "movw", { INDV32, INDV32 } },     // 0x18

    { "movd", { REG, INDREG } },        // 0x19
    { "movd", { REG, INDV32 } },        // 0x1A
    { "movd", { INDREG, REG } },        // 0x1B
    { "movd", { INDREG, V32 } },        // 0x1C
    { "movd", { INDREG, INDREG } },     // 0x1D
    { "movd", { INDREG, INDV32 } },     // 0x1E
    { "movd", { INDV32, REG } },        // 0x1F
    { "movd", { INDV32, V32 } },        // 0x20
    { "movd", { INDV32, INDREG } },     // 0x21
    { "movd", { INDV32, INDV32 } },     // 0x22

    { "swap", { REG, REG } },           // 0x23

    // logic
    { "or",  { REG, REG } },            // 0x24
    { "or",  { REG, V8 } },             // 0x25
    { "or",  { REG, V16 } },            // 0x26
    { "or",  { REG, V32 } },            // 0x27
    { "xor", { REG, REG } },            // 0x28
    { "xor", { REG, V8 } },             // 0x29
    { "xor", { REG, V16 } },            // 0x2A
    { "xor", { REG, V32 } },            // 0x2B
    { "and", { REG, REG } },            // 0x2C
    { "and", { REG, V8 } },             // 0x2D
    { "and", { REG, V16 } },            // 0x2E
    { "and", { REG, V32 } },            // 0x2F
    { "shl", { REG, REG } },            // 0x30
    { "shl", { REG, V8 } },             // 0x31
    { "shr", { REG, REG } },            // 0x32
    { "shr", { REG, V8 } },             // 0x33
    { "not", { REG, } },                // 0x34

    // arithmetic
    { "add",  { REG, REG } },           // 0x35
    { "add",  { REG, V8 } },            // 0x36
    { "add",  { REG, V16 } },           // 0x37
    { "add",  { REG, V32 } },           // 0x38
    { "sub",  { REG, REG } },           // 0x39
    { "sub",  { REG, V8 } },            // 0x3A
    { "sub",  { REG, V16 } },           // 0x3B
    { "sub",  { REG, V32 } },           // 0x3C
    { "cmp",  { REG, REG } },           // 0x3D
    { "cmp",  { REG, V8 } },            // 0x3E
    { "cmp",  { REG, V16 } },           // 0x3F
    { "cmp",  { REG, V32 } },           // 0x40
    { "cmp",  { REG, } },               // 0x41
    { "mul",  { REG, REG } },           // 0x42
    { "mul",  { REG, V8 } },            // 0x43
    { "mul",  { REG, V16 } },           // 0x44
    { "mul",  { REG, V32 } },           // 0x45
    { "idiv", { REG, REG } },           // 0x46
    { "idiv", { REG, V8 } },            // 0x47
    { "idiv", { REG, V16 } },           // 0x48
    { "idiv", { REG, V32 } },           // 0x49
    { "mod",  { REG, REG } },           // 0x4A
    { "mod",  { REG, V8 } },            // 0x4B
    { "mod",  { REG, V16 } },           // 0x4C
    { "mod",  { REG, V32 } },           // 0x4D
    { "inc",  { REG, } },               // 0x4E
    { "dec",  { REG, } },               // 0x4F

    // jumps
    { "bz",   { REG, } },               // 0x50
    { "bz",   { V32, } },               // 0x51
    { "bnz",  { REG, } },               // 0x52
    { "bnz",  { V32, } },               // 0x53
    { "bneg", { REG, } },               // 0x54
    { "bneg", { V32, } },               // 0x55
    { "bpos", { REG, } },               // 0x56
    { "bpos", { V32, } },               // 0x57
    { "bgt",  { REG, } },               // 0x58
    { "bgt",  { V32, } },               // 0x59
    { "bgte", { REG, } },               // 0x5A
    { "bgte", { V32, } },               // 0x5B
    { "blt",  { REG, } },               // 0x5C
    { "blt",  { V32, } },               // 0x5D
    { "blte", { REG, } },               // 0x5E
    { "blte", { V32, } },               // 0x5F
    { "bv",   { REG, } },               // 0x60
    { "bv",   { V32, } },               // 0x61
    { "bnv",  { REG, } },               // 0x62
    { "bnv",  { V32, } },               // 0x63

    { "jmp",  { REG, } },               // 0x64
    { "jmp",  { V32, } },               // 0x65
    { "jsr",  { REG, } },               // 0x66
    { "jsr",  { V32, } },               // 0x67
    { "ret",  {} },                     // 0x68
    { "iret", {} },                     // 0x69

    // stack
    { "pushb",  { REG, } },             // 0x6A
    { "pushb",  { V8, } },              // 0x6B
    { "pushw",  { REG, } },             // 0x6C
    { "pushw",  { V16, } },             // 0x6D
    { "pushd",  { REG, } },             // 0x6E
    { "pushd",  { V32, } },             // 0x6F
    { "push.a", {} },                   // 0x70
    { "popb",   { REG, } },             // 0x71
    { "popw",   { REG, } },             // 0x72
    { "popd",   { REG, } },             // 0x73
    { "pop.a",  {} },                   // 0x74
    { "popx",   { REG, } },             // 0x75
    { "popx",   { V8, } },              // 0x76
    { "popx",   { V16, } },             // 0x77

    // other
    { "nop",  {} },                     // 0x78
    { "halt", {} },                     // 0x79
    { "dbg",  {} },                     // 0x7A
}};


#endif
