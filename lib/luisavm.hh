#ifndef LUISAVM_HH_
#define LUISAVM_HH_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
using namespace std;

#include "device.hh"
#include "cpu.hh"
#include "video.hh"

namespace luisavm {

class LuisaVM {
public:
    explicit LuisaVM(uint32_t physical_memory_size = 16*1024);

    void Reset();
    void Step();  // TODO - time

    uint8_t  Get(uint32_t pos) const;
    void     Set(uint32_t pos, uint8_t data);
    uint16_t Get16(uint32_t pos) const;
    void     Set16(uint32_t pos, uint16_t data);
    uint32_t Get32(uint32_t pos) const;
    void     Set32(uint32_t pos, uint32_t data);

    vector<uint8_t>& PhysicalMemory() { return _physical_memory; }

    void LoadROM(vector<uint8_t> const& data);
    void LoadROM(string const& filename);

    static const uint32_t COMMAND_POS = 0xFFFF0000;

    CPU& cpu() const;

private:
    vector<unique_ptr<Device>> _devices;
    vector<uint8_t> _physical_memory;
};

void run_tests();

}  // namespace luisavm

#endif
