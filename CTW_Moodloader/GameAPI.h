#pragma once
#include <windows.h>
struct StringStorage {
    char junk[64];
};
struct Vec2i {
    int x;
    int y;
};
#pragma pack(push, 1)
struct BaseString32 {
    char buffer[16];
    unsigned int size;
    unsigned int capacity;

    BaseString32(const char* str) {
        size = (unsigned int)strlen(str);
        capacity = 15;
        memset(buffer, 0, 16);
        memcpy(buffer, str, size < 16 ? size : 15);
    }

    BaseString32() {
        size = 0;
        capacity = 15;
        memset(buffer, 0, 16);
    }
};
#pragma pack(pop)
void SetCustomTime(uintptr_t moduleBase, float myTime);
float GetTimeForState(uintptr_t moduleBase, int stateEnum);
int FGetCurrentLevel(uintptr_t moduleBase);
void AddExp(uintptr_t moduleBase, int amount);
void __fastcall Hooked_AddExp(void* _this, void* edx, int amount);
void SetManaC(uintptr_t moduleBase, float amount);
void TriggerLevelUp(uintptr_t moduleBase);
void* __fastcall Hooked_ShowInventoryDialog(void* pGame, void* edx, bool param_1);
int __fastcall Hooked_OnEvent(void* _this, void* edx, void* pEvent);
void InstallHooks(uintptr_t moduleBase);
void GiveResource(uintptr_t moduleBase, void* pWorld, int id, int amount);
int FGetResourceCount(uintptr_t moduleBase, void* currentWorld, int id);
void GetNameALLResource(uintptr_t moduleBase, void* currentWorld);
void TriggerCallDay(uintptr_t moduleBase, void* currentWorld);
void TriggerCalNight(uintptr_t moduleBase, void* currentWorld);
void SpawnDwarf(uintptr_t moduleBase, int count);
void SpawnDwarfDirect(uintptr_t moduleBase, void* pAIManager);
void SetupImGuiHook(uintptr_t moduleBase);
void BuildBlockAt(int x, int y, int blockID, bool isFront);
void AddBonusGnomes(uintptr_t moduleBase, int amount);
void SpawnZombie(uintptr_t moduleBase, int x, int y, bool boss = false);
using CallDay_t = void(__thiscall*)(void* pWorld);
using CallNight_t = void(__thiscall*)(void* pWorld);
using GetResNameById_t = void* (__thiscall*)(void* pWorld, StringStorage* pOut, int id);
using GetResourceCount_t = int(__thiscall*)(void* pWorld, int id);
using ChangeResourceCount_t = bool(__thiscall*)(void* pWorld, int id, int count, bool isCrafted, bool silent);
using ShowInventoryDialog_t = void* (__thiscall*)(void* pGame, bool param_1);
using OnEvent_t = int(__fastcall*)(void* _this, void* edx, void* pEvent);
using GetGameInstance_t = void* (__cdecl*)();
using ShowLevelUpDialog_t = void(__thiscall*)(void* pGame);
using SetMana_t = void(__thiscall*)(void* pGame, float amount);
using AddExp_t = void(__thiscall*)(void* _this, int amount);
using GetCurrentLevel_t = int(__thiscall*)(void* _this);
using SetTime_t = void(__thiscall*)(void* _this, float time);
using GetDayTime_t = void* (__cdecl*)(const char* name, bool saveable);
using SetCurrentDayTime_t = void(__thiscall*)(void* pDayTime, float time);
using GetStateDayTime_t = float(__thiscall*)(void* pDayTime, int stateEnum);
using SpawnWorkers_t = void(__thiscall*)(void* _this, int oldLevel, int newLevel);
using GetWorkersCount_t = int(__thiscall*)(void* _this);
using BuildBlock_t = void(__thiscall*)(
    void* _this,
    __int64 x,
    int id,
    char option,
    bool byWorker,
    bool forceFront
    );
using Blackboard_Get_t = void* (__cdecl*)(void* result, void* hashTag);
using AddAchivementResource_t = void(__thiscall*)(void* pGame, void* pName);
using AddCreature_t = void* (__thiscall*)(
    void* pThisManager,
    BaseString32 creatureName,
    const Vec2i* pPos,      
    bool changeNameByTemplate,
    void** outObject
    );
using CreateSkeleton_t = void* (__cdecl*)(const Vec2i* pos);
void SpawnSheep(uintptr_t moduleBase, int x, int y);
using AddCreature_hook_t = void* (__thiscall*)(
    void* _this,
    BaseString32 creatureName,
    const Vec2i* pPos,      
    bool changeNameByTemplate,
    void** outObject
    );
extern AddCreature_hook_t fpAddCreatureOriginal;
using ShowModsDialog_t = void(__thiscall*)(void* _this);
using OnDestroyModsDialog_t = void(__thiscall*)(void* _this);
extern ShowModsDialog_t fpShowModsDialogOriginal;
extern OnDestroyModsDialog_t fpOnDestroyModsDialog;
void __fastcall Hooked_ShowModsDialog(void* _this, void* edx);
void* __fastcall Hooked_AddCreature(void* _this, void* edx, BaseString32 creatureName, const Vec2i* pPos, bool changeNameByTemplate, void** outObject);

// Hooked CrashBlock
typedef void(__thiscall* CrashBlock_t)(void* _this, int x, int y, int requiredBlockId, unsigned __int64 byWorkerGUID, unsigned __int64 entityGUID, char byNoNameWorker, bool allowDrops);
extern CrashBlock_t fpCrashBlockOriginal;
void __fastcall Hooked_CrashBlock(void* _this, void* edx, int x, int y, int requiredBlockId, unsigned __int64 byWorkerGUID, unsigned __int64 entityGUID, char byNoNameWorker, bool allowDrops);

// Drop Item API
typedef void* (__thiscall* CreateDynamicObjectRes_t)(void* _this, void* result_array, int resId, int recipeId, int count, float x, float y, int assignment);
void DropItem(int resId, int count, float x, float y);