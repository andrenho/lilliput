#include "memory.h"

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <syslog.h>

typedef struct {
    uint32_t pos, 
             sz;
    get_t    get;
    set_t    set;
} MemoryArea;

static uint8_t*    ram = NULL;
static size_t      ram_sz = 0;
static uint32_t    offset = 0x0;
static MemoryArea* areas = NULL;
static size_t      n_areas = 0;

static uint8_t my_get(uint32_t pos);
static void    my_set(uint32_t pos, uint8_t data);

void
memory_init(Config* config)
{
    ram_sz = config->memory_kb * 1024;
    if(ram_sz >= SYSTEM_AREA) {
        syslog(LOG_ERR, "The memory size is too big (maximum %d).", SYSTEM_AREA / 1024);
        exit(EXIT_FAILURE);
    }
    ram = calloc(1, ram_sz);

    memory_addmap(SYSTEM_AREA, sizeof(offset), my_get, my_set);
}


void
memory_destroy()
{
    if(areas) {
        free(areas);
    }
    free(ram);
}


uint32_t
memory_addmap(uint32_t pos, uint32_t sz, get_t get, set_t set)
{
    assert((uint64_t)pos + (uint64_t)sz < 0x100000000);
    areas = realloc(areas, (n_areas+1) * sizeof(MemoryArea));
    areas[n_areas] = (MemoryArea) { pos, sz, get, set };
    ++n_areas;
    return pos + sz;
}


uint8_t
memory_get(uint32_t pos)
{
    if(pos < SYSTEM_AREA) {
        uint64_t off = pos + offset;
        if(off > ram_sz) {
            syslog(LOG_ERR, "Invalid memory access (trying to read position 0x%" PRIX32 " [offset 0x%" PRIx64 "]).", pos, off);
            exit(EXIT_FAILURE);
        }
        return ram[off];
    } else {
        for(size_t i=0; i<n_areas; ++i) {
            if(pos >= areas[i].pos && pos < (areas[i].pos + areas[i].sz)) {
                get_t get = areas[i].get;
                if(get) {
                    return get(pos - areas[i].pos);
                }
            }
        }
        syslog(LOG_ERR, "Invalid memory access (trying to read position 0x%" PRIX32 ").", pos);
        exit(EXIT_FAILURE);
    }
}


void
memory_set(uint32_t pos, uint8_t data)
{
    if(pos < SYSTEM_AREA) {
        uint64_t off = pos + offset;
        if(off > ram_sz) {
            syslog(LOG_ERR, "Invalid memory access (trying to write position 0x%" PRIX32 " [offset 0x%" PRIx64 "]).", pos, off);
            exit(EXIT_FAILURE);
        }
        ram[off] = data;
    } else {
        for(size_t i=0; i<n_areas; ++i) {
            if(pos >= areas[i].pos && pos < (areas[i].pos + areas[i].sz)) {
                set_t set = areas[i].set;
                if(set) {
                    set(pos - areas[i].pos, data);
                    return;
                }
            }
        }
        syslog(LOG_ERR, "Invalid memory access (trying to write position 0x%" PRIX32 ").", pos);
        exit(EXIT_FAILURE);
    }
}


uint16_t 
memory_get16(uint32_t pos)
{
    uint16_t b1 = memory_get(pos),
             b2 = memory_get(pos+1);
    return b1 | (b2 >> 8);
}


uint32_t 
memory_get32(uint32_t pos)
{
    uint32_t b1 = memory_get(pos),
             b2 = memory_get(pos+1),
             b3 = memory_get(pos+2),
             b4 = memory_get(pos+3);
    return b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
}


void     
memory_set16(uint32_t pos, uint16_t data)
{
    memory_set(pos, (uint8_t)data);
    memory_set(pos+1, (uint8_t)(data >> 8));
}


void     
memory_set32(uint32_t pos, uint32_t data)
{
    memory_set(pos, (uint8_t)data);
    memory_set(pos+1, (uint8_t)(data >> 8));
    memory_set(pos+2, (uint8_t)(data >> 16));
    memory_set(pos+3, (uint8_t)(data >> 24));
}


static uint8_t 
my_get(uint32_t pos)
{
    assert(pos < 4);
    return ((offset >> (pos * 8)) & 0xFF);
}


static void
my_set(uint32_t pos, uint8_t data)
{
    uint32_t n = pos * 8;
    offset -= (offset & (uint32_t)(0xFF >> n));
    offset += ((uint32_t)data << n);
}


// {{{ TESTS

#include "test.h"

void
memory_test()
{
    syslog(LOG_NOTICE, "[MEMORY]");

    memory_set(0x12, 0xAF);
    TEST(memory_get(0x12), 0xAF);

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
}

// }}}
