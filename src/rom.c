#include "rom.h"

#include <assert.h>
#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>

uint8_t* rom = NULL;
uint32_t rom_sz = 0;

void 
rom_init(const char* filename)
{
    FILE* f = fopen(filename, "r");
    if(!f) {
        perror("fopen");
        syslog(LOG_ERR, "Could not open ROM file '%s'.", filename);
        exit(EXIT_FAILURE);
    }

    // get file size
    fseek(f, 0, SEEK_END);
    rom_sz = (uint32_t)ftell(f);
    rom = malloc(rom_sz);
    rewind(f);

    // load file
    int c;
    size_t n = 0;
    while((c = fgetc(f)) != EOF) {
        assert(n < rom_sz);
        rom[n++] = (uint8_t)c;
    }

    syslog(LOG_NOTICE, "ROM file '%s' loaded (%d bytes).", filename, rom_sz);
}


void 
rom_destroy()
{
    free(rom);
}


uint32_t 
rom_size()
{
    return rom_sz;
}


uint8_t 
rom_get(uint32_t pos)
{
    return rom[pos];
}
