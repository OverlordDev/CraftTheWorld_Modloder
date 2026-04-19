#include "pch.h"
#include "LuaManager.h"
#include <Windows.h>
#include "Offsets.h"
#include "ModManager.h"
#include "LuaAPI.h"
#include <iostream>

#include "lua_src/lua.h"
#include "lua_src/lualib.h"
#include "lua_src/lauxlib.h"

bool LuaManager::Init() {
    if (L) Shutdown();

    L = luaL_newstate();
    if (!L) {
        std::cerr << "[-] Error initializing Lua state!" << std::endl;
        return false;
    }

    luaL_openlibs(L);
    
    RegisterLuaAPI(L);

    return true;
}

void LuaManager::Shutdown() {
    if (L) {
        lua_close(L);
        L = nullptr;
    }
    m_Timers.clear();
}

void LuaManager::ReloadAll() {
    std::cout << "[Lua] Reloading all scripts (F6)..." << std::endl;
    Shutdown();
    Init();
    
    ModManager::GetInstance().ReloadAll();
}

void LuaManager::LoadScript(const std::string& path) {
    if (!L) return;
    
    if (luaL_dofile(L, path.c_str()) != LUA_OK) {
        std::cerr << "[-] Error loading script " << path << ": " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
    } else {
        std::cout << "[+] Script " << path << " successfully loaded." << std::endl;
    }
}

int LuaManager::AddTimer(int refId, float seconds, bool repeat) {
    LuaTimer t;
    t.callbackRef = refId;
    t.timeRemaining = seconds;
    t.interval = seconds;
    t.repeat = repeat;
    m_Timers.push_back(t);
    return m_Timers.size() - 1;
}

void LuaManager::OnUpdate(float dt) {
    if (!L) return;

    static bool wasWorldLoaded = false;
    uintptr_t base = (uintptr_t)GetModuleHandle(NULL);
    void* world = *(void**)(base + Offsets::offsetWorldInstance);
    
    if (world != nullptr && !wasWorldLoaded) {
        wasWorldLoaded = true;
        FireEvent("OnWorldLoaded");
        std::cout << "[LuaManager] Event OnWorldLoaded() triggered" << std::endl;
    } else if (world == nullptr && wasWorldLoaded) {
        wasWorldLoaded = false;
        FireEvent("OnWorldClosed");
    }

    for (auto it = m_Timers.begin(); it != m_Timers.end(); ) {
        it->timeRemaining -= dt;
        if (it->timeRemaining <= 0.0f) {
            lua_rawgeti(L, LUA_REGISTRYINDEX, it->callbackRef);
            if (lua_isfunction(L, -1)) {
                if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
                    std::cerr << "[-] Error in Timer Callback: " << lua_tostring(L, -1) << std::endl;
                    lua_pop(L, 1);
                }
            } else {
                lua_pop(L, 1); // pop nil
            }

            if (it->repeat) {
                it->timeRemaining = it->interval;
                ++it;
            } else {
                luaL_unref(L, LUA_REGISTRYINDEX, it->callbackRef);
                it = m_Timers.erase(it);
            }
        } else {
            ++it;
        }
    }
}

void LuaManager::FireEvent(const std::string& eventName) {
    if (!L) return;
    
    lua_getglobal(L, eventName.c_str());
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            std::cerr << "[-] Lua Event Error: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1); 
    }
}

void LuaManager::FireEventString(const std::string& eventName, const std::string& arg) {
    if (!L) return;
    
    lua_getglobal(L, eventName.c_str());
    if (lua_isfunction(L, -1)) {
        lua_pushstring(L, arg.c_str());
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            std::cerr << "[-] Lua Event Error: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1);
    }
}

void LuaManager::TriggerCTWEvent(const std::string& eventName, const std::string& arg) {
    if (!L) return;

    // CTW._triggerEvent(eventName, arg)
    lua_getglobal(L, "CTW");
    if (!lua_istable(L, -1)) { lua_pop(L, 1); return; }

    lua_getfield(L, -1, "_triggerEvent");
    if (!lua_isfunction(L, -1)) { lua_pop(L, 2); return; }

    lua_pushstring(L, eventName.c_str());
    lua_pushstring(L, arg.c_str());

    if (lua_pcall(L, 2, 0, 0) != LUA_OK) {
        std::cerr << "[-] TriggerCTWEvent '" << eventName << "' error: "
                  << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
    }
    lua_pop(L, 1); // pop CTW table
}

void LuaManager::TriggerBlockEvent(int x, int y, int id) {
    if (!L) return;

    lua_getglobal(L, "CTW");
    if (!lua_istable(L, -1)) { lua_pop(L, 1); return; }

    lua_getfield(L, -1, "_triggerEvent");
    if (!lua_isfunction(L, -1)) { lua_pop(L, 2); return; }

    lua_pushstring(L, "OnBlockCrashed");
    
    // Создаем таблицу {x=x, y=y, id=id}
    lua_newtable(L);
    lua_pushinteger(L, x);
    lua_setfield(L, -2, "x");
    lua_pushinteger(L, y);
    lua_setfield(L, -2, "y");
    lua_pushinteger(L, id);
    lua_setfield(L, -2, "id");

    if (lua_pcall(L, 2, 0, 0) != LUA_OK) {
        std::cerr << "[-] Error in OnBlockCrashed event: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
    }
    lua_pop(L, 1); // pop CTW
}