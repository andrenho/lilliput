#include "luisavm.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "device.h"
#include "debugger.h"

#define KEYBOARD_POSITION 0xF0000000
#define VIDEO_POSITION    0xF8000000

typedef struct Video Video;

extern LVM_CPU* lvm_createcpu(LVM_Computer* comp);
extern void     lvm_destroycpu(LVM_CPU* cpu);
extern void     lvm_cpustep(LVM_CPU* cpu);
extern void     lvm_cpureset(LVM_CPU* cpu);
extern Video*   video_init(VideoCallbacks cbs);

typedef struct LVM_Computer {
    uint8_t*  physical_memory;
    uint32_t  physical_memory_size;
    uint32_t  offset;
    Device**  device;
    LVM_CPU** cpu;
    Video*    video;
    Debugger* debugger;
    struct timespec last_step;
} LVM_Computer;

// {{{ COMPUTER MANAGEMENT

LVM_Computer*
lvm_computercreate(uint32_t physical_memory_size, bool debugger_active)
{
    LVM_Computer* comp = calloc(1, sizeof(LVM_Computer));
    comp->physical_memory = calloc(physical_memory_size, 1);
    comp->physical_memory_size = physical_memory_size;
    comp->device = calloc(1, sizeof(Device*));
    comp->cpu = calloc(1, sizeof(LVM_CPU*));
    comp->debugger = debugger_init(comp, debugger_active);
    clock_gettime(CLOCK_MONOTONIC, &comp->last_step);

    syslog(LOG_DEBUG, "Computer created with %d kB of physical memory.", physical_memory_size / 1024);
    return comp;
}


void
lvm_computerdestroy(LVM_Computer* comp)
{
    size_t i = 0;
    debugger_free(comp->debugger);
    while(comp->device[i]) {
        if(comp->device[i]->free) {
            comp->device[i]->free(comp->device[i]);
        }
        ++i;
    }
    free(comp->device);
    i = 0;
    while(comp->cpu[i]) {
        lvm_destroycpu(comp->cpu[i++]);
    }
    free(comp->cpu);
    free(comp->physical_memory);
    free(comp);
    syslog(LOG_DEBUG, "Computer destroyed.");
}

// }}}

// {{{ STEP/RESET

void
lvm_step(LVM_Computer* comp, size_t force_time_us)
{
    // calculate time since last frame
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
    ssize_t time_us = (size_t)force_time_us;
#pragma GCC diagnostic pop
    if(force_time_us > 0) {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        time_us = ((now.tv_sec - comp->last_step.tv_sec) * 1000000) +   // seconds
                  ((now.tv_nsec - comp->last_step.tv_nsec) / 1000);     // nanoseconds
        comp->last_step = now;
    } else {
        // advance last frame manually
        const long BILLION = 1000000000;
        long sec = comp->last_step.tv_sec;
        long nsec = comp->last_step.tv_nsec + (time_us * 1000);
        if(nsec >= BILLION) {
            nsec -= BILLION;
            ++sec;
        }
        comp->last_step = (struct timespec) { .tv_sec = sec, .tv_nsec = nsec };
    }

    // step devices
    size_t i = 0;
    while(comp->cpu[i]) {
        lvm_cpustep(comp->cpu[i++]);
    }
    
    for(size_t i=0; comp->device[i]; ++i) {
        if(comp->device[i]->step) {
            comp->device[i]->step(comp->device[i], (size_t)time_us);
        }
    }
}


void
lvm_reset(LVM_Computer* comp)
{
    comp->offset = 0;
    for(uint32_t pos = 0; pos < comp->physical_memory_size; ++pos) {
        comp->physical_memory[pos] = 0;
    }

    size_t i = 0;
    while(comp->cpu[i]) {
        lvm_cpureset(comp->cpu[i++]);
    }
}

// }}}

// {{{ MEMORY MANAGEMENT

uint8_t 
lvm_get(LVM_Computer* comp, uint32_t pos)
{
    pos += comp->offset;
    if(pos < comp->physical_memory_size) {
        return comp->physical_memory[pos];
    } else if(pos >= 0xEF000000) {
        for(size_t i=0; comp->device[i]; ++i) {
            uint32_t sz = comp->device[i]->sz;
            if(pos >= comp->device[i]->pos && pos < (comp->device[i]->pos + sz)) {
                if(comp->device[i]->get) {
                    return comp->device[i]->get(comp->device[i], pos - comp->device[i]->pos);
                }
                return 0;
            }
        }
        return 0;
    } else {
        return 0;
    }
}


void 
lvm_set(LVM_Computer* comp, uint32_t pos, uint8_t data)
{
    pos += comp->offset;
    if(pos < comp->physical_memory_size) {
        comp->physical_memory[pos] = data;
    } else if(pos >= 0xEF000000) {
        for(size_t i=0; comp->device[i]; ++i) {
            uint32_t sz = comp->device[i]->sz;
            if(pos >= comp->device[i]->pos && pos < (comp->device[i]->pos + sz)) {
                if(comp->device[i]->set) {
                    comp->device[i]->set(comp->device[i], pos - comp->device[i]->pos, data);
                    return;
                }
            }
        }
    }
}


uint16_t 
lvm_get16(LVM_Computer* comp, uint32_t pos)
{
    uint16_t b1 = lvm_get(comp, pos),
             b2 = lvm_get(comp, pos+1);
    return b1 | (uint16_t)(b2 << 8);
}


void
lvm_set16(LVM_Computer* comp, uint32_t pos, uint16_t data)
{
    lvm_set(comp, pos, (uint8_t)data);
    lvm_set(comp, pos+1, (uint8_t)(data >> 8));
}


