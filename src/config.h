#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>

typedef struct {
    uint32_t memory_kb;
    char**   rom_files;
} Config;

Config* config_init(int argc, char** argv);
void    config_free(Config* config);

void    config_log(Config* config);

#endif
