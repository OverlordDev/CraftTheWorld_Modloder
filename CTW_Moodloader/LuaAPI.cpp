#include "pch.h"
#include "LuaAPI.h"
#include "LuaManager.h"
#include "GameAPI.h"
#include <iostream>
#include "Offsets.h"

#include "lua_src/lua.h"
#include "lua_src/lauxlib.h"

// CTW.spawnCreature(name, x, y)
static int l_spawnCreature(lua_State* L) {
    if (lua_gettop(L) < 3) return luaL_error(L, "CTW.spawnCreature requires 3 arguments");
    const char* name = luaL_checkstring(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    
    uintptr_t base = (uintptr_t)GetModuleHandle(NULL);
    bool boss = (strcmp(name, "zombie_boss") == 0);
    if (strcmp(name, "sheep") == 0) {
        SpawnSheep(base, x, y);
    } else {
        SpawnZombie(base, x, y, boss);
    }
    return 0;
}

// CTW.giveResource(id, amount)
static int l_giveResource(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    int amount = luaL_checkinteger(L, 2);
    uintptr_t base = (uintptr_t)GetModuleHandle(NULL);
    void* pWorld = *(void**)(base + Offsets::offsetWorldInstance);
    GiveResource(base, pWorld, id, amount);
    return 0;
}

// CTW.getResourceCount(id)
static int l_getResourceCount(lua_State* L) {
    int id = luaL_checkinteger(L, 1);
    uintptr_t base = (uintptr_t)GetModuleHandle(NULL);
    void* pWorld = *(void**)(base + Offsets::offsetWorldInstance);
    int count = FGetResourceCount(base, pWorld, id);
    lua_pushinteger(L, count);
    return 1;
}

// CTW.addExp(amount)
static int l_addExp(lua_State* L) {
    int amount = luaL_checkinteger(L, 1);
    AddExp((uintptr_t)GetModuleHandle(NULL), amount);
    return 0;
}

// CTW.setDay()
static int l_setDay(lua_State* L) {
    uintptr_t base = (uintptr_t)GetModuleHandle(NULL);
    void* pWorld = *(void**)(base + Offsets::offsetWorldInstance);
    TriggerCallDay(base, pWorld);
    return 0;
}

// CTW.setNight()
static int l_setNight(lua_State* L) {
    uintptr_t base = (uintptr_t)GetModuleHandle(NULL);
    void* pWorld = *(void**)(base + Offsets::offsetWorldInstance);
    TriggerCalNight(base, pWorld);
    return 0;
}

// CTW.buildBlock(x, y, blockId, isFront)
static int l_buildBlock(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int blockId = luaL_checkinteger(L, 3);
    bool isFront = lua_toboolean(L, 4);
    BuildBlockAt(x, y, blockId, isFront);
    return 0;
}

// CTW.dropItem(id, count, x, y)
static int l_dropItem(lua_State* L) {
    if (lua_gettop(L) < 4) return luaL_error(L, "CTW.dropItem requires 4 arguments: id, count, x, y");
    int id = luaL_checkinteger(L, 1);
    int count = luaL_checkinteger(L, 2);
    float x = (float)luaL_checknumber(L, 3);
    float y = (float)luaL_checknumber(L, 4);

    DropItem(id, count, x, y);
    return 0;
}

// CTW.log(message)

// CTW.setTimeout(seconds, callback)
static int l_setTimeout(lua_State* L) {
    float seconds = (float)luaL_checknumber(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    
    lua_pushvalue(L, 2);
    int refId = luaL_ref(L, LUA_REGISTRYINDEX);
    
    int timerId = LuaManager::GetInstance().AddTimer(refId, seconds, false);
    lua_pushinteger(L, timerId);
    return 1;
}

// CTW.setInterval(seconds, callback)
static int l_setInterval(lua_State* L) {
    float seconds = (float)luaL_checknumber(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    
    lua_pushvalue(L, 2);
    int refId = luaL_ref(L, LUA_REGISTRYINDEX);
    
    int timerId = LuaManager::GetInstance().AddTimer(refId, seconds, true);
    lua_pushinteger(L, timerId);
    return 1;
}

void RegisterLuaAPI(lua_State* L) {
    lua_newtable(L);

    lua_pushcfunction(L, l_spawnCreature); lua_setfield(L, -2, "spawnCreature");
    lua_pushcfunction(L, l_giveResource); lua_setfield(L, -2, "giveResource");
    lua_pushcfunction(L, l_getResourceCount); lua_setfield(L, -2, "getResourceCount");
    lua_pushcfunction(L, l_addExp); lua_setfield(L, -2, "addExp");
    lua_pushcfunction(L, l_setDay); lua_setfield(L, -2, "setDay");
    lua_pushcfunction(L, l_setNight); lua_setfield(L, -2, "setNight");
    lua_pushcfunction(L, l_buildBlock); lua_setfield(L, -2, "buildBlock");
    lua_pushcfunction(L, l_dropItem); lua_setfield(L, -2, "dropItem");
    lua_pushcfunction(L, l_log); lua_setfield(L, -2, "log");
    
    lua_pushcfunction(L, l_setTimeout); lua_setfield(L, -2, "setTimeout");
    lua_pushcfunction(L, l_setInterval); lua_setfield(L, -2, "setInterval");

    // function CTW._triggerEvent(name, arg) ... end
    
    lua_setglobal(L, "CTW");
    
    const char* bootScript = 
        "CTW._events = {}\n"
        "function CTW.onEvent(name, cb)\n"
        "    if not CTW._events[name] then CTW._events[name] = {} end\n"
        "    table.insert(CTW._events[name], cb)\n"
        "end\n"
        "function CTW._triggerEvent(name, arg)\n"
        "    if CTW._events[name] then\n"
        "        for i, cb in ipairs(CTW._events[name]) do cb(arg) end\n"
        "    end\n"
        "end\n"
        "function CTW.onDayStart(cb) CTW.onEvent('DayStart', cb) end\n"
        "function CTW.onNightStart(cb) CTW.onEvent('NightStart', cb) end\n"
        "function CTW.onCreatureSpawn(cb) CTW.onEvent('OnCreatureSpawn', cb) end\n"
        "function CTW.onBlockCrashed(cb) CTW.onEvent('OnBlockCrashed', cb) end\n";
        
    luaL_dostring(L, bootScript);
}