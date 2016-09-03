#ifndef CPU_HH_
#define CPU_HH_

#include <array>
#include <cstdint>
using namespace std;

#include "device.hh"
#include "opcodes.hh"

namespace luisavm {

enum Flag { Y, V, Z, S, GT, LT };

class CPU : public Device {
public:
    explicit CPU(class LuisaVM& comp) : comp(comp) {}
    void Reset() override;
    void Step() override;

    bool Flag(enum Flag f) const;
    void setFlag(enum Flag f, bool value);

    array<uint32_t, 16> Register = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

    uint32_t& A = Register[0];
    uint32_t& B = Register[1];
    // {{{ other registers ...
    uint32_t& C = Register[2];
    uint32_t& D = Register[3];
    uint32_t& E = Register[4];
    uint32_t& F = Register[5];
    uint32_t& G = Register[6];
    uint32_t& H = Register[7];
    uint32_t& I = Register[8];
    uint32_t& J = Register[9];
    uint32_t& K = Register[10];
    uint32_t& L = Register[11];
    uint32_t& FP = Register[12];
    uint32_t& SP = Register[13];
    uint32_t& PC = Register[14];
    uint32_t& FL = Register[15];
    // }}}

    struct Parameter {
        ParameterType type;
        uint32_t      value;
    };

private:
    vector<Parameter> ParseParameters(Opcode const& opcode, uint8_t& sz) const;
    void     Apply(Parameter const& dest, uint32_t value, uint8_t sz=0);
    uint32_t Take(Parameter const& orig);

    void Push8(uint8_t value);
    void Push16(uint16_t value);
    void Push32(uint32_t value);
    uint8_t Pop8();
    uint16_t Pop16();
    uint32_t Pop32();

    class LuisaVM& comp;
};

}  // namespace luisavm

#endif
