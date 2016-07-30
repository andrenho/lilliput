#include "luisavm.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "device.h"

typedef enum State {
    ST_LOGICAL,
    ST_PHYSICAL,
    ST_CPU,
    ST_QUESTION,
} State;

typedef struct Logical {
    uint32_t top_addr;
} Logical;

typedef struct Physical {
    uint32_t top_addr;
} Physical;

typedef struct CPU {
    uint32_t code_start;
    uint32_t top_addr;
} CPU;

typedef struct Question {
    State    return_to;
    const char* text;
    char     buffer[9];
    uint32_t response;
    bool     ok;
} Question;

typedef struct Debugger {
    bool          active;
    LVM_Computer* comp;
    State         state;
    bool          dirty;
    Logical       logical;
    Physical      physical;
    CPU           cpu;
    Question      question;
} Debugger;

// {{{ CONSTRUCTOR/DESTRUCTOR

Debugger*
debugger_init(LVM_Computer* comp, bool active)
{
    Debugger* dbg = calloc(sizeof(Debugger), 1);
    dbg->active = active;
    dbg->comp = comp;
    dbg->state = ST_CPU;
    dbg->dirty = true;
    dbg->logical = (Logical) { .top_addr = 0x0 };
    dbg->physical = (Physical) { .top_addr = 0x0 };
    dbg->cpu = (CPU) { .code_start = 0x0, .top_addr = 0x0 };

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
extern void lvm_clrscr(LVM_Computer* comp);
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
    if(dbg->question.ok) {
        uint32_t addr = dbg->question.response / 8;
        dbg->logical.top_addr = addr * 8;
        if(dbg->logical.top_addr > 0xFFFFFF50) {
            dbg->logical.top_addr = 0xFFFFFF50;
        }
        dbg->question.ok = false;
    }

    print(dbg, 0, 0, 10, 0, "Logical memory");
    print(dbg, 33, 0, 10, 8, "[F?]"); print(dbg, 38, 0, 10, 0, "- choose device");
    print(dbg, 0, 25, 10, 8, "[G]"); print(dbg, 4, 25, 10, 0, "- go to");

    draw_box(dbg, 1, 1, 50, 24, 10, 0, true, false);

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
        case PGDOWN:
            if(debugger->logical.top_addr < 0xFFFFFF50) {
                debugger->logical.top_addr += 0xB0;
                debugger->dirty = true;
            }
            break;
        case PGUP:
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
                .response = 0,
                .ok = false,
            };
            memset(debugger->question.buffer, 0, sizeof debugger->question.buffer);
            debugger->state = ST_QUESTION;
            debugger->dirty = true;
            break;
    }
}

// }}}

// {{{ PHYSICAL

static void
physical_update(Debugger* dbg)
{
    if(dbg->question.ok) {
        uint32_t addr = dbg->question.response / 8;
        dbg->physical.top_addr = addr * 8;
        if(dbg->physical.top_addr > lvm_physicalmemorysz(dbg->comp) - 0xB0) {
            dbg->physical.top_addr = lvm_physicalmemorysz(dbg->comp) - 0xB0;
        }
        dbg->question.ok = false;
    }

    print(dbg, 0, 0, 10, 0, "Physical memory");
    print(dbg, 33, 0, 10, 8, "[F?]"); print(dbg, 38, 0, 10, 0, "- choose device");
    print(dbg, 0, 25, 10, 8, "[G]"); print(dbg, 4, 25, 10, 0, "- go to");

    draw_box(dbg, 1, 1, 50, 24, 10, 0, true, false);

    // addresses
    for(uint32_t i=0; i<22; ++i) {
        uint32_t addr = dbg->physical.top_addr + (i*0x8);
        print(dbg, 3, i+2, 10, 0, "%08X:", addr);
        for(int j=0; j<8; ++j) {
            if(addr+j < lvm_physicalmemorysz(dbg->comp)) {
                uint8_t data = lvm_physicalmemory(dbg->comp)[addr+j];
                print(dbg, 15 + (j*3), i+2, 10, 0, "%02X", data);
                const char* printable = (data >= 32 && data < 127) ? (char[]) { data, '\0' } : ".";
                print(dbg, 41 + j, i+2, 10, 0, "%s", printable);
            } else {
                print(dbg, 15 + (j*3), i+2, 10, 0, "   ");
                print(dbg, 41 + j, i+2, 10, 0, " ");
            }
        }
    }

    print(dbg, 37, 25, 10, 0, "Offset: %08X", lvm_offset(dbg->comp));
}


