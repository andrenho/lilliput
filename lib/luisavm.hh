#ifndef LUISAVM_HH_
#define LUISAVM_HH_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
using namespace std;

#include "assembler.hh"
#include "cpu.hh"
#include "device.hh"
#include "keyboard.hh"
#include "video.hh"

namespace luisavm {

class LuisaVM {
public:
    explicit LuisaVM(uint32_t physical_memory_size = 16*1024);

    void Reset();
    void Step();  // TODO - time
    void StepDevices();

    uint8_t  Get(uint32_t pos) const;
    void     Set(uint32_t pos, uint8_t data);
    uint16_t Get16(uint32_t pos) const;
    void     Set16(uint32_t pos, uint16_t data);
    uint32_t Get32(uint32_t pos) const;
    void     Set32(uint32_t pos, uint32_t data);

    vector<uint8_t>& PhysicalMemory() { return _physical_memory; }

    void LoadROM(string const& rom_filename);

    Video& AddVideo(Video::Callbacks const& cb);

    void RegisterKeyEvent(Keyboard::KeyPress const& kp);

    static const uint32_t COMMAND_POS = 0xFFFF0000;

    CPU&      cpu() const      { return *dynamic_cast<CPU*>(_devices[0].get()); }
    Keyboard& keyboard() const { return *dynamic_cast<Keyboard*>(_devices[1].get()); }

private:
    vector<unique_ptr<Device>> _devices;
    vector<uint8_t> _physical_memory;

    class Debugger* _debugger = nullptr;
    
    template<typename D, typename ...Args>
    D& AddDevice(Args&&... args) {
        _devices.push_back(make_unique<D>(args...));
        return *static_cast<D*>(_devices.back().get());
    }
};

void run_tests();

}  // namespace luisavm

#endif
