#include "tests.h"

#ifdef DEBUG

#include <assert.h>
#include <stdlib.h>
#include <syslog.h>
#include <pcre.h>

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

//--------------------------------------------------------------

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

//--------------------------------------------------------------

// {{{ opcode execution

pcre* re;

static void
cpu_add(const char* code)
{
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

    static const char* registers[] = { "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "fp", "sp", "pc", "fl" };

    int substr[30];
    int n = pcre_exec(re, NULL, code, strlen(code), 0, 0, substr, 30);
    if(n < 0) {
        syslog(LOG_ERR, "Error matching regex.");
        exit(EXIT_FAILURE);
    }

    for(int i=0; i<n; ++i) {
        const char* match;
        pcre_get_substring(code, substr, n, i, &match);
        syslog(LOG_NOTICE, "match: '%s'", match);
        pcre_free_substring(match);
    }

}

#define EXECP(precode, assembly, tst, expected) \
    computer_reset();                           \
    { precode; }                                \
    cpu_add(assembly);                          \
    cpu_step();                                 \
    test(tst, expected, assembly);

#define EXEC(assembly, tst, expected)           \
    computer_reset();                           \
    cpu_add(assembly);                          \
    cpu_step();                                 \
    test(tst, expected, assembly);

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
}


static void
test_cpu_MOV()
{
    EXECP(cpu_setregister(B, 0x42), "mov a, b", cpu_register(A), 0x42);
    //test(cpu_register(PC), 2, "PC = 2");
    /*
        conn.exec('mov a, 0x34')
        self.assertE('c r a', 0x34)

        conn.exec('mov a, 0x1234')
        self.assertE('m r 0', 3)
        self.assertE('c r a', 0x1234)

        conn.exec('mov a, 0xFABC1234')
        self.assertE('c r a', 0xFABC1234)

        conn.exec('mov a, 0')
        self.assertE('c f z', 1)
        self.assertE('c f p', 1)
        self.assertE('c f s', 0)

        conn.exec('mov a, 0xF0000001')
        self.assertE('c f z', 0)
        self.assertE('c f p', 0)
        self.assertE('c f s', 1)
    */
}


static void
test_cpu()
{
    syslog(LOG_NOTICE, "[[[ CPU ]]]");

    const char* pcre_str;
    int pcre_err_offset;
    re = pcre_compile("([a-z\\.]+?)\\s+([\\w|\\[|\\]]+?)?(?:,\\s*([\\w|\\[|\\]]+?)?)", 
            PCRE_CASELESS, &pcre_str, &pcre_err_offset, NULL);
    if(!re) {
        syslog(LOG_ERR, "Could not compile re: %s", pcre_str);
        exit(EXIT_FAILURE);
    }

    test_cpu_basic();
    test_cpu_MOV();

    pcre_free(&re);
}

#endif