uint32_t 
lvm_get32(LVM_Computer* comp, uint32_t pos)
{
    uint32_t b1 = lvm_get(comp, pos),
             b2 = lvm_get(comp, pos+1),
             b3 = lvm_get(comp, pos+2),
             b4 = lvm_get(comp, pos+3);
    return b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
}


void
lvm_set32(LVM_Computer* comp, uint32_t pos, uint32_t data)
{
    lvm_set(comp, pos, (uint8_t)data);
    lvm_set(comp, pos+1, (uint8_t)(data >> 8));
    lvm_set(comp, pos+2, (uint8_t)(data >> 16));
    lvm_set(comp, pos+3, (uint8_t)(data >> 24));
}


uint8_t*
lvm_physicalmemory(LVM_Computer* comp)
{
    return comp->physical_memory;
}


uint32_t
lvm_physicalmemorysz(LVM_Computer* comp)
{
    return comp->physical_memory_size;
}


void
lvm_setoffset(LVM_Computer* comp, uint32_t offset)
{
    if(offset >= 0xF0000000) {
        syslog(LOG_ERR, "Offset can't be higher than 0xF0000000");
        abort();
    }
    comp->offset = offset;
}


uint32_t
lvm_offset(LVM_Computer* comp)
{
    return comp->offset;
}

// }}}

// {{{ DEVICE MANAGEMENT

static void
lvm_adddevice(LVM_Computer* comp, Device* dev, uint32_t pos)
{
    dev->pos = pos;

    size_t i = 0;
    while(comp->device[i++]);
    
    // create space for one extra device
    comp->device = realloc(comp->device, sizeof(Device*) * (i+1));
    comp->device[i] = NULL;

    // add device
    comp->device[i-1] = dev;
    syslog(LOG_DEBUG, "New device added to computer.");
}

// }}}

// {{{ CPU MANAGEMENT

LVM_CPU* 
lvm_addcpu(LVM_Computer* comp)
{
    size_t i = 0;
    while(comp->cpu[i++]);
    
    // create space for one extra CPU
    comp->cpu = realloc(comp->cpu, sizeof(LVM_CPU*) * (i+1));
    comp->cpu[i] = NULL;

    // add CPU
    comp->cpu[i-1] = lvm_createcpu(comp);
    syslog(LOG_DEBUG, "New CPU added to computer.");
    return comp->cpu[i-1];
}


LVM_CPU* 
lvm_cpu(LVM_Computer* comp, size_t n)
{
    return comp->cpu[n];
}

// }}}

// {{{ ROM

void
lvm_loadrom(LVM_Computer* comp, uint32_t sz, uint8_t* data)
{
    if(sz > lvm_physicalmemorysz(comp)) {
        syslog(LOG_ERR, "Memory is too small to accomodate such a large ROM.");
        exit(EXIT_FAILURE);
    }

    memcpy(comp->physical_memory, data, sz);
}


bool
lvm_loadromfile(LVM_Computer* comp, const char* filename)
{
    // open file
    FILE* f = fopen(filename, "r");
    if(!f) {
        syslog(LOG_ERR, "Error loading ROM file '%s': %s", filename, strerror(errno));
        return false;
    }

    // get file size
    fseek(f, 0L, SEEK_END);
    long int sz = ftell(f);
    rewind(f);

    // load file
    uint8_t* data = malloc((size_t)sz);
    if(fread(data, (size_t)sz, 1, f) == 0) {
        syslog(LOG_ERR, "Error loading ROM file '%s': %s", filename, strerror(errno));
        return false;
    }

    lvm_loadrom(comp, (uint32_t)sz, data);

    fclose(f);
    return true;
}

// }}}

// {{{ VIDEO

void lvm_setupvideo(LVM_Computer* comp, VideoCallbacks cbs)
{
    Video* video = video_init(cbs);
    comp->video = video;
    lvm_adddevice(comp, (Device*)video, VIDEO_POSITION);
}

void lvm_draw_char(LVM_Computer* comp, uint8_t c, uint16_t x, uint16_t y, uint8_t fg, uint8_t bg)
{
    extern void video_draw_char(Video* video, uint8_t c, uint16_t x, uint16_t y, uint8_t fg, uint8_t bg);
    if(comp->video) {
        video_draw_char(comp->video, c, x, y, fg, bg);
    }
}

void lvm_clrscr(LVM_Computer* comp)
{
    extern void video_clrscr(Video* video);
    video_clrscr(comp->video);
}

// }}}

// {{{ DEBUGGER

bool
lvm_debuggeractive(LVM_Computer* computer)
{
    return debugger_active(computer->debugger);
}


void
lvm_debuggerupdate(LVM_Computer* computer)
{
    debugger_update(computer->debugger);
}


void
lvm_debuggerloadmap(LVM_Computer* computer, const char* filename)
{
    debugger_loadmap(computer->debugger, filename);
}

// }}}

// {{{ KEYBOARD

void
lvm_keypressed(LVM_Computer* comp, uint32_t chr, uint8_t modifiers)
{
    if(lvm_debuggeractive(comp)) {
        debugger_keypressed(comp->debugger, chr, modifiers);
    } else {
        abort();  // TODO - not implemented
    }
}


void
lvm_keyreleased(LVM_Computer* comp, uint32_t chr, uint8_t modifiers)
{
    (void) comp;
    (void) chr;
    (void) modifiers;

    if(!lvm_debuggeractive(comp)) {
        abort();  // TODO - not implemented
    }
}

// }}}
