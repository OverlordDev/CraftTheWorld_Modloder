#include "pch.h"
#include "DataAPI.h"
#include "ModStructures.h"
#include "Offsets.h"
#include "GameAPI.h" // Added GameAPI for SetPropertyValue
#include "lua_src/lua.h"
#include "lua_src/lauxlib.h"
#include <windows.h>
#include <iostream>

World* GetWorld() {
    __try {
        uintptr_t base = (uintptr_t)GetModuleHandle(NULL);
        if (!base) return nullptr;
        return *(World**)(base + Offsets::offsetWorldInstance);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
}

void lua_push_BaseString(lua_State* L, const BaseString& s) {
    __try {
        const char* str = s.c_str();
        // Check if string pointer is valid
        if (str && !IsBadReadPtr(str, 1)) {
            lua_pushstring(L, str);
        } else {
            lua_pushstring(L, "");
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        lua_pushstring(L, "<err>");
    }
}

static int l_getCraftResources(lua_State* L) {
    lua_newtable(L);
    __try {
        World* world = GetWorld();
        if (!world || !world->m_isGame) return 1;

        auto& res = world->m_craftResources;
        size_t count = res.size();
        if (count > 5000 || !res.first) return 1;

        for (size_t i = 0; i < count; ++i) {
            lua_pushinteger(L, (int)i + 1);
            lua_newtable(L);
            
            lua_push_BaseString(L, res[i].Name.m_data);
            lua_setfield(L, -2, "name");

            lua_push_BaseString(L, res[i].Title);
            lua_setfield(L, -2, "title");

            lua_push_BaseString(L, res[i].Description);
            lua_setfield(L, -2, "desc");

            lua_push_BaseString(L, res[i].FileName);
            lua_setfield(L, -2, "file");

            lua_pushinteger(L, (int)i);
            lua_setfield(L, -2, "id");
            
            lua_pushinteger(L, res[i].Order);
            lua_setfield(L, -2, "order");

            lua_settable(L, -3);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return 1;
}

static int l_getBlockTypes(lua_State* L) {
    lua_newtable(L);
    __try {
        World* world = GetWorld();
        if (!world || !world->m_isGame) return 1;

        auto& blocks = world->m_blockTypes;
        size_t count = blocks.size();
        if (count > 2000 || !blocks.first) return 1;

        for (size_t i = 0; i < count; ++i) {
            if (!blocks[i]) continue;
            
            lua_pushinteger(L, (int)i + 1);
            lua_newtable(L);

            lua_push_BaseString(L, blocks[i]->Name.m_data);
            lua_setfield(L, -2, "name");

            lua_pushinteger(L, (int)i);
            lua_setfield(L, -2, "id");

            lua_pushnumber(L, (double)blocks[i]->Extract);
            lua_setfield(L, -2, "extract");

            lua_pushinteger(L, blocks[i]->Build);
            lua_setfield(L, -2, "build");

            lua_pushinteger(L, blocks[i]->Application);
            lua_setfield(L, -2, "application");

            lua_settable(L, -3);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return 1;
}

static int l_getRecipes(lua_State* L) {
    lua_newtable(L);
    __try {
        World* world = GetWorld();
        if (!world || !world->m_isGame) return 1;

        auto& recipes = world->m_recipes;
        size_t count = recipes.size();
        if (count > 5000 || !recipes.first) return 1;

        for (size_t i = 0; i < count; ++i) {
            lua_pushinteger(L, (int)i + 1);
            lua_newtable(L);

            lua_pushinteger(L, recipes[i].Id);
            lua_setfield(L, -2, "id");

            lua_push_BaseString(L, recipes[i].Name.m_data);
            lua_setfield(L, -2, "name");

            lua_pushboolean(L, recipes[i].Allow);
            lua_setfield(L, -2, "allow");

            lua_settable(L, -3);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return 1;
}

static int l_setRecipeAllow(lua_State* L) {
    int id = (int)luaL_checkinteger(L, 1);
    bool allow = lua_toboolean(L, 2);

    __try {
        World* world = GetWorld();
        if (!world || !world->m_isGame) return 0;

        auto& recipes = world->m_recipes;
        if (id >= 1 && id <= (int)recipes.size()) {
            Recipe& r = recipes[id - 1];
            r.Allow = allow;
            r.AllowOrig = allow;
            r.Enabled = allow;
            r.ReadyForEnable = allow;
            r.CraftTreeEnabled = allow;

            // Sync corresponding CraftResource Allow flag
            auto& resources = world->m_craftResources;
            for (size_t i = 0; i < resources.size(); ++i) {
                if (resources[i].Id == r.ProduceId) {
                    resources[i].Allow = allow;
                    break;
                }
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return 0;
}

static int l_syncRecipeUI(lua_State* L) {
    __try {
        Game* game = Game::GetInstance();
        if (!game) return 0;

        // Dynamic search for m_invDialog in Game structure
        // We know Game size is 0x2B8. Dialogs are Refs (8 bytes: ptr, refcount_ptr)
        // m_invDialog is likely the 36th Ref or so (from constructor count)
        // We'll scan the Game memory for a pointer that looks like an InventoryDialog
        
        uintptr_t* pGameRaw = (uintptr_t*)game;
        for (int i = 0; i < 0x2B8 / 4; ++i) {
            void* potentialDialog = (void*)pGameRaw[i];
            if (potentialDialog && !IsBadReadPtr(potentialDialog, 4)) {
                // Check if it has a vftable pointing to our Refresh address proximity
                uintptr_t vtable = *(uintptr_t*)potentialDialog;
                if (vtable > (uintptr_t)GetModuleHandle(NULL) + 0x800000 && 
                    vtable < (uintptr_t)GetModuleHandle(NULL) + 0xD00000) {
                    
                    // Simple heuristic: InventoryDialog is large and has specific fields
                    // For now, we'll try Refreshing if it looks like a dialog
                    InventoryDialog* dialog = (InventoryDialog*)potentialDialog;
                    dialog->Refresh();
                    // break; // Might be multiple dialogs, but we usually only care about the active one
                }
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return 0;
}

// -----------------------------------------------------------------------------
// CTW.setPropertyValue(type, id, propEnum, ruleEnum, value)
// type: "item" (CraftResource) or "block" (BlockType)
// -----------------------------------------------------------------------------
static int l_setPropertyValue(lua_State* L) {
    const char* type = luaL_checkstring(L, 1);
    int id = (int)luaL_checkinteger(L, 2);
    int propEnum = (int)luaL_checkinteger(L, 3);
    int ruleEnum = (int)luaL_checkinteger(L, 4);
    float value = (float)luaL_checknumber(L, 5);

    __try {
        World* world = GetWorld();
        if (!world || !world->m_isGame) return 0;

        uintptr_t moduleBase = (uintptr_t)GetModuleHandle(NULL);
        void* targetProperties = nullptr;

        if (strcmp(type, "item") == 0) {
            auto& res = world->m_craftResources;
            // The Lua array starts from 1 but internal array usually from 0.
            // Be careful if your ID matches index or not. Assuming ID matches the real ID.
            for (size_t i = 0; i < res.size(); ++i) {
                if (res[i].Id == id || i == id) { // fallback to index if ID is tricky
                    targetProperties = &res[i].m_properties;
                    break;
                }
            }
        } 
        else if (strcmp(type, "block") == 0) {
            auto& blocks = world->m_blockTypes;
            if (id >= 0 && id < (int)blocks.size() && blocks.first) {
                if (blocks[id]) {
                    targetProperties = &blocks[id]->m_properties;
                } else {
                    printf("[-] SetPropertyValue: Block %d is null pointer\n", id);
                }
            } else {
                printf("[-] SetPropertyValue: Block ID %d out of bounds (max %d)\n", id, (int)blocks.size());
            }
        }

        if (targetProperties) {
            SetPropertyValue(moduleBase, targetProperties, propEnum, ruleEnum, value);
            lua_pushboolean(L, true);
            return 1;
        } else {
            printf("[-] SetPropertyValue: Target '%s' ID %d not found!\n", type, id);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        printf("[-] Exception in CTW.setPropertyValue\n");
    }

    lua_pushboolean(L, false);
    return 1;
}

void RegisterDataAPI(lua_State* L) {
    lua_getglobal(L, "CTW");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_setglobal(L, "CTW");
        lua_getglobal(L, "CTW");
    }

    lua_pushcfunction(L, l_getCraftResources);
    lua_setfield(L, -2, "getCraftResources");

    lua_pushcfunction(L, l_getBlockTypes);
    lua_setfield(L, -2, "getBlockTypes");

    lua_pushcfunction(L, l_getRecipes);
    lua_setfield(L, -2, "getRecipes");

    lua_pushcfunction(L, l_setRecipeAllow);
    lua_setfield(L, -2, "setRecipeAllow");

    lua_pushcfunction(L, l_syncRecipeUI);
    lua_setfield(L, -2, "syncRecipeUI");

    lua_pushcfunction(L, l_setPropertyValue);
    lua_setfield(L, -2, "setPropertyValue");

    lua_pop(L, 1);
}
