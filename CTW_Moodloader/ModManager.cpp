#include "pch.h"
#include "ModManager.h"
#include "LuaManager.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <fstream>

namespace fs = std::filesystem;
#include "lua_src/lua.h"
#include "lua_src/lualib.h"
#include "lua_src/lauxlib.h"

void ModManager::Init(const std::string& modsDirectory) {
    m_ModsDirectory = modsDirectory;
    ReloadAll();
}

void ModManager::ReloadAll() {
    m_Mods.clear();
    ScanDirectory();
    TopoSort();
    
    for (const auto& mod : m_Mods) {
        if (!mod.enabled) continue;
        
        std::cout << "[ModLoader] Loading mod: " << mod.name << " (" << mod.version << ")" << std::endl;
        for (const auto& script : mod.scripts) {
            std::string fullPath = mod.path + "/" + script;
            LuaManager::GetInstance().LoadScript(fullPath);
        }
    }
}

bool ModManager::ParseManifest(const std::string& manifestPath, Mod& outMod) {
    lua_State* L = luaL_newstate();
    if (!L) return false;

    if (luaL_dofile(L, manifestPath.c_str()) != LUA_OK) {
        std::cerr << "[-] Error in " << manifestPath << ": " << lua_tostring(L, -1) << std::endl;
        lua_close(L);
        return false;
    }

    if (!lua_istable(L, -1)) {
        std::cerr << "[-] Error: " << manifestPath << " must return a table!" << std::endl;
        lua_close(L);
        return false;
    }

    lua_getfield(L, -1, "id");
    if (lua_isstring(L, -1)) outMod.id = lua_tostring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "name");
    if (lua_isstring(L, -1)) outMod.name = lua_tostring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "version");
    if (lua_isstring(L, -1)) outMod.version = lua_tostring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "author");
    if (lua_isstring(L, -1)) outMod.author = lua_tostring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "description");
    if (lua_isstring(L, -1)) outMod.description = lua_tostring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "scripts");
    if (lua_istable(L, -1)) {
        int len = lua_rawlen(L, -1);
        for (int i = 1; i <= len; i++) {
            lua_rawgeti(L, -1, i);
            if (lua_isstring(L, -1)) outMod.scripts.push_back(lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    lua_getfield(L, -1, "dependencies");
    if (lua_istable(L, -1)) {
        int len = lua_rawlen(L, -1);
        for (int i = 1; i <= len; i++) {
            lua_rawgeti(L, -1, i);
            if (lua_isstring(L, -1)) outMod.dependencies.push_back(lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    lua_close(L);
    
    return !outMod.id.empty() && !outMod.scripts.empty();
}

void ModManager::ScanDirectory() {
    if (!fs::exists(m_ModsDirectory)) {
        fs::create_directories(m_ModsDirectory);
        std::cout << "[ModLoader] Created folder " << m_ModsDirectory << std::endl;
        return;
    }

    for (const auto& entry : fs::directory_iterator(m_ModsDirectory)) {
        if (entry.is_directory()) {
            std::string manifestFile = entry.path().string() + "/manifest.lua";
            if (fs::exists(manifestFile)) {
                Mod m;
                m.path = entry.path().string();
                std::replace(m.path.begin(), m.path.end(), '\\', '/'); // normalize path
                
                if (ParseManifest(manifestFile, m)) {
                    m.enabled = !fs::exists(m.path + "/.disabled");
                    m_Mods.push_back(m);
                }
            }
        }
    }
}

void ModManager::TopoSort() {
    std::vector<Mod> sorted;
    std::vector<Mod> unsorted = m_Mods;
    bool changed = true;

    while (changed && !unsorted.empty()) {
        changed = false;
        for (auto it = unsorted.begin(); it != unsorted.end(); ) {
            bool depsMet = true;
            for (const auto& dep : it->dependencies) {
                bool found = false;
                for (const auto& s : sorted) {
                    if (s.id == dep) { found = true; break; }
                }
                if (!found) { depsMet = false; break; }
            }

            if (depsMet) {
                sorted.push_back(*it);
                it = unsorted.erase(it);
                changed = true;
            } else {
                ++it;
            }
        }
    }

    if (!unsorted.empty()) {
        std::cerr << "[-] Load Error: Cyclic dependency or missing mod found!" << std::endl;
        for (const auto& m : unsorted) {
            std::cerr << "   - Not loaded: " << m.id << std::endl;
        }
    }

    m_Mods = sorted;
}

void ModManager::ToggleMod(const std::string& modId) {
    for (auto& m : m_Mods) {
        if (m.id == modId) {
            m.enabled = !m.enabled;
            std::string disabledFile = m.path + "/.disabled";
            if (!m.enabled) {
                std::ofstream ofs(disabledFile);
                if (ofs) ofs.close();
            } else {
                if (fs::exists(disabledFile)) {
                    fs::remove(disabledFile);
                }
            }
            break;
        }
    }
}

void ModManager::DeleteMod(const std::string& modId) {
    for (auto it = m_Mods.begin(); it != m_Mods.end(); ++it) {
        if (it->id == modId) {
            try {
                fs::remove_all(it->path);
            } catch(const std::exception& e) {
                std::cerr << "[-] Error deleting mod: " << e.what() << std::endl;
            }
            m_Mods.erase(it);
            break;
        }
    }
}