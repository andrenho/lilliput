#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#pragma GCC diagnostic ignored "-Wformat"

#define TEST(test, expect)                      \
{                                               \
    ssize_t v = (test);                         \
    if(v == expect) {                           \
        syslog(LOG_NOTICE, "  ok -> " #test " == 0x%" PRIX64, v);   \
    } else {                                    \
        syslog(LOG_ERR, "  error! -> " #test " (expected 0x%" PRIX64 ", found 0x%" PRIX64 ")", expect, v);  \
        abort();                                \
    }                                           \
}
