#ifndef DEBUGGER_H_
#define DEBUGGER_H_

#include <stdbool.h>

typedef struct Debugger Debugger;

Debugger* debugger_init(struct LVM_Computer* comp, bool active);
void debugger_free(Debugger* dbg);

bool debugger_active(Debugger* dbg);
void debugger_update(Debugger* dbg);
void debugger_keypressed(Debugger* debugger, uint32_t chr, uint8_t modifiers);

#endif
