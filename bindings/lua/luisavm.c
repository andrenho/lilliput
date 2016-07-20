#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>

#include "luisavm.h"

// 
// HELPER FUNCTIONS
//

// {{{

static void inspect(lua_State* L, int i)
{
    int t = lua_type(L, i);
    switch(t) {
        case LUA_TSTRING:
            printf("'%s'", lua_tostring(L, i));
            break;
        case LUA_TBOOLEAN:
            printf("%s", lua_toboolean(L, i) ? "true" : "false");
            break;
        case LUA_TNUMBER:
            printf("%g", lua_tonumber(L, i));
            break;
        case LUA_TTABLE: {
                // print metatable name
                if(lua_getmetatable(L, i) == 1) {
                    lua_getfield(L, -1, "__name");
                    if(lua_tostring(L, -1)) {
                        printf("<%s>", lua_tostring(L, -1));
                    }
                    lua_pop(L, 2);
                }

                // print table
                printf("{ ");
                lua_pushnil(L);  // first key
                while(lua_next(L, i) != 0) {
                    printf("["); inspect(L, lua_absindex(L, -2)); printf("] = "); inspect(L, lua_absindex(L, -1)); printf(", ");
                    fflush(stdout);
                    lua_pop(L, 1);
                }
                printf("}");
            }
            break;
        default:
            printf("%s", lua_typename(L, t));
            break;
    }
}


static void dump_stack(lua_State *L)
{
    int top = lua_gettop(L);
    for(int i=1; i<=top; ++i) {
        printf("  %d/%d: ", i, i-top-1);
        inspect(L, i);
        printf("\n");
    }
    printf("---------------------\n");
}

static int assert_stack(lua_State* L, int n, int extra)
{
    if(lua_gettop(L) != n + extra) {
        printf("Expected stack size to be %d, but is %d\n", n + extra, lua_gettop(L));
        dump_stack(L);
        abort();
    }
    return n;
}

void* create_object(lua_State* L, const char* metatable_name, void* ptr, const luaL_Reg* functions)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverflow"   // I don't know why am I getting this warning
    luaL_newlibtable(L, functions);
#pragma GCC diagnostic pop
    luaL_setfuncs(L, functions, 0);
    luaL_setmetatable(L, metatable_name);
    lua_pushstring(L, "__ptr");
    lua_pushlightuserdata(L, ptr);
    lua_rawset(L, -3);
    return ptr;
}

void* get_object_ptr(lua_State* L, int n)
{
    if(!lua_istable(L, n)) {
        dump_stack(L);
        luaL_error(L, "Object in stack position #%d is not a table", n);
    }
    lua_getfield(L, 1, "__ptr");
    if(!lua_islightuserdata(L, -1)) {
        dump_stack(L);
        luaL_error(L, "Object does not contain a field __ptr");
    }
    void* ptr = lua_touserdata(L, -1);
    lua_pop(L, 1);
    return ptr;
}

// }}}

//
// CPU
//

// {{{

static const char* cpu_flags[] = {
    "Y", "V", "Z", "S", "GT", "LT",
};

static int flag_get(lua_State* L)
{
    LVM_CPU* cpu = get_object_ptr(L, 1);

    if(lua_type(L, 2) == LUA_TSTRING) {
        for(size_t i=0; i < (sizeof(cpu_flags) / sizeof(cpu_flags[0])); ++i) {
            if(strcmp(lua_tostring(L, 2), cpu_flags[i]) == 0) {
                lua_pushboolean(L, lvm_cpuflag(cpu, i));
                return 1;
            }
        }
        goto regular_get;
    } else {
regular_get:
        lua_pushvalue(L, 2);
        lua_rawget(L, 1);
    }
    return 1;
}


static int flag_set(lua_State* L)
{
    LVM_CPU* cpu = get_object_ptr(L, 1);

    if(lua_type(L, 2) == LUA_TSTRING) {
        for(size_t i=0; i < (sizeof(cpu_flags) / sizeof(cpu_flags[0])); ++i) {
            if(strcmp(lua_tostring(L, 2), cpu_flags[i]) == 0) {
                lvm_cpusetflag(cpu, i, (bool)lua_toboolean(L, 3));
                return 0;
            }
        }
    } else {
regular_set:
        lua_rawset(L, 1);
    }
}


static const char* cpu_regs[] = {
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "FP", "SP", "PC", "FL"
};

static int cpu_get(lua_State* L)
{
    LVM_CPU* cpu = get_object_ptr(L, 1);

    if(lua_type(L, 2) == LUA_TSTRING) {
        for(size_t i=0; i < (sizeof(cpu_regs) / sizeof(cpu_regs[0])); ++i) {
            if(strcmp(lua_tostring(L, 2), cpu_regs[i]) == 0) {
                lua_pushinteger(L, lvm_cpuregister(cpu, i));
                return 1;
            }
        }
        goto regular_get;
    } else {
regular_get:
        lua_pushvalue(L, 2);
        lua_rawget(L, 1);
    }
    return 1;
}


