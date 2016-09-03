#ifndef DEBUGGER_HELP_HH_
#define DEBUGGER_HELP_HH_

#include "debuggerscreen.hh"

namespace luisavm {

class DebuggerHelp : public DebuggerScreen {
public:
    explicit DebuggerHelp(Video& video) : DebuggerScreen(video) {}

    void Update() override;
};

}  // namespace luisavm

#endif
