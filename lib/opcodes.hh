#ifndef OPCODES_HH_
#define OPCODES_HH_

#include <array>
#include <string>
#include <vector>

namespace luisavm {

enum ParameterType { REG, V8, V16, V32, INDREG, INDV32 };

enum Instruction {
    MOV, MOVB, MOVW, MOVD, SWAP,
    OR, XOR, AND, SHL, SHR, NOT,
    ADD, SUB, CMP, MUL, IDIV, MOD, INC, DEC,
    BZ, BNZ, BNEG, BPOS, BGT, BGTE, BLT, BLTE, BV, BNV,
    JMP, JSR, RET,
    PUSHB, PUSHW, PUSHD, PUSH_A, POPB, POPW, POPD, POP_A, POPX,
    NOP,
    INVALID 
};

struct Opcode {
    string                description;
    Instruction           instruction;
    vector<ParameterType> parameter;
};

static array<Opcode, 255> opcodes = {{
    // invalid
    { "", INVALID, {} },                      // 0x00

    // movement
    { "mov", MOV, { REG, REG } },             // 0x01
    { "mov", MOV, { REG, V8 } },              // 0x02
    { "mov", MOV, { REG, V16 } },             // 0x03
    { "mov", MOV, { REG, V32 } },             // 0x04

    { "movb", MOVB, { REG, INDREG } },        // 0x05
    { "movb", MOVB, { REG, INDV32 } },        // 0x06
    { "movb", MOVB, { INDREG, REG } },        // 0x07
    { "movb", MOVB, { INDREG, V8 } },         // 0x08
    { "movb", MOVB, { INDREG, INDREG } },     // 0x09
    { "movb", MOVB, { INDREG, INDV32 } },     // 0x0A
    { "movb", MOVB, { INDV32, REG } },        // 0x0B
    { "movb", MOVB, { INDV32, V8 } },         // 0x0C
    { "movb", MOVB, { INDV32, INDREG } },     // 0x0D
    { "movb", MOVB, { INDV32, INDV32 } },     // 0x0E

    { "movw", MOVW, { REG, INDREG } },        // 0x0F
    { "movw", MOVW, { REG, INDV32 } },        // 0x10
    { "movw", MOVW, { INDREG, REG } },        // 0x11
    { "movw", MOVW, { INDREG, V16 } },        // 0x12
    { "movw", MOVW, { INDREG, INDREG } },     // 0x13
    { "movw", MOVW, { INDREG, INDV32 } },     // 0x14
    { "movw", MOVW, { INDV32, REG } },        // 0x15
    { "movw", MOVW, { INDV32, V16 } },        // 0x16
    { "movw", MOVW, { INDV32, INDREG } },     // 0x17
    { "movw", MOVW, { INDV32, INDV32 } },     // 0x18

    { "movd", MOVD, { REG, INDREG } },        // 0x19
    { "movd", MOVD, { REG, INDV32 } },        // 0x1A
    { "movd", MOVD, { INDREG, REG } },        // 0x1B
    { "movd", MOVD, { INDREG, V32 } },        // 0x1C
    { "movd", MOVD, { INDREG, INDREG } },     // 0x1D
    { "movd", MOVD, { INDREG, INDV32 } },     // 0x1E
    { "movd", MOVD, { INDV32, REG } },        // 0x1F
    { "movd", MOVD, { INDV32, V32 } },        // 0x20
    { "movd", MOVD, { INDV32, INDREG } },     // 0x21
    { "movd", MOVD, { INDV32, INDV32 } },     // 0x22

    { "swap", SWAP, { REG, REG } },           // 0x23

    // logic
    { "or",  OR,  { REG, REG } },             // 0x24
    { "or",  OR,  { REG, V8 } },              // 0x25
    { "or",  OR,  { REG, V16 } },             // 0x26
    { "or",  OR,  { REG, V32 } },             // 0x27
    { "xor", XOR, { REG, REG } },             // 0x28
    { "xor", XOR, { REG, V8 } },              // 0x29
    { "xor", XOR, { REG, V16 } },             // 0x2A
    { "xor", XOR, { REG, V32 } },             // 0x2B
    { "and", AND, { REG, REG } },             // 0x2C
    { "and", AND, { REG, V8 } },              // 0x2D
    { "and", AND, { REG, V16 } },             // 0x2E
    { "and", AND, { REG, V32 } },             // 0x2F
    { "shl", SHL, { REG, REG } },             // 0x30
    { "shl", SHL, { REG, V8 } },              // 0x31
    { "shr", SHR, { REG, REG } },             // 0x32
    { "shr", SHR, { REG, V8 } },              // 0x33
    { "not", NOT, { REG, } },                 // 0x34

    // arithmetic
    { "add",  ADD,  { REG, REG } },           // 0x35
    { "add",  ADD,  { REG, V8 } },            // 0x36
    { "add",  ADD,  { REG, V16 } },           // 0x37
    { "add",  ADD,  { REG, V32 } },           // 0x38
    { "sub",  SUB,  { REG, REG } },           // 0x39
    { "sub",  SUB,  { REG, V8 } },            // 0x3A
    { "sub",  SUB,  { REG, V16 } },           // 0x3B
    { "sub",  SUB,  { REG, V32 } },           // 0x3C
    { "cmp",  CMP,  { REG, REG } },           // 0x3D
    { "cmp",  CMP,  { REG, V8 } },            // 0x3E
    { "cmp",  CMP,  { REG, V16 } },           // 0x3F
    { "cmp",  CMP,  { REG, V32 } },           // 0x40
    { "cmp",  CMP,  { REG, } },               // 0x41
    { "mul",  MUL,  { REG, REG } },           // 0x42
    { "mul",  MUL,  { REG, V8 } },            // 0x43
    { "mul",  MUL,  { REG, V16 } },           // 0x44
    { "mul",  MUL,  { REG, V32 } },           // 0x45
    { "idiv", IDIV, { REG, REG } },           // 0x46
    { "idiv", IDIV, { REG, V8 } },            // 0x47
    { "idiv", IDIV, { REG, V16 } },           // 0x48
    { "idiv", IDIV, { REG, V32 } },           // 0x49
    { "mod",  MOD,  { REG, REG } },           // 0x4A
    { "mod",  MOD,  { REG, V8 } },            // 0x4B
    { "mod",  MOD,  { REG, V16 } },           // 0x4C
    { "mod",  MOD,  { REG, V32 } },           // 0x4D
    { "inc",  INC,  { REG, } },               // 0x4E
    { "dec",  DEC,  { REG, } },               // 0x4F

    // jumps
    { "bz",   BZ,   { REG, } },               // 0x50
    { "bz",   BZ,   { V32, } },               // 0x51
    { "bnz",  BNZ,  { REG, } },               // 0x52
    { "bnz",  BNZ,  { V32, } },               // 0x53
    { "bneg", BNEG, { REG, } },               // 0x54
    { "bneg", BNEG, { V32, } },               // 0x55
    { "bpos", BPOS, { REG, } },               // 0x56
    { "bpos", BPOS, { V32, } },               // 0x57
    { "bgt",  BGT,  { REG, } },               // 0x58
    { "bgt",  BGT,  { V32, } },               // 0x59
    { "bgte", BGTE, { REG, } },               // 0x5A
    { "bgte", BGTE, { V32, } },               // 0x5B
    { "blt",  BLT,  { REG, } },               // 0x5C
    { "blt",  BLT,  { V32, } },               // 0x5D
    { "blte", BLTE, { REG, } },               // 0x5E
    { "blte", BLTE, { V32, } },               // 0x5F
    { "bv",   BV,   { REG, } },               // 0x60
    { "bv",   BV,   { V32, } },               // 0x61
    { "bnv",  BNV,  { REG, } },               // 0x62
    { "bnv",  BNV,  { V32, } },               // 0x63

    { "jmp",  JMP,  { REG, } },               // 0x64
    { "jmp",  JMP,  { V32, } },               // 0x65
    { "jsr",  JSR,  { REG, } },               // 0x66
    { "jsr",  JSR,  { V32, } },               // 0x67
    { "ret",  RET,  {} },                     // 0x68

    // stack
    { "pushb",  PUSHB,  { REG, } },             // 0x69
    { "pushb",  PUSHB,  { V8, } },              // 0x6A
    { "pushw",  PUSHW,  { REG, } },             // 0x6B
    { "pushw",  PUSHW,  { V16, } },             // 0x6C
    { "pushd",  PUSHD,  { REG, } },             // 0x6D
    { "pushd",  PUSHD,  { V32, } },             // 0x6E
    { "push.a", PUSH_A, {} },                   // 0x6F
    { "popb",   POPB,   { REG, } },             // 0x60
    { "popw",   POPW,   { REG, } },             // 0x61
    { "popd",   POPD,   { REG, } },             // 0x62
    { "pop.a",  POP_A,  {} },                   // 0x63
    { "popx",   POPX,   { REG, } },             // 0x64
    { "popx",   POPX,   { V8, } },              // 0x65
    { "popx",   POPX,   { V16, } },             // 0x66

    // other
    { "nop", NOP,  {} },                        // 0x67
}};

}

#endif
