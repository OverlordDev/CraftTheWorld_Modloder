#pragma once
#include <string>
#include <vector>

struct lua_State;

struct LuaTimer {
    int callbackRef;
    float timeRemaining;
    float interval;
    bool repeat;
};

class LuaManager {
public:
    static LuaManager& GetInstance() {
        static LuaManager instance;
        return instance;
    }

    bool Init();
    void Shutdown();
    void ReloadAll();
    
    // Вызывай из Hooked_OnUpdate или аналогичного каждого кадра (dt в секундах)
    void OnUpdate(float dt); 

    void LoadScript(const std::string& path);
    void FireEvent(const std::string& eventName);
    void FireEventString(const std::string& eventName, const std::string& arg);
    // Вызывает CTW._triggerEvent(eventName, arg) — диспатч через систему onEvent
    void TriggerCTWEvent(const std::string& eventName, const std::string& arg);
    void TriggerBlockEvent(int x, int y, int id);

    int AddTimer(int refId, float seconds, bool repeat);

    lua_State* GetState() { return L; }

private:
    lua_State* L = nullptr;
    std::vector<LuaTimer> m_Timers;
};
