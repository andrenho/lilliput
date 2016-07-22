#include <stdio.h>
#include <stdint.h>

#include <lua.h>
#include <lauxlib.h>

#include "luisavm.h"

static int create_computer(lua_State* L)
{
    uint32_t sz = luaL_checkinteger(L, 1);
    LVM_Computer* comp = lvm_computercreate(sz);
    
    luaL_newlib(L, ((struct luaL_Reg[]) {
        { NULL, NULL },
    }));

    lua_pushstring(L, "__ptr");
    lua_pushlightuserdata(L, comp);
    lua_settable(L, -3);

    return 1;
}


int luaopen_luisavm(lua_State* L)
{
    luaL_newmetatable(L, "computer");
    lua_pushvalue(L, -1);

    lua_setfield(L, -2, "__index");
    luaL_newlib(L, ((struct luaL_Reg[]) {
        { "create_computer", create_computer },
        { NULL, NULL },
    }));

    return 1;
}
