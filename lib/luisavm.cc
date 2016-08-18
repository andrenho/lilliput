#include "luisavm.hh"

#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>  // TODO
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
    if(_debugger != nullptr && _debugger->Active) {
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
    } 
    
    if(pos < COMMAND_POS) {
        return 0;
    }

    throw logic_error("not implemented");
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
LuisaVM::LoadROM(string const& rom_filename)
{
    ios::sync_with_stdio(false);

    ifstream ifs(rom_filename);
    ifstream::pos_type ps = ifs.tellg();
    if(ps < 0) {
        throw runtime_error("Error reading file " + rom_filename);
    }

    string s1, s2;

    // header
    size_t sz;
    ifs >> s1 >> s2 >> sz; /* >> "\n"; */
    if(ifs.fail() || s1 != "**" || s2 != "binary") {
        throw runtime_error("Invalid file format");
    }

    // binary
    uint32_t pos = 0;
    while(ifs >> s1 && s1 != "**") {
        if(s1.size() % 2 != 0) {
            throw runtime_error("Invalid file format (hex not paired)");
        }
        for(size_t i=0; i < s1.size(); i += 2) {
            uint8_t n = stoi(s1.substr(i, 2), nullptr, 16);
            if(pos >= _physical_memory.size()) {
                throw runtime_error("Memory is too small to accomodate such a large ROM.");
            }
            _physical_memory[pos++] = n;
        }
    }

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

// {{{ user events

void 
LuisaVM::RegisterKeyEvent(Keyboard::KeyPress const& kp)
{
    if(_debugger != nullptr && _debugger->Active) {
        if(kp.state == PRESSED) {
            _debugger->Keypressed(kp);
        }
    } else {
        keyboard().Queue.push_back(kp);
    }
}

// }}}

}  // namespace luisavm
