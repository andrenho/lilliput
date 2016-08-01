#include "cpu.hh"

#include "luisavm.hh"

namespace luisavm {

void CPU::Reset()
{
    for(uint8_t i=0; i<16; ++i) {
        Register[i] = 0x0;
    }
}


// {{{ void CPU::Step()

void CPU::Step()
{
    // find opcode
    uint32_t op = comp.Get(PC);
    if(op == 0 || op >= opcodes.size()) {
        throw runtime_error("Invalid instruction " + to_string(op));
    }
    Opcode& opcode = opcodes[op];

    // find values
    uint8_t sz;
    vector<Parameter> pars = ParseParameters(opcode, sz);

    // execute
    switch(opcode.instruction) {
        case MOV:  
            Apply(pars[0], Take(pars[1])); 
            break;
        case MOVB: 
            Apply(pars[0], static_cast<uint8_t>(Take(pars[1])), 8); 
            break;
        case MOVW: 
            Apply(pars[0], static_cast<uint16_t>(Take(pars[1])), 16);
            break;
        case MOVD: 
            Apply(pars[0], Take(pars[1]), 32);
            break;
        case SWAP: {
                uint32_t tmp = Take(pars[1]);
                Apply(pars[1], Take(pars[0]));
                Apply(pars[0], tmp);
            }
            break;
        case OR:   
            Apply(pars[0], Take(pars[0]) | Take(pars[1])); 
            break;
        case XOR:  
            Apply(pars[0], Take(pars[0]) ^ Take(pars[1])); 
            break;
        case AND:
            Apply(pars[0], Take(pars[0]) & Take(pars[1])); 
            break;
        case SHL:
            Apply(pars[0], Take(pars[0]) << Take(pars[1])); 
            break;
        case SHR:
            Apply(pars[0], Take(pars[0]) >> Take(pars[1])); 
            break;
        case NOT:
            Apply(pars[0], ~Take(pars[0])); 
            break;
        case ADD: {
                uint64_t value = static_cast<uint64_t>(Take(pars[0])) + static_cast<uint64_t>(Take(pars[1])) + static_cast<uint64_t>(Flag(Flag::Y));
                bool y = value > 0xFFFFFFFF;
                Apply(pars[0], static_cast<uint32_t>(value));
                setFlag(Flag::Y, y);
            }
            break;
        case SUB: {
                int64_t value = static_cast<int64_t>(Take(pars[0])) - static_cast<int64_t>(Take(pars[1])) - static_cast<int64_t>(Flag(Flag::Y));
                bool y = value < 0;
                Apply(pars[0], static_cast<uint32_t>(value));
                setFlag(Flag::Y, y);
            }
            break;
        case CMP: {
                uint32_t p0 = Take(pars[0]), p1 = pars.size() == 1 ? p0 : Take(pars[1]);
                uint32_t diff = p0 - p1 - static_cast<uint32_t>(Flag(Flag::Y));
                setFlag(Flag::Z, diff == 0);
                setFlag(Flag::S, (diff >> 31) & 1);
                setFlag(Flag::V, false);
                setFlag(Flag::Y, (static_cast<int64_t>(p0) - static_cast<int64_t>(p1) - static_cast<int64_t>(Flag(Flag::Y))) < 0);
                setFlag(Flag::GT, p0 > p1);
                setFlag(Flag::LT, p1 > p0);
            }
            break;
        case MUL: {
                uint64_t value = static_cast<uint64_t>(Take(pars[0])) * static_cast<uint64_t>(Take(pars[1]));
                bool v = value > 0xFFFFFFFF;
                Apply(pars[0], static_cast<uint32_t>(value));
                setFlag(Flag::V, v);
            }
            break;
        case IDIV: Apply(pars[0], Take(pars[0]) / Take(pars[1])); break;
        case MOD:  Apply(pars[0], Take(pars[0]) % Take(pars[1])); break;
        case INC: {
                bool y = (static_cast<uint64_t>(Take(pars[0])) + 1) > 0xFFFFFFFF;
                Apply(pars[0], Take(pars[0])+1);
                setFlag(Flag::Y, y);
            }
            break;
        case DEC: {
                bool y = (static_cast<int64_t>(Take(pars[0])) + 1) < 0;
                Apply(pars[0], Take(pars[0])-1);
                setFlag(Flag::Y, y);
            }
            break;
        case BZ:
            if(Flag(Flag::Z)) { 
                PC = Take(pars[0]); 
                return; 
            }
            break;
        case BNZ:
            if(!Flag(Flag::Z)) {
                PC = Take(pars[0]);
                return;
            } 
            break;
        case BNEG:
            if(Flag(Flag::S)) {
                PC = Take(pars[0]);
                return;
            }
            break;
        case BPOS:
            if(!Flag(Flag::S)) {
                PC = Take(pars[0]);
                return;
            }
            break;
        case BGT:
            if(Flag(Flag::GT) && !Flag(Flag::Z)) {
                PC = Take(pars[0]);
                return;
            }
            break;
        case BGTE:
            if(Flag(Flag::GT) && Flag(Flag::Z)) {
                PC = Take(pars[0]);
                return;
            }
            break;
        case BLT:
            if(Flag(Flag::LT) && !Flag(Flag::Z)) {
                PC = Take(pars[0]);
                return;
            }
            break;
        case BLTE:
            if(Flag(Flag::LT) && Flag(Flag::Z)) {
                PC = Take(pars[0]);
                return;
            }
            break;
        case BV:
            if(Flag(Flag::V)) {
                PC = Take(pars[0]);
                return;
            }
            break;
        case BNV:
            if(!Flag(Flag::V)) {
                PC = Take(pars[0]);
                return;
            }
            break;
        case JMP:
            PC = Take(pars[0]);
            return;
/*
        case JSR:    ADVANCE_PC(pars); _push32(cpu, PC); PC = Take(pars[0]); return;
        case RET:    PC = _pop32(cpu); return;
        case HALT:   abort();  // TODO
        case IRET:   abort();  // TODO
        case PUSHB:  _push8(cpu, static_cast<uint8_t>(Take(pars[0]))); break;
        case PUSHW:  _push16(cpu, static_cast<uint16_t>(Take(pars[0]))); break;
        case PUSHD:  _push32(cpu, Take(pars[0])); break;
        case PUSH_A: for(int i=0; i<=11; ++i) _push32(cpu, cpu->reg[i]); break;
        case POPB:   Apply(pars[0], _pop8(cpu)); break;
        case POPW:   Apply(pars[0], _pop16(cpu)); break;
        case POPD:   Apply(pars[0], _pop32(cpu)); break;
        case POP_A:  for(int i=11; i>=0; --i) cpu->reg[i] = _pop32(cpu); break;
        case POPX:   for(size_t i=0; i<Take(pars[0]); ++i) _pop8(cpu); break;
        case NOP:    break;
        case DBG:    lvm_addbreakpoint(cpu, rPC+1); break;
        case INVALID:
        default:

 */
    }

    PC += sz;
}


vector<CPU::Parameter> CPU::ParseParameters(Opcode const& opcode, uint8_t& sz) const
{
    uint32_t pos = PC + 1;
    sz = 1;
    vector<Parameter> par;
    int i = 0;
    for(ParameterType pt: opcode.parameter) {
        switch(pt) {
            case REG: case INDREG: case V8:
                sz += 1;
                par.push_back({ pt, comp.Get(pos) });
                pos += 1;
                break;
            case V16:
                sz += 2;
                par.push_back({ pt, comp.Get16(pos) });
                pos += 2;
                break;
            case V32: case INDV32:
                sz += 4;
                par.push_back({ pt, comp.Get32(pos) });
                pos += 4;
                break;
        }
        ++i;
    }

    if(opcode.parameter.size() == 2 && (opcode.parameter[0] == REG || opcode.parameter[0] == INDREG)
                                    && (opcode.parameter[1] == REG || opcode.parameter[1] == INDREG)) {
        par[1].value = par[0].value & 0xF;
        par[0].value >>= 4;
        --sz;
    }

    return par;
}


void CPU::Apply(Parameter const& dest, uint32_t value, uint8_t sz)
{
    setFlag(Z, value == 0);
    setFlag(S, (value >> 31) & 1);
    setFlag(V, false);
    setFlag(Y, false);
    setFlag(GT, false);
    setFlag(LT, false);

    switch(dest.type) {
        case REG:
            if(dest.value >= 16) {
                throw logic_error("Invalid register");
            }
            Register[dest.value] = value;
            break;
        case INDREG:
            if(dest.value >= 16) {
                throw logic_error("Invalid register");
            }
            switch(sz) {
                case 8: 
                    comp.Set(Register[dest.value], static_cast<uint8_t>(value)); 
                    break;
                case 16: 
                    comp.Set16(Register[dest.value], static_cast<uint16_t>(value)); 
                    break;
                case 32:
                    comp.Set32(Register[dest.value], value); 
                    break;
                default:
                    throw logic_error("Invalid option");
            }
            break;
        case INDV32:
            switch(sz) {
                case 8: 
                    comp.Set(dest.value, static_cast<uint8_t>(value)); 
                    break;
                case 16: 
                    comp.Set16(dest.value, static_cast<uint16_t>(value)); 
                    break;
                case 32:
                    comp.Set32(dest.value, value); 
                    break;
                default:
                    throw logic_error("Invalid option");
            }
            break;
        case V8: case V16: case V32: default:
            throw logic_error("Invalid option");
    }
}


uint32_t CPU::Take(Parameter const& orig)
{
    switch(orig.type) {
        case REG:                    return Register[orig.value];
        case V8: case V16: case V32: return orig.value;
        case INDV32:                 return comp.Get32(orig.value);
        case INDREG:                 return comp.Get32(Register[orig.value]);
        default: 
            throw logic_error("Invalid option");
    }
}


// }}}


bool CPU::Flag(enum Flag f) const
{
    return static_cast<bool>((FL >> static_cast<int>(f)) & 1);
}


void CPU::setFlag(enum Flag f, bool value)
{
    int64_t new_value = FL;
    new_value ^= (-value ^ new_value) & (1 << static_cast<int>(f));
    FL = static_cast<uint32_t>(new_value);
}


}  // namespace luisavm
