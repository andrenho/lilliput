#include "luisavm.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>

#include "device.h"

typedef enum State {
    ST_LOGICAL,
} State;

typedef struct Debugger {
    bool          active;
    LVM_Computer* comp;
    State         state;
    bool          dirty;
} Debugger;

// {{{ CONSTRUCTOR/DESTRUCTOR

Debugger*
debugger_init(LVM_Computer* comp, bool active)
{
    Debugger* dbg = calloc(sizeof(Debugger), 1);
    dbg->active = active;
    dbg->comp = comp;
    dbg->state = ST_LOGICAL;
    dbg->dirty = true;

    syslog(LOG_DEBUG, "Debugger created.");

    return dbg;
}


void
debugger_free(Debugger* dbg)
{
    free(dbg);
}

// }}}

// {{{ DRAW

extern void lvm_draw_char(LVM_Computer* comp, uint8_t c, uint16_t x, uint16_t y, uint8_t fg, uint8_t bg);
static void print(Debugger* dbg, uint16_t x, uint16_t y, uint8_t fg, uint8_t bg, const char* fmt, ...) __attribute__((format(printf, 6, 7)));

static void
print(Debugger* dbg, uint16_t x, uint16_t y, uint8_t fg, uint8_t bg, const char* fmt, ...)
{
    char buf[100];
    va_list ap;

    va_start(ap, fmt);
    int sz = vsnprintf(buf, sizeof buf, fmt,ap);
    va_end(ap);

    for(uint16_t n = 0; n < sz; ++n) {
        lvm_draw_char(dbg->comp, (uint8_t)buf[n], (uint16_t)(x + n), y, fg, bg);
    }
}

static void
draw_box(Debugger* dbg, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t fg, uint8_t bg)
{
    lvm_draw_char(dbg->comp, 218, x1, y1, fg, bg);
    lvm_draw_char(dbg->comp, 191, x2, y1, fg, bg);
    lvm_draw_char(dbg->comp, 217, x2, y2, fg, bg);
    lvm_draw_char(dbg->comp, 192, x1, y2, fg, bg);
    for(int x=x1+1; x<x2; ++x) {
        lvm_draw_char(dbg->comp, 196, x, y1, fg, bg);
        lvm_draw_char(dbg->comp, 196, x, y2, fg, bg);
    }
    for(int y=y1+1; y<y2; ++y) {
        lvm_draw_char(dbg->comp, 179, x1, y, fg, bg);
        lvm_draw_char(dbg->comp, 179, x2, y, fg, bg);
    }
}

static void
logical_update(Debugger* dbg)
{
    print(dbg, 0, 0, 10, 0, "Logical memory");
    draw_box(dbg, 1, 1, 49, 23, 10, 0);

    // addresses
    for(uint32_t addr=0; addr<21; ++addr) {
        print(dbg, 3, addr+2, 10, 0, "%08X:", addr);
        for(int i=0; i<8; ++i) {
            print(dbg, 14 + (i*3), addr+2, 10, 0, "%02X", 0);
            print(dbg, 40 + i, addr+2, 10, 0, ".");
        }
    }

    print(dbg, 34, 24, 10, 0, "Offset: %08X", 0);
}

// }}}

// {{{ PUBLIC

bool 
debugger_active(Debugger* dbg)
{
    return dbg->active;
}


void
debugger_update(Debugger* dbg)
{
    if(dbg->dirty) {
        switch(dbg->state) {
            case ST_LOGICAL:
                logical_update(dbg);
                break;
            default:
                abort();
        }
        
        dbg->dirty = false;
    }
}

// }}}
