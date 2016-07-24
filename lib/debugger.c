#include "luisavm.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "device.h"

typedef enum State {
    ST_LOGICAL,
    ST_QUESTION,
} State;

typedef struct Logical {
    uint32_t top_addr;
} Logical;

typedef struct Question {
    State    return_to;
    char*    text;
    char*    buffer;
    uint32_t response;
} Question;

typedef struct Debugger {
    bool          active;
    LVM_Computer* comp;
    State         state;
    bool          dirty;
    Logical       logical;
    Question      question;
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
    dbg->logical = (Logical) { .top_addr = 0x0 };

    syslog(LOG_DEBUG, "Debugger created.");

    return dbg;
}


void
debugger_free(Debugger* dbg)
{
    free(dbg);
}

// }}}

// {{{ DRAW (GENERIC)

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
draw_box(Debugger* dbg, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t fg, uint8_t bg, bool clear, bool shadow)
{
    lvm_draw_char(dbg->comp, 218, x1, y1, fg, bg);
    lvm_draw_char(dbg->comp, 191, x2, y1, fg, bg);
    lvm_draw_char(dbg->comp, 217, x2, y2, fg, bg);
    lvm_draw_char(dbg->comp, 192, x1, y2, fg, bg);
    for(uint16_t x=x1+1; x<x2; ++x) {
        lvm_draw_char(dbg->comp, 196, x, y1, fg, bg);
        lvm_draw_char(dbg->comp, 196, x, y2, fg, bg);
    }
    for(uint16_t y=y1+1; y<y2; ++y) {
        lvm_draw_char(dbg->comp, 179, x1, y, fg, bg);
        lvm_draw_char(dbg->comp, 179, x2, y, fg, bg);
    }

    if(clear) {
        for(uint16_t x=x1+1; x<x2; ++x) {
            for(uint16_t y=y1+1; y<y2; ++y) {
                lvm_draw_char(dbg->comp, ' ', x, y, fg, bg);
            }
        }
    }

    if(shadow) {
        for(uint16_t x=x1+1; x<=(x2+1); ++x) {
            lvm_draw_char(dbg->comp, 176, x, y2+1, fg, bg);
        }
        for(uint16_t y=y1+1; y<=(y2+1); ++y) {
            lvm_draw_char(dbg->comp, 176, x2+1, y, fg, bg);
        }
    }
}

// }}}

// {{{ LOGICAL

static void
logical_update(Debugger* dbg)
{
    print(dbg, 0, 0, 10, 0, "Logical memory");
    print(dbg, 33, 0, 10, 8, "[F?]"); print(dbg, 38, 0, 10, 0, "- choose device");
    print(dbg, 0, 25, 10, 8, "[G]"); print(dbg, 4, 25, 10, 0, "- go to");

    draw_box(dbg, 1, 1, 50, 24, 10, 0, false, false);

    // addresses
    for(uint32_t i=0; i<22; ++i) {
        uint32_t addr = dbg->logical.top_addr + (i*0x8);
        print(dbg, 3, i+2, 10, 0, "%08X:", addr);
        for(int j=0; j<8; ++j) {
            uint8_t data = lvm_get(dbg->comp, addr+j);
            print(dbg, 15 + (j*3), i+2, 10, 0, "%02X", data);
            const char* printable = (data >= 32 && data < 127) ? (char[]) { data, '\0' } : ".";
            print(dbg, 41 + j, i+2, 10, 0, "%s", printable);
        }
    }

    print(dbg, 37, 25, 10, 0, "Offset: %08X", lvm_offset(dbg->comp));
}


static void 
logical_keypressed(Debugger* debugger, uint32_t chr, uint8_t modifiers)
{
    (void) modifiers;

    switch(chr) {
        case DOWN:
            if(debugger->logical.top_addr < 0xFFFFFF50) {
                debugger->logical.top_addr += 0x8;
                debugger->dirty = true;
            }
            break;
        case UP:
            if(debugger->logical.top_addr > 0) {
                debugger->logical.top_addr -= 0x8;
                debugger->dirty = true;
            }
            break;
        case PGUP:
            if(debugger->logical.top_addr < 0xFFFFFF50) {
                debugger->logical.top_addr += 0xB0;
                debugger->dirty = true;
            }
            break;
        case PGDOWN:
            if(debugger->logical.top_addr > 0xB0) {
                debugger->logical.top_addr -= 0xB0;
                debugger->dirty = true;
            } else if(debugger->logical.top_addr > 0x0) {
                debugger->logical.top_addr = 0x0;
                debugger->dirty = true;
            }
            break;
        case 'g':
            debugger->question = (Question) {
                .return_to = ST_LOGICAL,
                .text = "Go to address:",
                .buffer = calloc(8, 1),
                .response = 0
            };
            debugger->state = ST_QUESTION;
            debugger->dirty = true;
            break;
    }
}

// }}}

// {{{ QUESTION

static void 
question_update(Debugger* dbg)
{
    Question* q = &dbg->question;

    uint16_t width = strlen(q->text);
    if(width < 12) width = 12;

    draw_box(dbg, (CH_COLUMNS/2) - (width/2) - 2, (CH_LINES/2 - 2), (CH_COLUMNS/2) + (width/2) + 2, (CH_LINES/2 + 2), 10, 0, true, true);
    print(dbg, (CH_COLUMNS/2) - (width/2) - 1, (CH_LINES/2 - 1), 10, 0, "%s", q->text);

    print(dbg, (CH_COLUMNS/2) - (width/2) - 1, (CH_LINES/2 + 1), 10, 8, "0x        ");
    print(dbg, (CH_COLUMNS/2) - (width/2) + 1, (CH_LINES/2 + 1), 10, 8, "%s%c", q->buffer, 255);
}

static void 
question_keypressed(Debugger* debugger, uint32_t chr)
{
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
            case ST_QUESTION:
                question_update(dbg);
                break;
            default:
                abort();
        }
        
        dbg->dirty = false;
    }
}

void debugger_keypressed(Debugger* debugger, uint32_t chr, uint8_t modifiers)
{
    switch(debugger->state) {
        case ST_LOGICAL:
            logical_keypressed(debugger, chr, modifiers);
            break;
        case ST_QUESTION:
            question_keypressed(debugger, chr);
            break;
        default:
            abort();
    }
}

// }}}
