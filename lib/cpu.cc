#include "cpu.hh"

namespace luisavm {

CPU::CPU()
{
}


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
