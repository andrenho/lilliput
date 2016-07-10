#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    XCB,
} VideoOutput;

typedef struct Config {
    uint32_t    memory_kb;
    char*       rom_file;
    int         zoom;
    VideoOutput video_output;
    bool        run_tests;
    bool        quiet;
} Config;

Config* config_init(int argc, char** argv);
void    config_free(Config* config);

void    config_log(Config* config);

#endif
