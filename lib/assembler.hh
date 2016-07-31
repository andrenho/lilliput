#ifndef ASSEMBLER_HH_
#define ASSEMBLER_HH_

#include <map>
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
    enum Section { NONE, TEXT, DATA, BSS };

    struct Pos {
        string filename;
        size_t n_line;
    };

    struct Parameter {
        ParameterType type;
        uint32_t      value;
    };

    struct Label {
        Section  section;
        uint32_t pos;
    };

    // preprocessing
    string Preprocess(string const& filename, string const& code) const;
    void   RemoveComments(string& line) const;
    Pos    ExtractPos(string& line) const;
    void   Define(string const& def, string const& val);
    void   Section(string const& section);
    void   ReplaceConstants(string& line) const;

    // labels
    void   ExtractLabel(string& line);
    void   ReplaceLabels();

    // data
    void   Data(string const& sz, string const& data);
    void   Bss(string const& sz, size_t n);
    void   Ascii(bool zero, string const& data);

    // instruction parsing
    void   Instruction(string const& inst, string const& pars);
    Parameter ParseParameter(string const& par);

    // linkage
    vector<uint8_t> CreateBinary() const;

    enum Section          _current_section = Section::NONE;
    vector<uint8_t>       _text, _data;
    uint32_t              _bss_sz = 0;
    map<string, Label>    _labels;
    string                _current_label;
    map<uint32_t, string> _pending_labels;
    map<string, string>   _defines;
};

}  // namespace luisavm

#endif