static int cpu_set(lua_State* L)
{
    LVM_CPU* cpu = get_object_ptr(L, 1);

    if(lua_type(L, 2) == LUA_TSTRING) {
        for(size_t i=0; i < (sizeof(cpu_regs) / sizeof(cpu_regs[0])); ++i) {
            if(strcmp(lua_tostring(L, 2), cpu_regs[i]) == 0) {
                lvm_cpusetregister(cpu, i, (uint32_t)luaL_checkinteger(L, 3));
                return 0;
            }
        }
        goto regular_set;
    } else {
regular_set:
        lua_rawset(L, 1);
    }
}



static void create_cpu_metatable(lua_State* L)
{
    luaL_newmetatable(L, "LVM_CPU");
    lua_pushcfunction(L, cpu_get);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, cpu_set);
    lua_setfield(L, -2, "__newindex");
    lua_pop(L, 1);

    luaL_newmetatable(L, "LVM_Flag");
    lua_pushcfunction(L, flag_get);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, flag_set);
    lua_setfield(L, -2, "__newindex");
    lua_pop(L, 1);
}


static int cpu_add_breakpoint(lua_State* L)
{
    lvm_addbreakpoint(get_object_ptr(L, 1), luaL_checknumber(L, 2));
    return 0;
}


static int cpu_remove_breakpoint(lua_State* L)
{
    lvm_removebreakpoint(get_object_ptr(L, 1), luaL_checknumber(L, 2));
    return 0;
}


static int cpu_is_breakpoint(lua_State* L)
{
    lua_pushboolean(L, lvm_isbreakpoint(get_object_ptr(L, 1), luaL_checknumber(L, 2)));
    return 1;
}


static int computer_addcpu(lua_State* L)
{
    LVM_Computer* comp = get_object_ptr(L, 1);
    LVM_CPU* cpu = lvm_addcpu(comp);
    
    lua_getfield(L, 1, "cpu");
    create_object(L, "LVM_CPU", cpu, (struct luaL_Reg[]) {
        { "add_breakpoint",     cpu_add_breakpoint },
        { "remove_breakpoint",  cpu_remove_breakpoint },
        { "is_breakpoint",      cpu_is_breakpoint },
        { NULL, NULL }
    });

    // add flags table to CPU
    lua_newtable(L);
    lua_pushlightuserdata(L, cpu);
    lua_setfield(L, -2, "__ptr");
    luaL_setmetatable(L, "LVM_Flag");
    lua_setfield(L, -2, "flags");

    // add CPU to computer
    lua_seti(L, -2, luaL_len(L, 1)+1);
    lua_pop(L, 1);

    return 0;
}

// }}}

// 
// PHYSICAL MEMORY
//

// {{{

static int ph_get(lua_State* L)
{
    lua_getfield(L, 1, "__ptr");
    uint8_t* data = lua_touserdata(L, -1);
    size_t pos = luaL_checkinteger(L, 2);

    lua_getfield(L, 1, "__sz");
    uint32_t sz = lua_tointeger(L, -1);
    lua_pop(L, 1);
    if(pos >= sz) {
        luaL_error(L, "Trying to load physical memory position %d, when memory size is %d", pos, sz);
    }
    
    lua_pushinteger(L, data[pos]);
    return 1;
}


static int ph_set(lua_State* L)
{
    lua_getfield(L, 1, "__ptr");
    uint8_t* data = lua_touserdata(L, -1);
    size_t pos = luaL_checkinteger(L, 2);
    int fvalue = luaL_checkinteger(L, 3);

    if(fvalue < 0x0 || fvalue > 0xFF) {
        luaL_error(L, "Value must be between 0 and 255 (%d given)", fvalue);
    }

    lua_getfield(L, 1, "__sz");
    uint32_t sz = lua_tointeger(L, -1);
    lua_pop(L, 1);
    if(pos >= sz) {
        luaL_error(L, "Trying to load physical memory position %d, when memory size is %d", pos, sz);
    }
    
    data[pos] = (uint8_t)fvalue;
    return 0;
}


static void create_physical_memory_metatable(lua_State* L)
{
    luaL_newmetatable(L, "LVM_PhysicalMemory");
    lua_pushcfunction(L, ph_get);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, ph_set);
    lua_setfield(L, -2, "__newindex");
    lua_pop(L, 1);
}

// }}}

// 
// OFFSET
//

// {{{

