#ifndef DEBUGGER_NOTIMPLEMENTED_HH_
#define DEBUGGER_NOTIMPLEMENTED_HH_

#include "debuggerscreen.hh"

namespace luisavm {

class DebuggerNotImplemented : public DebuggerScreen {
public:
    DebuggerNotImplemented(Video& video) : DebuggerScreen(video) {}

    void Update() override;
};

}  // namespace luisavm

#endif
