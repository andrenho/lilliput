#ifndef DEBUGGER_HH_
#define DEBUGGER_HH_

#include "device.hh"
#include "video.hh"
#include "debuggerscreen.hh"

#include <memory>
#include <vector>
using namespace std;

namespace luisavm {

class Debugger : public Device {
public:
    Debugger(Video& video);
    void Step() override;

    bool Active = true;

private:
    vector<unique_ptr<DebuggerScreen>> _screens;
    uint8_t _selected = 0;
};

}  // namespace luisavm

#endif
