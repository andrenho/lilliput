#include "luisavm.hh"

#include <exception>
using namespace std;

namespace luisavm {

LuisaVM::LuisaVM(uint32_t physical_memory_size)
{
    _physical_memory.resize(physical_memory_size, 0);
    _devices.push_back(make_unique<CPU>());
}


uint8_t LuisaVM::Get(uint32_t pos) const
{
    if(pos < _physical_memory.size()) {
        return _physical_memory.at(pos);
    } else if(pos < COMMAND_POS) {
        return 0;
    } else {
        throw logic_error("not implemented");
    }
}


void LuisaVM::Set(uint32_t pos, uint8_t data)
{
    if(pos < _physical_memory.size()) {
        _physical_memory[pos] = data;
    } else if(pos >= COMMAND_POS) {
        throw logic_error("not implemented");
    }
}


uint16_t LuisaVM::Get16(uint32_t pos) const
{
    uint16_t b1 = Get(pos),
             b2 = Get(pos+1);
    return b1 | static_cast<uint16_t>(b2 << 8);
}


void LuisaVM::Set16(uint32_t pos, uint16_t data)
{
    Set(pos, static_cast<uint8_t>(data));
    Set(pos+1, static_cast<uint8_t>(data >> 8));
}


uint32_t LuisaVM::Get32(uint32_t pos) const
{
    uint32_t b1 = Get(pos),
             b2 = Get(pos+1),
             b3 = Get(pos+2),
             b4 = Get(pos+3);
    return b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
}


void LuisaVM::Set32(uint32_t pos, uint32_t data)
{
    Set(pos, static_cast<uint8_t>(data));
    Set(pos+1, static_cast<uint8_t>(data >> 8));
    Set(pos+2, static_cast<uint8_t>(data >> 16));
    Set(pos+3, static_cast<uint8_t>(data >> 24));
}


CPU& LuisaVM::cpu() const
{
    return *static_cast<CPU*>(_devices[0].get());
}


}  // namespace luisavm
