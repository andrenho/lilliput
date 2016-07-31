#include "assembler.hh"

#include <cctype>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
using namespace std;

// TODO - create a new exception

namespace luisavm {

// {{{ preprocessing

string Assembler::Preprocess(string const& filename, string const& code) const
{
    static regex import(R"(^%import\s+(.*))", regex_constants::icase);
    smatch match;

    string line;
    stringstream ss(code);

    string source;
    size_t nline = 1;
    while(getline(ss, line, '\n')) {
        if(regex_search(line, match, import)) {
            // a import was found, read and preprocess file
            ifstream t(match.str(1));
            if(!t.is_open()) {
                throw runtime_error("Could not open file " + match.str(1));
            }
            stringstream buf;
            buf << t.rdbuf();
            source += Preprocess(match.str(1), buf.str());
        } else {
            // regular line, just add the file/line indicator
            source += "<" + filename + ":" + to_string(nline++) + ">" + line + "\n";
        }
    }
    return source;
}


void Assembler::RemoveComments(string& line) const
{
    static regex comment(R"(^(.*?)\s*[^\\];.*$)");
    smatch match;

    if(!line.empty() && line[0] == ';') {
        line = "";
    } else if(regex_search(line, match, comment)) {
        line = match.str(1);
    }
}


Assembler::Pos Assembler::ExtractPos(string& line) const
{
    static regex fl(R"(^<([^:]+):(\d+)>\s*(.*?)\s*$)");
    smatch match;

    if(regex_search(line, match, fl)) {
        Pos pos = { match.str(1), static_cast<size_t>(stoll(match.str(2))) };
        line = match.str(3);
        return pos;
    } else {
        throw logic_error("Invalid preprocessed line");
    }
}


void Assembler::Define(string const& def, string const& val)
{
    if(_defines.find(def) != _defines.end()) {
        throw runtime_error("Constant " + def + " already defined.");
    }
    _defines[def] = val;
}


void Assembler::Section(string const& section)
{
    string sec;
    transform(section.begin(), section.end(), back_inserter(sec), ::tolower);
    if(sec == ".text") {
        _current_section = TEXT;
    } else if(sec == ".data") {
        _current_section = DATA;
    } else if(sec == ".bss") {
        _current_section = BSS;
    } else {
        throw runtime_error("Invalid section |" + sec + "|");
    }
}

void Assembler::ReplaceConstants(string& line) const
{
    for(auto const& kv: _defines) {
        line = regex_replace(line, regex(kv.first), kv.second);
    }
}

// }}}

// {{{ labels

void Assembler::ExtractLabel(string& line)
{
    static regex label(R"(^\s*(\.?[a-z_]\w*)\s*:(.*)$)", regex_constants::icase),
                 lbl_remove(R"(^\s*(\.?[a-z_]\w*)\s*:\s*)", regex_constants::icase);
    smatch match;

    if(regex_search(line, match, label)) {
        // find label name
        string lbl;
        if(match.str(1)[0] == '.') {
            lbl = _current_label + "@" + match.str(1);
        } else {
            lbl = _current_label = match.str(1);
        }
        line = regex_replace(line, lbl_remove, "");

        // was this label already used?
        if(_labels.find(lbl) != _labels.end()) {
            throw runtime_error("label '" + lbl + "' reused.");
        }

        // find position
        uint32_t pos = 0;
        switch(_current_section) {
            case TEXT: pos = static_cast<uint32_t>(_text.size()); break;
            case DATA: pos = static_cast<uint32_t>(_data.size()); break;
            case BSS:  pos = _bss_sz; break;
            default: throw runtime_error("no section defined");
        }
        _labels[lbl] = { _current_section, pos };
    }
}


void Assembler::ReplaceLabels()
{
}

// }}}

// {{{ data

void Assembler::Data(string const& sz, string const& data)
{
    static regex dt(R"(([^\s,]+))");

    auto end = sregex_token_iterator();
    for(sregex_token_iterator i(data.begin(), data.end(), dt, 1); i != end; ++i) {
        try {
            uint32_t value = static_cast<uint32_t>(stoll(*i, nullptr, 0));
            if(sz == "b") {
                if(value > 0xFF) {
                    throw out_of_range("");
                }
                _data.push_back(static_cast<uint8_t>(value));
            } else if(sz == "w") {
                if(value > 0xFFFF) {
                    throw out_of_range("");
                }
                _data.push_back(static_cast<uint8_t>(value));
                _data.push_back(static_cast<uint8_t>(value >> 8));
            } else if(sz == "d") {
                _data.push_back(static_cast<uint8_t>(value));
                _data.push_back(static_cast<uint8_t>(value >> 8));
                _data.push_back(static_cast<uint8_t>(value >> 16));
                _data.push_back(static_cast<uint8_t>(value >> 24));
            } else {
                throw logic_error("invalid data type");
            }
        } catch(invalid_argument&) {
            throw runtime_error("Invalid data argument " + string(*i) + ".");
        } catch(out_of_range&) {
            throw runtime_error("Value " + string(*i) + " out of range.");
        }
    }
}


void Assembler::Bss(string const& sz, size_t n)
{
    _bss_sz += n;
}


void Assembler::Ascii(bool zero, string const& data)
{
    for(auto const& c: data) {
        _data.push_back(c);
    }
    if(zero) {
        _data.push_back(0);
    }
}

// }}}

// {{{ instruction parsing

void Assembler::Instruction(string const& inst, string const& pars)
{
    static regex rpar(R"(([^,]+),?\s*)");
    
    // find parameters
    vector<string> spar;
    copy(sregex_token_iterator(begin(pars), end(pars), rpar, 1), sregex_token_iterator(),
            back_inserter(spar));

    // find parameter type and value
    vector<Parameter> par;
    transform(begin(spar), end(spar), back_inserter(par),
            [this](string const& p){ return ParseParameter(p); });

    vector<ParameterType> partype;
    transform(begin(par), end(par), back_inserter(partype),
            [this](Parameter const& p){ return p.type; });

    // find instruction
    uint8_t i=0;
    for(auto const& op: opcodes) {   // opcodes in 'opcodes.hh'
        if(inst == op.instruction && partype == op.parameter) {
            _text.push_back(i);
            goto found;
        }
        ++i;
    }
    throw runtime_error("Invalid instruction '" + inst + " " + pars + "'.");

found:
    if(par.size() == 2 && (par[0].type == REG || par[0].type == INDREG) && (par[1].type == REG || par[1].type == INDREG)) {
        _text.push_back((par[0].value << 4) | (par[1].value & 0xF));
    } else {
        for(auto const& p: par) {
            switch(p.type) {
                case REG: case INDREG: case V8:
                    _text.push_back(static_cast<uint8_t>(p.value));
                    break;
                case V16:
                    _text.push_back(static_cast<uint8_t>(p.value));
                    _text.push_back(static_cast<uint8_t>(p.value >> 8));
                    break;
                case V32: case INDV32:
                    _text.push_back(static_cast<uint8_t>(p.value));
                    _text.push_back(static_cast<uint8_t>(p.value >> 8));
                    _text.push_back(static_cast<uint8_t>(p.value >> 16));
                    _text.push_back(static_cast<uint8_t>(p.value >> 24));
                    break;
                default: throw logic_error("invalid parameter type");
            }
        }
    }
}


Assembler::Parameter Assembler::ParseParameter(string const& par)
{
    string lpar;
    transform(par.begin(), par.end(), back_inserter(lpar), ::tolower);
    
    smatch match;
    static regex indreg(R"(\[([a-z][a-z]?)\])"),
                 indv32(R"(\[([xb\d]+)\])");

    static map<string, uint8_t> reg = {
        {"a",0}, {"b",1}, {"c",2}, {"d",3}, {"e",4}, {"f",5}, {"g",6}, {"h",7}, 
        {"i",8}, {"j",9}, {"k",10}, {"l",11}, {"fp",12}, {"sp",13}, {"pc",14}, {"fl",15}
    };

    // register?
    if(reg.find(lpar) != reg.end()) {
        return { REG, reg.at(lpar) };
    // indirect register?
    } else if(regex_match(lpar, match, indreg)) {
        try {
            return { INDREG, reg.at(match.str(1)) };
        } catch(out_of_range&) {
            throw runtime_error("Invalid register " + match.str(1));
        }
    }

    // indirect value?
    if(regex_match(lpar, match, indv32)) {
        try {
            uint32_t value = static_cast<uint32_t>(stoll(match.str(1), nullptr, 0));
            return { INDV32, value };
        } catch(invalid_argument&) {
            throw runtime_error("Invalid indirect value " + par);
        }
    }

    // value?
    try {
        uint32_t value = static_cast<uint32_t>(stoll(lpar, nullptr, 0));
        if(value <= 0xFF) {
            return { V8, value };
        } else if(value <= 0xFFFF) {
            return { V16, value };
        } else if(value <= 0xFFFFFFFF) {
            return { V32, value };
        }
    // label?
    } catch(invalid_argument&) {
        string lbl_name = (par[0] == '.') ? _current_label + par : par;
        _pending_labels[_text.size()] = lbl_name;
        return { V32, 0 };
    }

    throw runtime_error("Invalid parameter " + par);
}


// }}}

// {{{ linkage

vector<uint8_t> Assembler::CreateBinary() const
{
    vector<uint8_t> bin;
    copy(begin(_text), end(_text), back_inserter(bin));
    copy(begin(_data), end(_data), back_inserter(bin));
    return bin;
}

// }}}

// {{{ main assembler

vector<uint8_t> Assembler::AssembleString(string const& filename, string const& code, string& mp)
{
    smatch match;
    static regex define(R"(^%define\s+([A-Za-z_][\w_]*)\s+([^\s]+)$)", regex_constants::icase),
                 section(R"(^section\s+(\.[a-z]+)$)", regex_constants::icase),
                 data(R"(^\.d([bwd])\s+(.+)$)", regex_constants::icase),
                 bss(R"(^\.res([bwd])\s+([xb\d]+)$)", regex_constants::icase),
                 ascii(R"(^\.ascii(z?)\s+(.+)$)", regex_constants::icase),
                 inst(R"(^([\w\.]+)\s+(.+)$)", regex_constants::icase);

    // parse code
    string line;
    stringstream ss(Preprocess(filename, code));
    while(getline(ss, line, '\n')) {
        RemoveComments(line);
        Pos pos = ExtractPos(line);
        string original = line;
        if(!line.empty()) {
            if(regex_match(line, match, define)) {
                Define(match.str(1), match.str(2));
            } else if(regex_match(line, match, section)) {
                Section(match.str(1));
            } else {
                ExtractLabel(line);
                ReplaceConstants(line);
                if(_current_section == DATA && regex_match(line, match, data)) {
                    Data(match.str(1), match.str(2));
                } else if(_current_section == BSS && regex_match(line, match, bss)) {
                    Bss(match.str(1), stoll(match.str(2), nullptr, 0));  // TODO - catch errors
                } else if(_current_section == DATA && regex_match(line, match, ascii)) {
                    Ascii(match.str(1) == "z", match.str(2));
                } else if(_current_section == TEXT && regex_match(line, match, inst)) {
                    Instruction(match.str(1), match.str(2));
                } else {
                    throw runtime_error("Syntax error in " + pos.filename + ":" + 
                            to_string(pos.n_line) + ":  |" + original + "|");
                }
            }
        }
    }

    // create binary
    ReplaceLabels();
    return CreateBinary();
}  

// }}}

}  // namespace luisavm