static int offset_get(lua_State* L)
{
    if(lua_type(L, 2) == LUA_TSTRING && strcmp(lua_tostring(L, 2), "offset") == 0) {
        lua_pushinteger(L, lvm_offset(get_object_ptr(L, 1)));
    } else {
        lua_pushvalue(L, 2);
        lua_rawget(L, 1);
    }
    return 1;
}


static int offset_set(lua_State* L)
{
    if(lua_type(L, 2) == LUA_TSTRING && strcmp(lua_tostring(L, 2), "offset") == 0) {
        lvm_setoffset(get_object_ptr(L, 1), luaL_checkinteger(L, 3));
    } else {
        lua_rawset(L, 1);
    }
}


static void create_offset_metatable(lua_State* L)
{
    luaL_getmetatable(L, "LVM_Computer");
    lua_pushcfunction(L, offset_get);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, offset_set);
    lua_setfield(L, -2, "__newindex");
    lua_pop(L, 1);
}

// }}}

// 
// COMPUTER
//

// {{{

#define GETTER(name) \
static int computer_ ## name(lua_State* L) \
{ \
    uint32_t pos = luaL_checkinteger(L, 2); \
    lua_pushinteger(L, lvm_ ## name((LVM_Computer*)get_object_ptr(L, 1), pos)); \
    return 1; \
}
GETTER(get)
GETTER(get16)
GETTER(get32)
#undef GETTER

#define SETTER(name, type) \
static int computer_ ## name(lua_State* L) \
{ \
    uint32_t pos = luaL_checkinteger(L, 2); \
    type data = luaL_checkinteger(L, 3); \
    lvm_ ## name((LVM_Computer*)get_object_ptr(L, 1), pos, data); \
    return 0; \
}
SETTER(set, uint8_t)
SETTER(set16, uint16_t)
SETTER(set32, uint32_t)
#undef SETTER


static int computer_step(lua_State* L)
{
    if(lua_gettop(L) == 1) {
        lvm_step(get_object_ptr(L, 1), 0);
    } else {
        lvm_step(get_object_ptr(L, 1), luaL_checknumber(L, 2));
    }
    return 0;
}


static int computer_reset(lua_State* L)
{
    lvm_reset(get_object_ptr(L, 1));
    return 0;
}


static int destroy_computer(lua_State* L)
{
    lvm_computerdestroy(get_object_ptr(L, 1));
    return 0;
}


static int computer_loadrom(lua_State* L)
{
    lua_pushboolean(L, lvm_loadromfile(get_object_ptr(L, 1), luaL_checkstring(L, 2)));
    return 1;
}


static int create_computer(lua_State* L)
{
    int n_args = lua_gettop(L);

    uint32_t sz = luaL_checkinteger(L, 1);
    LVM_Computer* comp = lvm_computercreate(sz);
    create_object(L, "LVM_Computer", comp, (struct luaL_Reg[]) {
        { "get",      computer_get     },
        { "get16",    computer_get16   },
        { "get32",    computer_get32   },
        { "set",      computer_set     },
        { "set16",    computer_set16   },
        { "set32",    computer_set32   },
        { "add_cpu",  computer_addcpu  },
        { "step",     computer_step    },
        { "reset",    computer_reset   },
        { "load_rom", computer_loadrom }, 
        { NULL, NULL },
    });

    // add physical memory
    lua_newtable(L);
    lua_pushlightuserdata(L, lvm_physicalmemory(comp));
    lua_setfield(L, -2, "__ptr");
    lua_pushinteger(L, lvm_physicalmemorysz(comp));
    lua_setfield(L, -2, "__sz");
    luaL_setmetatable(L, "LVM_PhysicalMemory");
    lua_setfield(L, -2, "physical_memory");

    // cpus
    lua_newtable(L);
    lua_setfield(L, -2, "cpu");

    return assert_stack(L, 1, n_args);
}

// }}}

// 
// LUISAVM
//

// {{{

static int debug_log(lua_State* L)
{
    lvm_debuglog(lua_toboolean(L, 1));
    return 0;
}

#define ADD_GC(metatable, destructor) {         \
    luaL_newmetatable(L, metatable);            \
    lua_pushcfunction(L, destructor);           \
    lua_setfield(L, -2, "__gc");                \
    lua_pop(L, 1);                              \
}


int luaopen_luisavm(lua_State* L)
{
    lua_pop(L, 2);

    // Create GCs - TODO - put this in an independent function
    ADD_GC("LVM_Computer", destroy_computer);
    create_physical_memory_metatable(L);
    create_offset_metatable(L);
    create_cpu_metatable(L);

    // create library
    luaL_newlib(L, ((struct luaL_Reg[]) {
        { "create_computer", create_computer },
        { "debug_log", debug_log },
        { NULL, NULL },
    }));

    return assert_stack(L, 1, 0);
}

// }}}