static void 
physical_keypressed(Debugger* debugger, uint32_t chr, uint8_t modifiers)
{
    (void) modifiers;

    switch(chr) {
        case DOWN:
            if(debugger->physical.top_addr < lvm_physicalmemorysz(debugger->comp) - 0xB0) {
                debugger->physical.top_addr += 0x8;
                debugger->dirty = true;
            }
            break;
        case UP:
            if(debugger->physical.top_addr > 0) {
                debugger->physical.top_addr -= 0x8;
                debugger->dirty = true;
            }
            break;
        case PGDOWN:
            if(debugger->physical.top_addr < (lvm_physicalmemorysz(debugger->comp) - 0xB0)) {
                debugger->physical.top_addr += 0xB0;
                debugger->dirty = true;
            } else {
                debugger->physical.top_addr = lvm_physicalmemorysz(debugger->comp) - 0xB0;
            }
            break;
        case PGUP:
            if(debugger->physical.top_addr > 0xB0) {
                debugger->physical.top_addr -= 0xB0;
                debugger->dirty = true;
            } else if(debugger->physical.top_addr > 0x0) {
                debugger->physical.top_addr = 0x0;
                debugger->dirty = true;
            }
            break;
        case 'g':
            debugger->question = (Question) {
                .return_to = ST_PHYSICAL,
                .text = "Go to address:",
                .response = 0,
                .ok = false,
            };
            memset(debugger->question.buffer, 0, sizeof debugger->question.buffer);
            debugger->state = ST_QUESTION;
            debugger->dirty = true;
            break;
    }
}

// }}}

// {{{ CPU

static const char* regs[] = {
    "A", "B", "C", "D", "E", "F", "G", "H", 
    "I", "J", "K", "L", "FP", "SP", "PC", "FL"
};

static const char* flags[] = {
    "Y", "V", "Z", "S", "G", "L",
};

static const char* reg(uint8_t r)
{
    if(r < 16) {
        return regs[r];
    } else {
        return "??";
    }
}

