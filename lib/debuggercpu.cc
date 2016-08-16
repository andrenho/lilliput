#include "debuggercpu.hh"

#include <cstdio>

#include "luisavm.hh"

namespace luisavm {

void DebuggerCPU::Update() {
    _video.ClearScreen(0);
    
    _video.Print(0, 0, 10, 0, "CPU");
    _video.Print(40, 0, 10, 8, "[F1]"); _video.Print(45, 0, 10, 0, "- help");
    
    _video.DrawBox(0, 1, 49, 21, 10, 0, true, false);

    // registers
    for(size_t i=0; i<16; ++i) {
        _video.Printf((i/4)*13 + (i >= 12 ? 1 : 0) + 1, 22+(i%4), 10, 0, 
                "%s%s: %08X", (i<12) ? " " : "", _regs[i].c_str(), _comp.cpu().Register[i]);
    }
    DrawInstructions();

    // flags
    for(size_t i=0; i<6; ++i) {
        _video.Printf(50, i+15, 10, 0, "%s:%d", _flags[i].c_str(), _comp.cpu().Flag(static_cast<Flag>(i)));
    }

    _video.UpdateScreen();
}


void DebuggerCPU::DrawInstructions() const
{
    uint32_t addr = 0;
    uint8_t y = 0;
    for(uint32_t i=_code_start; ; ++i) {
        if(addr >= _top_addr) {
            if(addr != _comp.cpu().PC) {
                _video.Printf(2, y+2, 10, 0, "%08X:  %s", addr, Instruction(addr).c_str());
            } else {
                _video.Printf(1, y+2, 0, 10, "%*s", 48, "");
                _video.Printf(2, y+2, 0, 10, "%08X:  %s", addr, Instruction(addr).c_str());
            }
            ++y;
            if(y == 19) {
                break;
            }
        }
        addr += InstructionSize(addr);
    }
}

// {{{ INSTRUCTIONS

string DebuggerCPU::Instruction(uint32_t addr) const
{
    auto reg = [](uint8_t r) { return (r < 16) ? _regs[r].c_str() : "??"; };

    char buf[50];
    uint8_t op = _comp.Get(addr);

    switch(op) {
        // {{{ ...
        // movement
        case 0x01: sprintf(buf, "mov    %s, %s", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;
        case 0x02: sprintf(buf, "mov    %s, 0x%02X", reg(_comp.Get(addr+1)), _comp.Get(addr+2)); break;
        case 0x03: sprintf(buf, "mov    %s, 0x%04X", reg(_comp.Get(addr+1)), _comp.Get16(addr+2)); break;
        case 0x04: sprintf(buf, "mov    %s, 0x%08X", reg(_comp.Get(addr+1)), _comp.Get32(addr+2)); break;

        case 0x05: sprintf(buf, "movb   %s, [%s]", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;
        case 0x06: sprintf(buf, "movb   %s, [0x%08X]", reg(_comp.Get(addr+1)), _comp.Get32(addr+2)); break;
        case 0x07: sprintf(buf, "movb   [%s], %s", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;
        case 0x08: sprintf(buf, "movb   [%s], 0x%02X", reg(_comp.Get(addr+1)), _comp.Get(addr+2)); break;
        case 0x09: sprintf(buf, "movb   [%s], [%s]", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;
        case 0x0A: sprintf(buf, "movb   [%s], [0x%08X]", reg(_comp.Get(addr+1)), _comp.Get32(addr+2)); break;
        case 0x0B: sprintf(buf, "movb   [0x%08X], %s", _comp.Get32(addr+1), reg(_comp.Get(addr+5))); break;
        case 0x0C: sprintf(buf, "movb   [0x%08X], 0x%02X", _comp.Get32(addr+1), _comp.Get(addr+5)); break;
        case 0x0D: sprintf(buf, "movb   [0x%08X], [%s]", _comp.Get32(addr+1), reg(_comp.Get(addr+5))); break;
        case 0x0E: sprintf(buf, "movb   [0x%08X], [0x%08X]", _comp.Get32(addr+1), _comp.Get32(addr+5)); break;

        case 0x0F: sprintf(buf, "movw   %s, [%s]", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;
        case 0x10: sprintf(buf, "movw   %s, [0x%08X]", reg(_comp.Get(addr+1)), _comp.Get32(addr+2)); break;
        case 0x11: sprintf(buf, "movw   [%s], %s", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;
        case 0x12: sprintf(buf, "movw   [%s], 0x%04X", reg(_comp.Get(addr+1)), _comp.Get(addr+2)); break;
        case 0x13: sprintf(buf, "movw   [%s], [%s]", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;
        case 0x14: sprintf(buf, "movw   [%s], [0x%08X]", reg(_comp.Get(addr+1)), _comp.Get32(addr+2)); break;
        case 0x15: sprintf(buf, "movw   [0x%08X], %s", _comp.Get32(addr+1), reg(_comp.Get(addr+5))); break;
        case 0x16: sprintf(buf, "movw   [0x%08X], 0x%04X", _comp.Get32(addr+1), _comp.Get16(addr+5)); break;
        case 0x17: sprintf(buf, "movw   [0x%08X], [%s]", _comp.Get32(addr+1), reg(_comp.Get(addr+5))); break;
        case 0x18: sprintf(buf, "movw   [0x%08X], [0x%08X]", _comp.Get32(addr+1), _comp.Get32(addr+5)); break;

        case 0x19: sprintf(buf, "movd   %s, [%s]", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;
        case 0x1A: sprintf(buf, "movd   %s, [0x%08X]", reg(_comp.Get(addr+1)), _comp.Get32(addr+2)); break;
        case 0x1B: sprintf(buf, "movd   [%s], %s", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;
        case 0x1C: sprintf(buf, "movd   [%s], 0x%08X", reg(_comp.Get(addr+1)), _comp.Get(addr+2)); break;
        case 0x1D: sprintf(buf, "movd   [%s], [%s]", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;
        case 0x1E: sprintf(buf, "movd   [%s], [0x%08X]", reg(_comp.Get(addr+1)), _comp.Get32(addr+2)); break;
        case 0x1F: sprintf(buf, "movd   [0x%08X], %s", _comp.Get32(addr+1), reg(_comp.Get(addr+5))); break;
        case 0x20: sprintf(buf, "movd   [0x%08X], 0x%08X", _comp.Get32(addr+1), _comp.Get16(addr+5)); break;
        case 0x21: sprintf(buf, "movd   [0x%08X], [%s]", _comp.Get32(addr+1), reg(_comp.Get(addr+5))); break;
        case 0x22: sprintf(buf, "movd   [0x%08X], [0x%08X]", _comp.Get32(addr+1), _comp.Get32(addr+5)); break;

        case 0x23: sprintf(buf, "swap %s, %s", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;

        // logic
        case 0x24: sprintf(buf, "or     %s, %s", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;
        case 0x25: sprintf(buf, "or     %s, 0x%02X", reg(_comp.Get(addr+1)), _comp.Get(addr+2)); break;
        case 0x26: sprintf(buf, "or     %s, 0x%04X", reg(_comp.Get(addr+1)), _comp.Get16(addr+2)); break;
        case 0x27: sprintf(buf, "or     %s, 0x%08X", reg(_comp.Get(addr+1)), _comp.Get32(addr+2)); break;
        case 0x28: sprintf(buf, "xor    %s, %s", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;
        case 0x29: sprintf(buf, "xor    %s, 0x%02X", reg(_comp.Get(addr+1)), _comp.Get(addr+2)); break;
        case 0x2A: sprintf(buf, "xor    %s, 0x%04X", reg(_comp.Get(addr+1)), _comp.Get16(addr+2)); break;
        case 0x2B: sprintf(buf, "xor    %s, 0x%08X", reg(_comp.Get(addr+1)), _comp.Get32(addr+2)); break;
        case 0x2C: sprintf(buf, "and    %s, %s", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;
        case 0x2D: sprintf(buf, "and    %s, 0x%02X", reg(_comp.Get(addr+1)), _comp.Get(addr+2)); break;
        case 0x2E: sprintf(buf, "and    %s, 0x%04X", reg(_comp.Get(addr+1)), _comp.Get16(addr+2)); break;
        case 0x2F: sprintf(buf, "and    %s, 0x%08X", reg(_comp.Get(addr+1)), _comp.Get32(addr+2)); break;
        case 0x30: sprintf(buf, "shl    %s, %s", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;
        case 0x31: sprintf(buf, "shl    %s, 0x%02X", reg(_comp.Get(addr+1)), _comp.Get(addr+2)); break;
        case 0x32: sprintf(buf, "shr    %s, %s", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;
        case 0x33: sprintf(buf, "shr    %s, 0x%02X", reg(_comp.Get(addr+1)), _comp.Get(addr+2)); break;
        case 0x34: sprintf(buf, "not    %s", reg(_comp.Get(addr+1))); break;

        // arithmetic
        case 0x35: sprintf(buf, "add    %s, %s", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;
        case 0x36: sprintf(buf, "add    %s, 0x%02X", reg(_comp.Get(addr+1)), _comp.Get(addr+2)); break;
        case 0x37: sprintf(buf, "add    %s, 0x%04X", reg(_comp.Get(addr+1)), _comp.Get16(addr+2)); break;
        case 0x38: sprintf(buf, "add    %s, 0x%08X", reg(_comp.Get(addr+1)), _comp.Get32(addr+2)); break;
        case 0x39: sprintf(buf, "sub    %s, %s", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;
        case 0x3A: sprintf(buf, "sub    %s, 0x%02X", reg(_comp.Get(addr+1)), _comp.Get(addr+2)); break;
        case 0x3B: sprintf(buf, "sub    %s, 0x%04X", reg(_comp.Get(addr+1)), _comp.Get16(addr+2)); break;
        case 0x3C: sprintf(buf, "sub    %s, 0x%08X", reg(_comp.Get(addr+1)), _comp.Get32(addr+2)); break;
        case 0x3D: sprintf(buf, "cmp    %s, %s", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;
        case 0x3E: sprintf(buf, "cmp    %s, 0x%02X", reg(_comp.Get(addr+1)), _comp.Get(addr+2)); break;
        case 0x3F: sprintf(buf, "cmp    %s, 0x%04X", reg(_comp.Get(addr+1)), _comp.Get16(addr+2)); break;
        case 0x40: sprintf(buf, "cmp    %s, 0x%08X", reg(_comp.Get(addr+1)), _comp.Get32(addr+2)); break;
        case 0x41: sprintf(buf, "cmp    %s", reg(_comp.Get(addr+1))); break;
        case 0x42: sprintf(buf, "mul    %s, %s", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;
        case 0x43: sprintf(buf, "mul    %s, 0x%02X", reg(_comp.Get(addr+1)), _comp.Get(addr+2)); break;
        case 0x44: sprintf(buf, "mul    %s, 0x%04X", reg(_comp.Get(addr+1)), _comp.Get16(addr+2)); break;
        case 0x45: sprintf(buf, "mul    %s, 0x%08X", reg(_comp.Get(addr+1)), _comp.Get32(addr+2)); break;
        case 0x46: sprintf(buf, "idiv   %s, %s", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;
        case 0x47: sprintf(buf, "idiv   %s, 0x%02X", reg(_comp.Get(addr+1)), _comp.Get(addr+2)); break;
        case 0x48: sprintf(buf, "idiv   %s, 0x%04X", reg(_comp.Get(addr+1)), _comp.Get16(addr+2)); break;
        case 0x49: sprintf(buf, "idiv   %s, 0x%08X", reg(_comp.Get(addr+1)), _comp.Get32(addr+2)); break;
        case 0x4A: sprintf(buf, "mod    %s, %s", reg(_comp.Get(addr+1) >> 4), reg(_comp.Get(addr+1) & 0xF)); break;
        case 0x4B: sprintf(buf, "mod    %s, 0x%02X", reg(_comp.Get(addr+1)), _comp.Get(addr+2)); break;
        case 0x4C: sprintf(buf, "mod    %s, 0x%04X", reg(_comp.Get(addr+1)), _comp.Get16(addr+2)); break;
        case 0x4D: sprintf(buf, "mod    %s, 0x%08X", reg(_comp.Get(addr+1)), _comp.Get32(addr+2)); break;
        case 0x4E: sprintf(buf, "inc    %s", reg(_comp.Get(addr+1))); break;
        case 0x4F: sprintf(buf, "dec    %s", reg(_comp.Get(addr+1))); break;

        // jumps
        case 0x50: sprintf(buf, "bz     %s", reg(_comp.Get(addr+1))); break;
        case 0x51: sprintf(buf, "bz     0x%08X", _comp.Get(addr+1)); break;
        case 0x52: sprintf(buf, "bnz    %s", reg(_comp.Get(addr+1))); break;
        case 0x53: sprintf(buf, "bnz    0x%08X", _comp.Get(addr+1)); break;
        case 0x54: sprintf(buf, "bneg   %s", reg(_comp.Get(addr+1))); break;
        case 0x55: sprintf(buf, "bneg   0x%08X", _comp.Get(addr+1)); break;
        case 0x56: sprintf(buf, "bpos   %s", reg(_comp.Get(addr+1))); break;
        case 0x57: sprintf(buf, "bpos   0x%08X", _comp.Get(addr+1)); break;
        case 0x58: sprintf(buf, "bgt    %s", reg(_comp.Get(addr+1))); break;
        case 0x59: sprintf(buf, "bgt    0x%08X", _comp.Get(addr+1)); break;
        case 0x5A: sprintf(buf, "bgte   %s", reg(_comp.Get(addr+1))); break;
        case 0x5B: sprintf(buf, "bgte   0x%08X", _comp.Get(addr+1)); break;
        case 0x5C: sprintf(buf, "blt    %s", reg(_comp.Get(addr+1))); break;
        case 0x5D: sprintf(buf, "blt    0x%08X", _comp.Get(addr+1)); break;
        case 0x5E: sprintf(buf, "blte   %s", reg(_comp.Get(addr+1))); break;
        case 0x5F: sprintf(buf, "blte   0x%08X", _comp.Get(addr+1)); break;
        case 0x60: sprintf(buf, "bv     %s", reg(_comp.Get(addr+1))); break;
        case 0x61: sprintf(buf, "bv     0x%08X", _comp.Get(addr+1)); break;
        case 0x62: sprintf(buf, "bnv    %s", reg(_comp.Get(addr+1))); break;
        case 0x63: sprintf(buf, "bnv    0x%08X", _comp.Get(addr+1)); break;

        case 0x64: sprintf(buf, "jmp    %s", reg(_comp.Get(addr+1))); break;
        case 0x65: sprintf(buf, "jmp    0x%08X", _comp.Get(addr+1)); break;
        case 0x66: sprintf(buf, "jsr    %s", reg(_comp.Get(addr+1))); break;
        case 0x67: sprintf(buf, "jsr    0x%08X", _comp.Get(addr+1)); break;
        case 0x68: sprintf(buf, "ret"); break;
        case 0x69: sprintf(buf, "iret"); break;

        // stack
        case 0x6A: sprintf(buf, "pushb  %s", reg(_comp.Get(addr+1))); break;
        case 0x6B: sprintf(buf, "pushb  0x%02X", _comp.Get(addr+1)); break;
        case 0x6C: sprintf(buf, "pushw  %s", reg(_comp.Get(addr+1))); break;
        case 0x6D: sprintf(buf, "pushw  0x%04X", _comp.Get(addr+1)); break;
        case 0x6E: sprintf(buf, "pushd  %s", reg(_comp.Get(addr+1))); break;
        case 0x6F: sprintf(buf, "pushd  0x%08X", _comp.Get(addr+1)); break;
        case 0x70: sprintf(buf, "push.a"); break;
        case 0x71: sprintf(buf, "popb   %s", reg(_comp.Get(addr+1))); break;
        case 0x72: sprintf(buf, "popw   %s", reg(_comp.Get(addr+1))); break;
        case 0x73: sprintf(buf, "popd   %s", reg(_comp.Get(addr+1))); break;
        case 0x74: sprintf(buf, "pop.a"); break;
        case 0x75: sprintf(buf, "popx   %s", reg(_comp.Get(addr+1))); break;
        case 0x76: sprintf(buf, "popx   0x%02X", _comp.Get(addr+1)); break;
        case 0x77: sprintf(buf, "popx   0x%04X", _comp.Get(addr+1)); break;

        // other
        case 0x78: sprintf(buf, "nop"); break;
        case 0x79: sprintf(buf, "halt"); break;
        case 0x7A: sprintf(buf, "debugger"); break;
        // }}}

        default: sprintf(buf, "data   0x%02X", op); break;
    }
    return string(buf);
}


uint8_t DebuggerCPU::InstructionSize(uint32_t addr) const
{
    uint8_t op = _comp.Get(addr);

    switch(op) {
        // {{{ ... 
        // movement
        case 0x01: return 2;
        case 0x02: return 3;
        case 0x03: return 4;
        case 0x04: return 6;

        case 0x05: return 3;
        case 0x06: return 6;
        case 0x07: return 3;
        case 0x08: return 3;
        case 0x09: return 2;
        case 0x0A: return 6;
        case 0x0B: return 6;
        case 0x0C: return 6;
        case 0x0D: return 6;
        case 0x0E: return 9;

        case 0x0F: return 3;
        case 0x10: return 6;
        case 0x11: return 3;
        case 0x12: return 4;
        case 0x13: return 3;
        case 0x14: return 6;
        case 0x15: return 6;
        case 0x16: return 7;
        case 0x17: return 6;
        case 0x18: return 9;

        case 0x19: return 3;
        case 0x1A: return 6;
        case 0x1B: return 3;
        case 0x1C: return 6;
        case 0x1D: return 3;
        case 0x1E: return 6;
        case 0x1F: return 6;
        case 0x20: return 9;
        case 0x21: return 6;
        case 0x22: return 9;

        case 0x23: return 2;

        // logic
        case 0x24: return 2;
        case 0x25: return 3;
        case 0x26: return 4;
        case 0x27: return 6;
        case 0x28: return 2;
        case 0x29: return 3;
        case 0x2A: return 4;
        case 0x2B: return 6;
        case 0x2C: return 2;
        case 0x2D: return 3;
        case 0x2E: return 4;
        case 0x2F: return 6;
        case 0x30: return 2;
        case 0x31: return 3;
        case 0x32: return 2;
        case 0x33: return 3;
        case 0x34: return 2;

        // arithmetic
        case 0x35: return 2;
        case 0x36: return 3;
        case 0x37: return 4;
        case 0x38: return 6;
        case 0x39: return 2;
        case 0x3A: return 3;
        case 0x3B: return 4;
        case 0x3C: return 6;
        case 0x3D: return 2;
        case 0x3E: return 3;
        case 0x3F: return 4;
        case 0x40: return 6;
        case 0x41: return 2;
        case 0x42: return 2;
        case 0x43: return 3;
        case 0x44: return 4;
        case 0x45: return 6;
        case 0x46: return 2;
        case 0x47: return 3;
        case 0x48: return 4;
        case 0x49: return 6;
        case 0x4A: return 2;
        case 0x4B: return 3;
        case 0x4C: return 4;
        case 0x4D: return 6;
        case 0x4E: return 2;
        case 0x4F: return 2;

        // jump
        case 0x50: return 2;
        case 0x51: return 5;
        case 0x52: return 2;
        case 0x53: return 5;
        case 0x54: return 2;
        case 0x55: return 5;
        case 0x56: return 2;
        case 0x57: return 5;
        case 0x58: return 2;
        case 0x59: return 5;
        case 0x5A: return 2;
        case 0x5B: return 5;
        case 0x5C: return 2;
        case 0x5D: return 5;
        case 0x5E: return 2;
        case 0x5F: return 5;
        case 0x60: return 2;
        case 0x61: return 5;
        case 0x62: return 2;
        case 0x63: return 5;

        case 0x64: return 2;
        case 0x65: return 5;
        case 0x66: return 2;
        case 0x67: return 5;
        case 0x68: return 1;
        case 0x69: return 1;

        // stack
        case 0x6A: return 2;
        case 0x6B: return 2;
        case 0x6C: return 2;
        case 0x6D: return 3;
        case 0x6E: return 2;
        case 0x6F: return 5;
        case 0x70: return 1;
        case 0x71: return 2;
        case 0x72: return 2;
        case 0x73: return 2;
        case 0x74: return 1;
        case 0x75: return 2;
        case 0x76: return 2;
        case 0x77: return 3;

        // other
        case 0x78: return 1;
        case 0x79: return 1;
        case 0x7A: return 1;
        // }}}
                   
        default: return 1;
    }
}

// }}}

const vector<string> DebuggerCPU::_regs = {
    "A", "B", "C", "D", "E", "F", "G", "H", 
    "I", "J", "K", "L", "FP", "SP", "PC", "FL"
};

const vector<string> DebuggerCPU::_flags = {
    "Y", "V", "Z", "S", "G", "L",
};

}  // namespace luisavm
