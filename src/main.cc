#include "luisavm.hh"

#include <cstdint>
#include <cstdlib>
#include <getopt.h>

#include <iostream>
#include <string>
using namespace std;

extern void run_tests();

// {{{ COMMANDLINE OPTIONS

struct Options {
    string   rom_file;
    string   map_file;
    uint32_t memory_size = 16;
    uint8_t  zoom = 2;
    bool     start_with_debugger = true;

    Options(int argc, char* argv[])
    {
        int c;

        while(true) {
            int option_index = 0;
            static struct option long_options[] = {
                {"memory",  required_argument, nullptr,  'M' },
                {"map",     required_argument, nullptr,  'm' },
                {"zoom",    required_argument, nullptr,  'z' },
                {"help",    no_argument,       nullptr,  'h' },
                {nullptr,   0,                 nullptr,   0  }
            };

            c = getopt_long(argc, argv, "m:M:z:h", long_options, &option_index);
            if(c == -1) {
                break;
            }

            switch(c) {
                case 'm':
                    memory_size = strtol(optarg, nullptr, 10);
                    break;
                case 'M':
                    map_file = optarg;
                    break;
                case 'z':
                    zoom = strtol(optarg, nullptr, 10);
                    break;
                case 'h':
                    cout << "LuisaVM emulator version " VERSION "\n";
                    cout << "Options:\n";
                    cout << "   -m, --memory      memory size, in kB\n";
                    cout << "   -z, --zoom        zoom of the display\n";
                    cout << "   -T, --test        run unit tests\n";
                    cout << "   -h, --help        this help\n";
                    exit(EXIT_SUCCESS);
                case '?':
                    exit(EXIT_FAILURE);
                default:
                    abort();
            }
        }

        if(optind < argc) {
            rom_file = argv[optind];
        }
    }
};

// }}}

int main(int argc, char* argv[])
{
    Options opt(argc, argv);
}
