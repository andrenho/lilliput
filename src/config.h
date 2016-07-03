#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>

typedef enum {
    XCB,
} VideoOutput;

typedef struct Config {
    uint32_t    memory_kb;
    char**      rom_files;
    int         zoom;
    VideoOutput video_output;
} Config;

Config* config_init(int argc, char** argv);
void    config_free(Config* config);

void    config_log(Config* config);

#endif
