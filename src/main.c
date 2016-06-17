#include <stdio.h>

#include "config.h"

int 
main(int argc, char** argv)
{
    Config* config = config_init(argc, argv);

    config_free(config);
    return 0;
}
