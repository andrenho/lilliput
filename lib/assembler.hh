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
    struct Pos {
        string filename;
        size_t n_line;
    };

    string Preprocess(string const& filename, string const& code) const;
    void   RemoveComments(string& line) const;
    Pos    ExtractPos(string& line) const;
    void   Define(string const& def, string const& val);
    void   Section(string const& section);
    void   ExtractLabel(string& line) const;
    void   ReplaceConstants(string& line) const;
    void   Data(string const& sz, string const& data);
    void   BSS(string const& sz, size_t n);
    void   Ascii(bool zero, string const& data);
    void   Instruction(string const& inst, string const& pars);

    string _current_section;
};

}  // namespace luisavm

#endif
