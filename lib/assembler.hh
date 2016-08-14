#ifndef ASSEMBLER_HH_
#define ASSEMBLER_HH_

#include <string>
#include <vector>
using namespace std;

namespace luisavm {

class Assembler {
public:
    vector<uint8_t> AssembleString(string const& filename, string const& code);

private:
    string Preprocess(string const& filename, string const& code);
};

}  // namespace luisavm

#endif
