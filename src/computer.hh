#ifndef COMPUTER_HH_
#define COMPUTER_HH_

#include <cstdint>
#include <string>
#include <vector>
using namespace std;

namespace luisavm {

class Computer {
public:
    Computer(uint32_t physical_memory_size);

    void Reset();
    void Step();  // TODO - time

    uint8_t  Get(uint32_t pos) const;
    void     Set(uint32_t pos, uint8_t data);
    uint16_t Get16(uint32_t pos) const;
    void     Set16(uint32_t pos, uint16_t data);
    uint32_t Get32(uint32_t pos) const;
    void     Set32(uint32_t pos, uint32_t data);

    uint8_t* PhysicalMemory();
    uint32_t PhysicalMemorySize();

    void LoadROM(vector<uint8_t> const& data);
    void LoadROM(string const& filename);
};

}

#endif
