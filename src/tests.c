#include "tests.h"

#ifdef DEBUG

#include <syslog.h>

#include "memory.h"

static void test_memory();

void 
tests_run()
{
    test_memory();
}


static void
test(uint32_t t, uint32_t expected, const char* desc)
{
    if(t == expected) {
        syslog(LOG_NOTICE, "  [\x1b[32mok\x1b[0m] %s", desc);
    } else {
        syslog(LOG_NOTICE, "  [\x1b[31merror\x1b[0m] %s", desc);
    }
}


static void
test_memory()
{
    syslog(LOG_NOTICE, "[[[ MEMORY ]]]");

    memory_set(0x12, 0xAF);
    test(memory_get(0x12), 0xAF, "memory_set");
/*
    syslog(LOG_NOTICE, "[MEMORY]");


    memory_set32(0x0, 0x12345678);
    TEST(memory_get(0x0), 0x78);
    TEST(memory_get(0x1), 0x56);
    TEST(memory_get(0x2), 0x34);
    TEST(memory_get(0x3), 0x12);

    memory_set32(SYSTEM_AREA, 0x1000);
    TEST(offset, 0x1000);
    TEST(memory_get32(SYSTEM_AREA), 0x1000);

    memory_set(0x12, 0xFF);
    TEST(ram[0x12], 0xAF);
    TEST(ram[0x1012], 0xFF);
    TEST(memory_get(0x12), 0xFF);
*/
}

#endif
