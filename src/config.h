#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>

typedef struct {
    uint64_t total_ram;
    char*    rom_file;
} Config;

Config* config_init(int argc, char** argv);
void    config_free(Config* config);

#endif
