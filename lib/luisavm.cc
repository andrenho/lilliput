#include "luisavm.hh"

#include <exception>
#include <fstream>
using namespace std;

#include "debugger.hh"

namespace luisavm {

LuisaVM::LuisaVM(uint32_t physical_memory_size)
{
    _physical_memory.resize(physical_memory_size, 0);
    AddDevice<CPU>(*this);
    AddDevice<Keyboard>();
}

// {{{ step/reset

void LuisaVM::Reset()
{
    for(auto& dev: _devices) {
        dev->Reset();
    }
}


void LuisaVM::Step() 
{
    if(_debugger && _debugger->Active) {
        _debugger->Step();
    } else {
        StepDevices();
    }
}

void LuisaVM::StepDevices() 
{
    for(auto& dev: _devices) {
        dev->Step();
    }
}

// }}}

// {{{ memory management

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

// }}}

// {{{ rom loading

void
LuisaVM::LoadROM(string const& rom_filename, string const& map_filename)
{
    ifstream ifs(rom_filename, ios::binary|ios::ate);
    ifstream::pos_type pos = ifs.tellg();
    if(pos < 0) {
        throw runtime_error("Error reading file " + rom_filename);
    }

    if(static_cast<size_t>(pos) >= PhysicalMemory().size()) {
        throw runtime_error("Memory is too small to accomodate such a large ROM.");
    }

    ifs.seekg(0, ios::beg);
    ifs.read(reinterpret_cast<char*>(&PhysicalMemory()[0]), pos);

    // TODO - load map
}

// }}}

// {{{ device management

Video& LuisaVM::AddVideo(Video::Callbacks const& cb)
{
    Video& video = AddDevice<Video>(cb);
    _debugger = &AddDevice<Debugger>(*this, video);
    return video;
}

// }}}

}  // namespace luisavm
