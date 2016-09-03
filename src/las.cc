#include "luisavm.hh"

#include <cstdint>
#include <cstdlib>
#include <getopt.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

// {{{ COMMANDLINE OPTIONS

struct Options {
    string   input_file = "";
    string   output_file = "";
    string   map_file = "";

    Options(int argc, char* argv[])
    {
        int c;

        while(true) {
            int option_index = 0;
            static struct option long_options[] = {
                {"output",  required_argument, nullptr,  'o' },
                {"map",     required_argument, nullptr,  'm' },
                {"help",    no_argument,       nullptr,  'h' },
                {nullptr,   0,                 nullptr,   0  }
            };

            c = getopt_long(argc, argv, "o:m:h", long_options, &option_index);
            if(c == -1) {
                break;
            }

            switch(c) {
                case 'm':
                    map_file = optarg;
                    break;
                case 'o':
                    map_file = optarg;
                    break;
                case 'h':
                    cout << "LuisaVM assembler\n";
                    cout << "Options:\n";
                    cout << "   -m, --map         map file\n";
                    cout << "   -o, --output      output file\n";
                    cout << "   -h, --help        this help\n";
                    exit(EXIT_SUCCESS);
                case '?':
                    exit(EXIT_FAILURE);
                default:
                    abort();
            }
        }

        if(optind < argc) {
            input_file = argv[optind];
        }
    }
};

// }}}

int main(int argc, char* argv[])
{
    // parse options
    Options opt(argc, argv);

    if(opt.input_file == "") {
        cerr << "Please define an input file.\n";
        return EXIT_FAILURE;
    }

    if(opt.output_file == "") {
        size_t lastindex = opt.input_file.find_last_of("."); 
        opt.output_file = opt.input_file.substr(0, lastindex) + ".bin";
    }

    // read source
    ifstream t(opt.input_file);
    if(!t.is_open()) {
        cerr << "Could not open file " + opt.input_file + "\n";
        return EXIT_FAILURE;
    }
    stringstream buf;
    buf << t.rdbuf();

    // assemble
    string mp;
    vector<uint8_t> data;
    try {  // NOLINT
        data = luisavm::Assembler().AssembleString(opt.input_file, buf.str(), mp);
    } catch(runtime_error& e) {
        cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    // store binary
    ofstream output(opt.output_file, ios::out | ios::binary);
    if(!output.is_open()) {
        cerr << "Could not open file " + opt.output_file + "\n";
        return EXIT_FAILURE;
    }
    output.write(reinterpret_cast<const char*>(&data[0]), data.size());
    output.close();

    // store map
    if(opt.map_file != "") {
        ofstream output(opt.map_file);
        if(!output.is_open()) {
            cerr << "Could not open file " + opt.map_file + "\n";
            return EXIT_FAILURE;
        }
        output.write(mp.c_str(), mp.size());
        output.close();
    }

}
