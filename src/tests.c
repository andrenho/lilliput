#include "tests.h"

#ifdef DEBUG

#include <stdlib.h>
#include <syslog.h>

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
        syslog(LOG_NOTICE, "  [\x1b[31merror\x1b[0m] %s", desc);
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
// }}}

static void
test_cpu()
{
    syslog(LOG_NOTICE, "[[[ CPU ]]]");
}

#endif
