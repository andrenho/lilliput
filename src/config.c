#include "config.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include <syslog.h>

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
    fprintf(out, "   -m, --memory-kb        Total memory, in KB (default: 1024)\n");
    fprintf(out, "   -z, --zoom             Video zoom (default: 3)\n");
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


static long int 
to_num(char* s, const char* what)
{
    char* endptr = NULL;
    long int m = strtol(s, &endptr, 10);
    // TODO - check overflow, underflow (see strtol manpage)
    if(endptr == s || *endptr != 0) {
        fprintf(stderr, "Please enter a valid %s.\n", what);
        exit(EXIT_FAILURE);
    }
    return (long int)m;
}


static void
parse_options(Config* config, int argc, char** argv)
{
    int c;

    while(1) {
        int option_index = 0;
        static struct option long_options[] = {
            { "memory",  required_argument, 0, 'm' },
            { "zoom",    required_argument, 0, 'z' },
            { "version", no_argument,       0, 'v' },
            { "help",    no_argument,       0, 'h' },
            { 0, 0, 0, 0 },
        };

        c = getopt_long(argc, argv, "vhm:z:T", long_options, &option_index);
        if(c == -1)
            break;

        switch(c) {
            case 0:
                printf("option %s", long_options[option_index].name);
                if (optarg)
                    printf(" with arg %s", optarg);
                printf("\n");
                break;

            case 'm': 
                config->memory_kb = (uint32_t)to_num(optarg, "memory value");
                break;

            case 'z':
                config->zoom = (int)to_num(optarg, "zoom level");
                break;

            case 'T':  // hidden option
                config->test_only = true;
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

    // load ROM files
    if(optind < (argc - 1)) {
        help(argv[0], false);
    } else if(optind == (argc - 1)) {
        config->rom_file = argv[optind];
    }
}


Config* 
config_init(int argc, char** argv)
{
    Config* config = calloc(sizeof(Config), 1);
    config->memory_kb = 1024;
    config->rom_file = NULL;
    config->zoom = 2;
    config->test_only = false;

    parse_options(config, argc, argv);

    return config;
}


void
config_free(Config* config)
{
    free(config);
}


void    
config_log(Config* config)
{
    syslog(LOG_DEBUG, "Options chosen:");
    syslog(LOG_DEBUG, "  Memory KB: %d", config->memory_kb);
    if(config->rom_file) {
        syslog(LOG_DEBUG, "  ROM: %s", config->rom_file);
    } else {
        syslog(LOG_DEBUG, "  ROM: -");
    }
}
