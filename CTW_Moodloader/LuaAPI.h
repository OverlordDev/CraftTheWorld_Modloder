#pragma once
struct lua_State;

// Регистрирует глобальную таблицу CTW и её функции в переданном состоянии
void RegisterLuaAPI(lua_State* L);
