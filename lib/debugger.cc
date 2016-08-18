#include "debugger.hh"

#include "debuggerhelp.hh"

namespace luisavm {

Debugger::Debugger(Video& video)
{
    _screens.push_back(make_unique<DebuggerHelp>(video));
}


void 
Debugger::Step()
{
    if(_screens[_selected]->dirty) {
        _screens[_selected]->Update();
        _screens[_selected]->dirty = true;
    }
}


}  // namespace luisavm
