#include "assembler.hh"

#include <iostream>
#include <regex>
#include <sstream>
#include <string>
using namespace std;

namespace luisavm {

string
Assembler::Preprocess(string const& filename, string const& code)
{
    static regex include(R"(^%include\s+(.*))");
    smatch match;

    string line;
    stringstream ss(code);

    string source;
    int nline = 1;
    while(getline(ss, line, '\n')) {
        if(regex_search(line, match, include)) {
            source += "match\n";
        } else {
            source += "<" + filename + ":" + to_string(nline++) + ">" + line + "\n";
        }
    }
    return source;
}


vector<uint8_t> Assembler::AssembleString(string const& filename, string const& code)
{
    string source = Preprocess(filename, code);
    cout << source << endl;
    return {};
}

}  // namespace luisavm