static void
cpu_inst(LVM_Computer* comp, uint32_t addr, char buf[40])
{
    uint8_t op = lvm_get(comp, addr);

    switch(op) {
        // {{{ ...
        // movement
        case 0x01: sprintf(buf, "mov    %s, %s", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;
        case 0x02: sprintf(buf, "mov    %s, 0x%02X", reg(lvm_get(comp, addr+1)), lvm_get(comp, addr+2)); break;
        case 0x03: sprintf(buf, "mov    %s, 0x%04X", reg(lvm_get(comp, addr+1)), lvm_get16(comp, addr+2)); break;
        case 0x04: sprintf(buf, "mov    %s, 0x%08X", reg(lvm_get(comp, addr+1)), lvm_get32(comp, addr+2)); break;

        case 0x05: sprintf(buf, "movb   %s, [%s]", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;
        case 0x06: sprintf(buf, "movb   %s, [0x%08X]", reg(lvm_get(comp, addr+1)), lvm_get32(comp, addr+2)); break;
        case 0x07: sprintf(buf, "movb   [%s], %s", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;
        case 0x08: sprintf(buf, "movb   [%s], 0x%02X", reg(lvm_get(comp, addr+1)), lvm_get(comp, addr+2)); break;
        case 0x09: sprintf(buf, "movb   [%s], [%s]", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;
        case 0x0A: sprintf(buf, "movb   [%s], [0x%08X]", reg(lvm_get(comp, addr+1)), lvm_get32(comp, addr+2)); break;
        case 0x0B: sprintf(buf, "movb   [0x%08X], %s", lvm_get32(comp, addr+1), reg(lvm_get(comp, addr+5))); break;
        case 0x0C: sprintf(buf, "movb   [0x%08X], 0x%02X", lvm_get32(comp, addr+1), lvm_get(comp, addr+5)); break;
        case 0x0D: sprintf(buf, "movb   [0x%08X], [%s]", lvm_get32(comp, addr+1), reg(lvm_get(comp, addr+5))); break;
        case 0x0E: sprintf(buf, "movb   [0x%08X], [0x%08X]", lvm_get32(comp, addr+1), lvm_get32(comp, addr+5)); break;

        case 0x0F: sprintf(buf, "movw   %s, [%s]", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;
        case 0x10: sprintf(buf, "movw   %s, [0x%08X]", reg(lvm_get(comp, addr+1)), lvm_get32(comp, addr+2)); break;
        case 0x11: sprintf(buf, "movw   [%s], %s", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;
        case 0x12: sprintf(buf, "movw   [%s], 0x%04X", reg(lvm_get(comp, addr+1)), lvm_get(comp, addr+2)); break;
        case 0x13: sprintf(buf, "movw   [%s], [%s]", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;
        case 0x14: sprintf(buf, "movw   [%s], [0x%08X]", reg(lvm_get(comp, addr+1)), lvm_get32(comp, addr+2)); break;
        case 0x15: sprintf(buf, "movw   [0x%08X], %s", lvm_get32(comp, addr+1), reg(lvm_get(comp, addr+5))); break;
        case 0x16: sprintf(buf, "movw   [0x%08X], 0x%04X", lvm_get32(comp, addr+1), lvm_get16(comp, addr+5)); break;
        case 0x17: sprintf(buf, "movw   [0x%08X], [%s]", lvm_get32(comp, addr+1), reg(lvm_get(comp, addr+5))); break;
        case 0x18: sprintf(buf, "movw   [0x%08X], [0x%08X]", lvm_get32(comp, addr+1), lvm_get32(comp, addr+5)); break;

        case 0x19: sprintf(buf, "movd   %s, [%s]", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;
        case 0x1A: sprintf(buf, "movd   %s, [0x%08X]", reg(lvm_get(comp, addr+1)), lvm_get32(comp, addr+2)); break;
        case 0x1B: sprintf(buf, "movd   [%s], %s", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;
        case 0x1C: sprintf(buf, "movd   [%s], 0x%08X", reg(lvm_get(comp, addr+1)), lvm_get(comp, addr+2)); break;
        case 0x1D: sprintf(buf, "movd   [%s], [%s]", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;
        case 0x1E: sprintf(buf, "movd   [%s], [0x%08X]", reg(lvm_get(comp, addr+1)), lvm_get32(comp, addr+2)); break;
        case 0x1F: sprintf(buf, "movd   [0x%08X], %s", lvm_get32(comp, addr+1), reg(lvm_get(comp, addr+5))); break;
        case 0x20: sprintf(buf, "movd   [0x%08X], 0x%08X", lvm_get32(comp, addr+1), lvm_get16(comp, addr+5)); break;
        case 0x21: sprintf(buf, "movd   [0x%08X], [%s]", lvm_get32(comp, addr+1), reg(lvm_get(comp, addr+5))); break;
        case 0x22: sprintf(buf, "movd   [0x%08X], [0x%08X]", lvm_get32(comp, addr+1), lvm_get32(comp, addr+5)); break;

        case 0x23: sprintf(buf, "swap %s, %s", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;

        // logic
        case 0x24: sprintf(buf, "or     %s, %s", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;
        case 0x25: sprintf(buf, "or     %s, 0x%02X", reg(lvm_get(comp, addr+1)), lvm_get(comp, addr+2)); break;
        case 0x26: sprintf(buf, "or     %s, 0x%04X", reg(lvm_get(comp, addr+1)), lvm_get16(comp, addr+2)); break;
        case 0x27: sprintf(buf, "or     %s, 0x%08X", reg(lvm_get(comp, addr+1)), lvm_get32(comp, addr+2)); break;
        case 0x28: sprintf(buf, "xor    %s, %s", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;
        case 0x29: sprintf(buf, "xor    %s, 0x%02X", reg(lvm_get(comp, addr+1)), lvm_get(comp, addr+2)); break;
        case 0x2A: sprintf(buf, "xor    %s, 0x%04X", reg(lvm_get(comp, addr+1)), lvm_get16(comp, addr+2)); break;
        case 0x2B: sprintf(buf, "xor    %s, 0x%08X", reg(lvm_get(comp, addr+1)), lvm_get32(comp, addr+2)); break;
        case 0x2C: sprintf(buf, "and    %s, %s", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;
        case 0x2D: sprintf(buf, "and    %s, 0x%02X", reg(lvm_get(comp, addr+1)), lvm_get(comp, addr+2)); break;
        case 0x2E: sprintf(buf, "and    %s, 0x%04X", reg(lvm_get(comp, addr+1)), lvm_get16(comp, addr+2)); break;
        case 0x2F: sprintf(buf, "and    %s, 0x%08X", reg(lvm_get(comp, addr+1)), lvm_get32(comp, addr+2)); break;
        case 0x30: sprintf(buf, "shl    %s, %s", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;
        case 0x31: sprintf(buf, "shl    %s, 0x%02X", reg(lvm_get(comp, addr+1)), lvm_get(comp, addr+2)); break;
        case 0x32: sprintf(buf, "shr    %s, %s", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;
        case 0x33: sprintf(buf, "shr    %s, 0x%02X", reg(lvm_get(comp, addr+1)), lvm_get(comp, addr+2)); break;
        case 0x34: sprintf(buf, "not    %s", reg(lvm_get(comp, addr+1))); break;

        // arithmetic
        case 0x35: sprintf(buf, "add    %s, %s", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;
        case 0x36: sprintf(buf, "add    %s, 0x%02X", reg(lvm_get(comp, addr+1)), lvm_get(comp, addr+2)); break;
        case 0x37: sprintf(buf, "add    %s, 0x%04X", reg(lvm_get(comp, addr+1)), lvm_get16(comp, addr+2)); break;
        case 0x38: sprintf(buf, "add    %s, 0x%08X", reg(lvm_get(comp, addr+1)), lvm_get32(comp, addr+2)); break;
        case 0x39: sprintf(buf, "sub    %s, %s", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;
        case 0x3A: sprintf(buf, "sub    %s, 0x%02X", reg(lvm_get(comp, addr+1)), lvm_get(comp, addr+2)); break;
        case 0x3B: sprintf(buf, "sub    %s, 0x%04X", reg(lvm_get(comp, addr+1)), lvm_get16(comp, addr+2)); break;
        case 0x3C: sprintf(buf, "sub    %s, 0x%08X", reg(lvm_get(comp, addr+1)), lvm_get32(comp, addr+2)); break;
        case 0x3D: sprintf(buf, "cmp    %s, %s", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;
        case 0x3E: sprintf(buf, "cmp    %s, 0x%02X", reg(lvm_get(comp, addr+1)), lvm_get(comp, addr+2)); break;
        case 0x3F: sprintf(buf, "cmp    %s, 0x%04X", reg(lvm_get(comp, addr+1)), lvm_get16(comp, addr+2)); break;
        case 0x40: sprintf(buf, "cmp    %s, 0x%08X", reg(lvm_get(comp, addr+1)), lvm_get32(comp, addr+2)); break;
        case 0x41: sprintf(buf, "cmp    %s", reg(lvm_get(comp, addr+1))); break;
        case 0x42: sprintf(buf, "mul    %s, %s", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;
        case 0x43: sprintf(buf, "mul    %s, 0x%02X", reg(lvm_get(comp, addr+1)), lvm_get(comp, addr+2)); break;
        case 0x44: sprintf(buf, "mul    %s, 0x%04X", reg(lvm_get(comp, addr+1)), lvm_get16(comp, addr+2)); break;
        case 0x45: sprintf(buf, "mul    %s, 0x%08X", reg(lvm_get(comp, addr+1)), lvm_get32(comp, addr+2)); break;
        case 0x46: sprintf(buf, "idiv   %s, %s", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;
        case 0x47: sprintf(buf, "idiv   %s, 0x%02X", reg(lvm_get(comp, addr+1)), lvm_get(comp, addr+2)); break;
        case 0x48: sprintf(buf, "idiv   %s, 0x%04X", reg(lvm_get(comp, addr+1)), lvm_get16(comp, addr+2)); break;
        case 0x49: sprintf(buf, "idiv   %s, 0x%08X", reg(lvm_get(comp, addr+1)), lvm_get32(comp, addr+2)); break;
        case 0x4A: sprintf(buf, "mod    %s, %s", reg(lvm_get(comp, addr+1) >> 4), reg(lvm_get(comp, addr+1) & 0xF)); break;
        case 0x4B: sprintf(buf, "mod    %s, 0x%02X", reg(lvm_get(comp, addr+1)), lvm_get(comp, addr+2)); break;
        case 0x4C: sprintf(buf, "mod    %s, 0x%04X", reg(lvm_get(comp, addr+1)), lvm_get16(comp, addr+2)); break;
        case 0x4D: sprintf(buf, "mod    %s, 0x%08X", reg(lvm_get(comp, addr+1)), lvm_get32(comp, addr+2)); break;
        case 0x4E: sprintf(buf, "inc    %s", reg(lvm_get(comp, addr+1))); break;
        case 0x4F: sprintf(buf, "dec    %s", reg(lvm_get(comp, addr+1))); break;

        // jumps
        case 0x50: sprintf(buf, "bz     %s", reg(lvm_get(comp, addr+1))); break;
        case 0x51: sprintf(buf, "bz     0x%08X", lvm_get(comp, addr+1)); break;
        case 0x52: sprintf(buf, "bnz    %s", reg(lvm_get(comp, addr+1))); break;
        case 0x53: sprintf(buf, "bnz    0x%08X", lvm_get(comp, addr+1)); break;
        case 0x54: sprintf(buf, "bneg   %s", reg(lvm_get(comp, addr+1))); break;
        case 0x55: sprintf(buf, "bneg   0x%08X", lvm_get(comp, addr+1)); break;
        case 0x56: sprintf(buf, "bpos   %s", reg(lvm_get(comp, addr+1))); break;
        case 0x57: sprintf(buf, "bpos   0x%08X", lvm_get(comp, addr+1)); break;
        case 0x58: sprintf(buf, "bgt    %s", reg(lvm_get(comp, addr+1))); break;
        case 0x59: sprintf(buf, "bgt    0x%08X", lvm_get(comp, addr+1)); break;
        case 0x5A: sprintf(buf, "bgte   %s", reg(lvm_get(comp, addr+1))); break;
        case 0x5B: sprintf(buf, "bgte   0x%08X", lvm_get(comp, addr+1)); break;
        case 0x5C: sprintf(buf, "blt    %s", reg(lvm_get(comp, addr+1))); break;
        case 0x5D: sprintf(buf, "blt    0x%08X", lvm_get(comp, addr+1)); break;
        case 0x5E: sprintf(buf, "blte   %s", reg(lvm_get(comp, addr+1))); break;
        case 0x5F: sprintf(buf, "blte   0x%08X", lvm_get(comp, addr+1)); break;
        case 0x60: sprintf(buf, "bv     %s", reg(lvm_get(comp, addr+1))); break;
        case 0x61: sprintf(buf, "bv     0x%08X", lvm_get(comp, addr+1)); break;
        case 0x62: sprintf(buf, "bnv    %s", reg(lvm_get(comp, addr+1))); break;
        case 0x63: sprintf(buf, "bnv    0x%08X", lvm_get(comp, addr+1)); break;

        case 0x64: sprintf(buf, "jmp    %s", reg(lvm_get(comp, addr+1))); break;
        case 0x65: sprintf(buf, "jmp    0x%08X", lvm_get(comp, addr+1)); break;
        case 0x66: sprintf(buf, "jsr    %s", reg(lvm_get(comp, addr+1))); break;
        case 0x67: sprintf(buf, "jsr    0x%08X", lvm_get(comp, addr+1)); break;
        case 0x68: sprintf(buf, "ret"); break;
        case 0x69: sprintf(buf, "iret"); break;

        // stack
        case 0x6A: sprintf(buf, "pushb  %s", reg(lvm_get(comp, addr+1))); break;
        case 0x6B: sprintf(buf, "pushb  0x%02X", lvm_get(comp, addr+1)); break;
        case 0x6C: sprintf(buf, "pushw  %s", reg(lvm_get(comp, addr+1))); break;
        case 0x6D: sprintf(buf, "pushw  0x%04X", lvm_get(comp, addr+1)); break;
        case 0x6E: sprintf(buf, "pushd  %s", reg(lvm_get(comp, addr+1))); break;
        case 0x6F: sprintf(buf, "pushd  0x%08X", lvm_get(comp, addr+1)); break;
        case 0x70: sprintf(buf, "push.a"); break;
        case 0x71: sprintf(buf, "popb   %s", reg(lvm_get(comp, addr+1))); break;
        case 0x72: sprintf(buf, "popw   %s", reg(lvm_get(comp, addr+1))); break;
        case 0x73: sprintf(buf, "popd   %s", reg(lvm_get(comp, addr+1))); break;
        case 0x74: sprintf(buf, "pop.a"); break;
        case 0x75: sprintf(buf, "popx   %s", reg(lvm_get(comp, addr+1))); break;
        case 0x76: sprintf(buf, "popx   0x%02X", lvm_get(comp, addr+1)); break;
        case 0x77: sprintf(buf, "popx   0x%04X", lvm_get(comp, addr+1)); break;

        // other
        case 0x78: sprintf(buf, "nop"); break;
        case 0x79: sprintf(buf, "halt"); break;
        case 0x7A: sprintf(buf, "dbg"); break;
        // }}}

        default: sprintf(buf, "data   0x%02X", op); break;
    }
}


static uint8_t
cpu_inst_sz(LVM_Computer* comp, uint32_t addr)
{
    uint8_t op = lvm_get(comp, addr);

    switch(op) {
        // {{{ ... 
        // movement
        case 0x01: return 2;
        case 0x02: return 3;
        case 0x03: return 4;
        case 0x04: return 6;

        case 0x05: return 3;
        case 0x06: return 6;
        case 0x07: return 3;
        case 0x08: return 3;
        case 0x09: return 2;
        case 0x0A: return 6;
        case 0x0B: return 6;
        case 0x0C: return 6;
        case 0x0D: return 6;
        case 0x0E: return 9;

        case 0x0F: return 3;
        case 0x10: return 6;
        case 0x11: return 3;
        case 0x12: return 4;
        case 0x13: return 3;
        case 0x14: return 6;
        case 0x15: return 6;
        case 0x16: return 7;
        case 0x17: return 6;
        case 0x18: return 9;

        case 0x19: return 3;
        case 0x1A: return 6;
        case 0x1B: return 3;
        case 0x1C: return 6;
        case 0x1D: return 3;
        case 0x1E: return 6;
        case 0x1F: return 6;
        case 0x20: return 9;
        case 0x21: return 6;
        case 0x22: return 9;

        case 0x23: return 2;

        // logic
        case 0x24: return 2;
        case 0x25: return 3;
        case 0x26: return 4;
        case 0x27: return 6;
        case 0x28: return 2;
        case 0x29: return 3;
        case 0x2A: return 4;
        case 0x2B: return 6;
        case 0x2C: return 2;
        case 0x2D: return 3;
        case 0x2E: return 4;
        case 0x2F: return 6;
        case 0x30: return 2;
        case 0x31: return 3;
        case 0x32: return 2;
        case 0x33: return 3;
        case 0x34: return 2;

        // arithmetic
        case 0x35: return 2;
        case 0x36: return 3;
        case 0x37: return 4;
        case 0x38: return 6;
        case 0x39: return 2;
        case 0x3A: return 3;
        case 0x3B: return 4;
        case 0x3C: return 6;
        case 0x3D: return 2;
        case 0x3E: return 3;
        case 0x3F: return 4;
        case 0x40: return 6;
        case 0x41: return 2;
        case 0x42: return 2;
        case 0x43: return 3;
        case 0x44: return 4;
        case 0x45: return 6;
        case 0x46: return 2;
        case 0x47: return 3;
        case 0x48: return 4;
        case 0x49: return 6;
        case 0x4A: return 2;
        case 0x4B: return 3;
        case 0x4C: return 4;
        case 0x4D: return 6;
        case 0x4E: return 2;
        case 0x4F: return 2;

        // jump
        case 0x50: return 2;
        case 0x51: return 5;
        case 0x52: return 2;
        case 0x53: return 5;
        case 0x54: return 2;
        case 0x55: return 5;
        case 0x56: return 2;
        case 0x57: return 5;
        case 0x58: return 2;
        case 0x59: return 5;
        case 0x5A: return 2;
        case 0x5B: return 5;
        case 0x5C: return 2;
        case 0x5D: return 5;
        case 0x5E: return 2;
        case 0x5F: return 5;
        case 0x60: return 2;
        case 0x61: return 5;
        case 0x62: return 2;
        case 0x63: return 5;

        case 0x64: return 2;
        case 0x65: return 5;
        case 0x66: return 2;
        case 0x67: return 5;
        case 0x68: return 1;
        case 0x69: return 1;

        // stack
        case 0x6A: return 2;
        case 0x6B: return 2;
        case 0x6C: return 2;
        case 0x6D: return 3;
        case 0x6E: return 2;
        case 0x6F: return 5;
        case 0x70: return 1;
        case 0x71: return 2;
        case 0x72: return 2;
        case 0x73: return 2;
        case 0x74: return 1;
        case 0x75: return 2;
        case 0x76: return 2;
        case 0x77: return 3;

        // other
        case 0x78: return 1;
        case 0x79: return 1;
        case 0x7A: return 1;
        // }}}
                   
        default: return 1;
    }
}


static void
cpu_update(Debugger* dbg)
{
    LVM_CPU* cpu = lvm_cpu(dbg->comp, 0);
    assert(cpu);

    print(dbg, 0, 0, 10, 0, "CPU");
    print(dbg, 33, 0, 10, 8, "[F?]"); print(dbg, 38, 0, 10, 0, "- choose device");
    print(dbg, 47, 2, 10, 8, "[S]"); print(dbg, 50, 2, 10, 0, "tep");

    draw_box(dbg, 0, 1, 46, 21, 10, 0, true, false);

    // registers
    for(size_t i=0; i<16; ++i) {
        print(dbg, (i%4)*13, 22+(i/4), 10, 0, "%s%s: %08X", regs[i], (i<12) ? " " : "", lvm_cpuregister(cpu, i));
    }

    // flags
    for(size_t i=0; i<6; ++i) {
        print(dbg, i+47, 19, 10, 0, "%s", flags[i]);
        print(dbg, i+47, 20, 10, 0, "%d", lvm_cpuflag(cpu, i));
    }

    // instructions
    uint32_t addr = 0;
    uint8_t y = 0;
    for(uint32_t i=dbg->cpu.code_start; ; ++i) {
        if(addr >= dbg->cpu.top_addr) {
            char buf[40];
            cpu_inst(dbg->comp, addr, buf);
            if(addr != lvm_cpuregister(cpu, PC)) {
                print(dbg, 2, y+2, 10, 0, "%08X:  %s", addr, buf);
            } else {
                print(dbg, 1, y+2, 0, 10, "%*s", 45, "");
                print(dbg, 2, y+2, 0, 10, "%08X:  %s", addr, buf);
            }
            ++y;
            if(y == 19)
                break;
        }
        addr += cpu_inst_sz(dbg->comp, addr);
    }
}


static void
cpu_advance(Debugger* debugger)
{
    debugger->cpu.top_addr += cpu_inst_sz(debugger->comp, debugger->cpu.top_addr);
    debugger->dirty = true;
}


static void
cpu_regress(Debugger* dbg)
{
    uint32_t addr = 0;
    uint32_t last_addr = 0;
    for(uint32_t i=dbg->cpu.code_start; ; ++i) {
        if(addr >= dbg->cpu.top_addr) {
            dbg->cpu.top_addr = last_addr;
            dbg->dirty = true;
            return;
        }
        last_addr = addr;
        addr += cpu_inst_sz(dbg->comp, addr);
    }
}


static void 
cpu_keypressed(Debugger* debugger, uint32_t chr)
{
    switch(chr) {
        case DOWN:
            cpu_advance(debugger);
            break;
        case UP:
            cpu_regress(debugger);
            break;
        case PGDOWN:
            for(int i=0; i<23; ++i) {
                cpu_advance(debugger);
            }
            break;
        case PGUP:
            for(int i=0; i<23; ++i) {
                cpu_regress(debugger);
            }
            break;
        case 's': {
                // step
                LVM_CPU* cpu = lvm_cpu(debugger->comp, 0);
                lvm_step(debugger->comp, 0);
                // center position
                uint32_t addr = debugger->cpu.top_addr;
                for(uint32_t y=0; y<22; ++y) {
                    addr += cpu_inst_sz(debugger->comp, addr);
                }
                uint32_t pc = lvm_cpuregister(cpu, PC);
                if(pc < debugger->cpu.top_addr || pc >= addr) {
                    debugger->cpu.top_addr = pc;
                }
                debugger->dirty = true;
            }
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
    Question* q = &debugger->question;

    if(isxdigit((char)chr) && strlen(q->buffer) < 8) {
        q->buffer[strlen(q->buffer)] = toupper(chr);
        debugger->dirty = true;
    } else if(chr == BACKSPACE && q->buffer[0]) {
        q->buffer[strlen(q->buffer)-1] = 0;
        debugger->dirty = true;
    } else if(chr == ESC) {
        debugger->state = q->return_to;
        debugger->dirty = true;
    } else if(chr == '\r') {
        q->response = (uint32_t)strtoll(q->buffer, NULL, 16);
        q->ok = true;
        debugger->state = q->return_to;
        debugger->dirty = true;
    }
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
            case ST_PHYSICAL:
                physical_update(dbg);
                break;
            case ST_CPU:
                cpu_update(dbg);
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
    switch(chr) {
        case F1:
            lvm_clrscr(debugger->comp);
            debugger->state = ST_CPU;
            debugger->dirty = true;
            break;
        case F2:
            lvm_clrscr(debugger->comp);
            debugger->state = ST_LOGICAL;
            debugger->dirty = true;
            break;
        case F3:
            lvm_clrscr(debugger->comp);
            debugger->state = ST_PHYSICAL;
            debugger->dirty = true;
            break;
        default:
            switch(debugger->state) {
                case ST_LOGICAL:
                    logical_keypressed(debugger, chr, modifiers);
                    break;
                case ST_PHYSICAL:
                    physical_keypressed(debugger, chr, modifiers);
                    break;
                case ST_CPU:
                    cpu_keypressed(debugger, chr);
                    break;
                case ST_QUESTION:
                    question_keypressed(debugger, chr);
                    break;
                default:
                    abort();
            }
    }
}

// }}}
