#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>


static void
help()
{
}


static void
version()
{
    printf("lillipad version " VERSION "\n");
    printf("Created by Andre Wagner. Distributed under the GPLv3.\n");
}


static void
parse_options(Config* config, int argc, char** argv)
{
    int c;

    while(1) {
        int option_index = 0;
        static struct option long_options[] = {
            { "version", no_argument, 0, 0 },
            { "help",    no_argument, 0, 0 },
            { 0, 0, 0, 0 },
        };

        c = getopt_long(argc, argv, "vh", long_options, &option_index);
        if(c == -1)
            break;

        switch(c) {
            case 0:
                printf("option %s", long_options[option_index].name);
                if (optarg)
                    printf(" with arg %s", optarg);
                printf("\n");
                break;

            case 'v':
                version();
                break;

            case 'h':
                help();
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
    config->total_ram = 2 * 1024 * 1024;

    parse_options(config, argc, argv);

    return config;
}


void
config_free(Config* config)
{
    free(config);
}
