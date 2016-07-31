#ifndef ASSEMBLER_HH_
#define ASSEMBLER_HH_

#include <string>
#include <vector>
using namespace std;

#include "opcodes.hh"

namespace luisavm {

class Assembler {
public:
    vector<uint8_t> AssembleString(string const& filename, string const& code, string& mp);
    vector<uint8_t> AssembleString(string const& filename, string const& code) {
        string mp;
        return AssembleString(filename, code, mp);
    }

private:
    struct Pos {
        string filename;
        size_t n_line;
    };

    struct Parameter {
        ParameterType type;
        uint32_t      value;
    };

    // preprocessing
    string Preprocess(string const& filename, string const& code) const;
    void   RemoveComments(string& line) const;
    Pos    ExtractPos(string& line) const;
    void   Define(string const& def, string const& val);
    void   Section(string const& section);
    void   ReplaceConstants(string& line) const;

    // labels
    void   ExtractLabel(string& line) const;
    void   ReplaceLabels();

    // data
    void   Data(string const& sz, string const& data);
    void   BSS(string const& sz, size_t n);
    void   Ascii(bool zero, string const& data);

    // instruction parsing
    void   Instruction(string const& inst, string const& pars);
    Parameter ParseParameter(string const& par) const;

    // linkage
    vector<uint8_t> CreateBinary() const;

    string          _current_section;
    vector<uint8_t> _code, _data;
};

}  // namespace luisavm

#endif
