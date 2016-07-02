#include "config.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>


static void
help(char* program_name, bool ok)
{
    FILE* out;
    int end;
    if(ok) {
        out = stdout;
        end = EXIT_SUCCESS;
    } else {
        out = stderr;
        end = EXIT_FAILURE;
    }

    fprintf(out, "Usage: %s [OPTIONS] [ROM_FILE]\n", program_name);
    fprintf(out, "Options:\n");
    fprintf(out, "   -m, --memory-kb        Total memory, in KB (default 1024)\n");
    fprintf(out, "   -v, --version          Print version and exit\n");
    fprintf(out, "   -h, --help             Print this help and exit\n");

    exit(end);
}


static void
version()
{
    printf("lillipad version " VERSION "\n");
    printf("Created by Andre' Wagner. Distributed under the GPLv3.\n");
    exit(EXIT_SUCCESS);
}


static void
parse_options(Config* config, int argc, char** argv)
{
    int c;

    while(1) {
        int option_index = 0;
        static struct option long_options[] = {
            { "memory",  required_argument, 0, 'm' },
            { "version", no_argument,       0, 'v' },
            { "help",    no_argument,       0, 'h' },
            { 0, 0, 0, 0 },
        };

        c = getopt_long(argc, argv, "vhm:", long_options, &option_index);
        if(c == -1)
            break;

        switch(c) {
            case 0:
                printf("option %s", long_options[option_index].name);
                if (optarg)
                    printf(" with arg %s", optarg);
                printf("\n");
                break;

            case 'm': {
                    char* endptr = NULL;
                    long int m = strtol(optarg, &endptr, 10);
                    // TODO - check overflow, underflow (see strtol manpage)
                    if(endptr == optarg || *endptr != 0) {
                        fprintf(stderr, "Please enter a valid memory value.\n");
                        exit(EXIT_FAILURE);
                    }
                    config->memory_kb = (uint32_t)m;
                }
                break;

            case 'v':
                version();
                break;

            case 'h':
                help(argv[0], true);
                break;

            case '?':
                break;

            default:
                printf("?? getopt returned character code 0x%o ??\n", c);
        }
    }

    if(optind < argc) {
        
    }
}


Config* 
config_init(int argc, char** argv)
{
    Config* config = calloc(sizeof(Config), 1);
    config->memory_kb = 1024;

    parse_options(config, argc, argv);

    return config;
}


void
config_free(Config* config)
{
    free(config);
}
