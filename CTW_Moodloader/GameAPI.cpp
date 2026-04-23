#include "pch.h"
#include "GameAPI.h"
#include "Offsets.h"
#include <iostream>
#include <MinHook.h>
#include <cstring>
#include "Menu.h"
#include "LuaManager.h"
#include "ModManager.h"

// Typedef for Game::OnMouseClick
typedef char(__thiscall* OnMouseClick_t)(void* pGame, int x, int y, bool force);

// Typedef for World::OnMouseClick
typedef char(__thiscall* WorldOnMouseClick_t)(void* pWorld, int x, int y);

// Typedef for GetMonstersManager
typedef void* (__cdecl* GetMonstersManager_t)();

OnEvent_t fpOnEventOriginal = nullptr;
AddExp_t fpAddExpOriginal = nullptr;
GetWorkersCount_t fpGetWorkersCountOriginal = nullptr;
OnMouseClick_t fpOnMouseClickOriginal = nullptr;
WorldOnMouseClick_t fpWorldOnMouseClickOriginal = nullptr;
ShowModsDialog_t fpShowModsDialogOriginal = nullptr;
OnDestroyModsDialog_t fpOnDestroyModsDialog = nullptr;
AddCreature_hook_t fpAddCreatureOriginal = nullptr;
CrashBlock_t fpCrashBlockOriginal = nullptr;
SetPropertyValue_raw_t fpSetPropertyValueOriginal = nullptr;

typedef int(__thiscall* StartDialog_OnEvent_t)(void* pThis, void* eventObj);
StartDialog_OnEvent_t fpStartDialog_OnEventOriginal = nullptr;

bool IsMatch(char* data, int offset, const char* target) {
    int len = strlen(target);
    for (int j = 0; j < len; j++) {
        if (data[offset + j] != target[j]) return false;
    }
    return data[offset + len] == '\0';
}

int __fastcall Hooked_StartDialog_OnEvent(void* _this, void* edx, void* eventObj) {
    char* eventData = (char*)eventObj;

    for (int i = 0; i < 100; i++) {
        if (eventData[i] >= 'a' && eventData[i] <= 'z') {
            int len = 0;
            while (eventData[i + len] >= 'a' && eventData[i + len] <= 'z' || eventData[i + len] == '_') {
                len++;
            }
            if (len >= 4 && eventData[i + len] == '\0') {
            }
        }
    }

    for (int i = 0; i < 100; i++) {
        if (IsMatch(eventData, i, "mods")) {
            printf("[MOD] 'mods' button intercepted!\n");
            eventData[i] = 'X'; 

            static DWORD lastMenuToggle = 0;
            if (GetTickCount() - lastMenuToggle > 300) {
                Menu::Toggle();
                lastMenuToggle = GetTickCount();
            }
            break;
        }
        else if (IsMatch(eventData, i, "play")) {
            // eventData[i] = 'X'; 
            break;
        }
        else if (IsMatch(eventData, i, "options")) {
            // eventData[i] = 'X'; 
            break;
        }
        else if (IsMatch(eventData, i, "exit")) {
            // eventData[i] = 'X'; 
            break;
        }
        else if (IsMatch(eventData, i, "dlc_sisters") || IsMatch(eventData, i, "dlc_truemp")) {
            // eventData[i] = 'X'; 
            break;
        }
    }

    return fpStartDialog_OnEventOriginal(_this, eventObj);
}

void* GetDayTimeComponent(uintptr_t moduleBase) {
    GetDayTime_t getDayTimeFunc = (GetDayTime_t)(moduleBase + Offsets::OFFSET_GET_DAYTIME);
    return getDayTimeFunc(nullptr, true);
}

void SetCustomTime(uintptr_t moduleBase, float myTime) {
    void* pDayTime = GetDayTimeComponent(moduleBase);
    if (pDayTime != nullptr) {
        SetCurrentDayTime_t setTimeFunc = (SetCurrentDayTime_t)(moduleBase + Offsets::OFFSET_SET_TIME);
        setTimeFunc(pDayTime, myTime);
        std::cout << "[+] Time set to: " << myTime << std::endl;
    }
    else {
        std::cout << "[-] Error: DayTime component not found!" << std::endl;
    }
}

float GetTimeForState(uintptr_t moduleBase, int stateEnum) {
    void* pDayTime = GetDayTimeComponent(moduleBase);
    if (pDayTime != nullptr) {
        GetStateDayTime_t getStateFunc = (GetStateDayTime_t)(moduleBase + Offsets::OFFSET_GET_STATE);
        return getStateFunc(pDayTime, stateEnum);
    }
    return 0.0f;
}

int FGetCurrentLevel(uintptr_t moduleBase) {
    GetCurrentLevel_t GetExpFunc = (GetCurrentLevel_t)(moduleBase + Offsets::offsetGetXP);
    void* pCharLevels = *(void**)(moduleBase + Offsets::offsetCharLevelsInstance);
    int Level = GetExpFunc(pCharLevels);
    return Level;
}

void ManualAddExp(uintptr_t moduleBase, int amount) {
    AddExp_t AddExpFunc = (AddExp_t)(moduleBase + Offsets::offsetAddExp);
    void* pCharLevels = *(void**)(moduleBase + Offsets::offsetCharLevelsInstance);
    if (pCharLevels != nullptr) {
        printf("[SDK] Manual exp addition: %d\n", amount);
        AddExpFunc(pCharLevels, amount);
    }
    else {
        printf("[-] Error: CharLevels object not found! (Is world loaded?)\n");
    }
}

void AddExp(uintptr_t moduleBase, int amount) {
    ManualAddExp(moduleBase, amount);
}

void SetManaC(uintptr_t moduleBase, float amount) {
    GetGameInstance_t GetInstance = (GetGameInstance_t)(moduleBase + 0x1EEA60);
    void* pGame = GetInstance();
    if (pGame != nullptr) {
        uintptr_t offsetSetMana = 0x4352A0;
        SetMana_t SetMana = (SetMana_t)(moduleBase + offsetSetMana);
        std::cout << "[SDK] SetMana Game: 0x" << std::hex << pGame << std::endl;
        SetMana(pGame, amount);
    }
}

void TriggerLevelUp(uintptr_t moduleBase) {
    GetGameInstance_t GetInstance = (GetGameInstance_t)(moduleBase + 0x1EEA60);
    void* pGame = GetInstance();
    if (pGame != nullptr) {
        ShowLevelUpDialog_t ShowLevelUp = (ShowLevelUpDialog_t)(moduleBase + Offsets::offsetShowLevelUp);
        std::cout << "[SDK] Triggering level up dialog for Game: 0x" << std::hex << pGame << std::endl;
        ShowLevelUp(pGame);
    }
    else {
        std::cout << "[-] Error: Game Instance not initialized yet!" << std::endl;
    }
}

int __fastcall Hooked_OnEvent(void* _this, void* edx, void* pEvent) {
    uintptr_t stringAddr = (uintptr_t)pEvent + 0x0C;
    size_t capacity = *(size_t*)(stringAddr + 0x14);
    const char* eventName = nullptr;

    if (capacity > 15) {
        eventName = *(const char**)stringAddr;
    }
    else {
        eventName = (const char*)stringAddr;
    }

    if (eventName && eventName[0] != '\0') {
        return fpOnEventOriginal(_this, edx, pEvent);
    }
    return 0;
}

char __fastcall Hooked_OnMouseClick(void* pGame, void* edx, int x, int y, bool force) {
    printf("[MouseClick] X=%d, Y=%d, Force=%s\n", x, y, force ? "true" : "false");
    
    char result = fpOnMouseClickOriginal(pGame, x, y, force);
    
    printf("[MouseClick] Result: %s\n", result ? "true" : "false");
    
    return result;
}

char __fastcall Hooked_WorldOnMouseClick(void* pWorld, void* edx, int x, int y) {
    printf("[WorldMouseClick] World click at X=%d, Y=%d\n", x, y);
    
    char result = fpWorldOnMouseClickOriginal(pWorld, x, y);
    
    printf("[WorldMouseClick] Result: %s\n", result ? "true" : "false");
    
    return result;
}

// -------------------------------------------------------
// AddCreature hook — logs every creature spawned in-game
// -------------------------------------------------------
void* __fastcall Hooked_AddCreature(
    void* _this,
    void* edx,
    BaseString32 creatureName,
    const Vec2i* pPos,
    bool changeNameByTemplate,
    void** outObject)
{
    // Read the name from our BaseString32 wrapper
    const char* name = creatureName.size > 0 ? creatureName.buffer : "<unknown>";
    int posX = pPos ? pPos->x : -1;
    int posY = pPos ? pPos->y : -1;

    printf("[AddCreature] '%s' at [%d, %d]\n", name, posX, posY);

    // Огонь события в Lua: CTW.onCreatureSpawn(function(name) ... end)
    LuaManager::GetInstance().TriggerCTWEvent("OnCreatureSpawn", name);

    return fpAddCreatureOriginal(_this, creatureName, pPos, changeNameByTemplate, outObject);
}

void __fastcall Hooked_CrashBlock(void* _this, void* edx, int x, int y, int requiredBlockId, unsigned __int64 byWorkerGUID, unsigned __int64 entityGUID, char byNoNameWorker, bool allowDrops) {
    printf("[CrashBlock] at [%d, %d] requiredBlockId: %d\n", x, y, requiredBlockId);

    // Тригерим событие в Lua
    LuaManager::GetInstance().TriggerBlockEvent(x, y, requiredBlockId);

    fpCrashBlockOriginal(_this, x, y, requiredBlockId, byWorkerGUID, entityGUID, byNoNameWorker, allowDrops);
}

void __fastcall Hooked_SetPropertyValue(void* _this, void* edx, int prop, int rule, SimpleVariant value) {
    const char* strVal = value.m_string.c_str();
    if (value.m_type == 5) { // Assuming 5 is string based on common patterns
        printf("[SetProp] %p Prop:%d Rule:%d Val:\"%s\" (String)\n", _this, prop, rule, strVal);
    } else {
        printf("[SetProp] %p Prop:%d Rule:%d Val:%f (Int/Float:%d)\n", _this, prop, rule, value.m_float, value.m_int);
    }

    // Call original
    fpSetPropertyValueOriginal(_this, prop, rule, value);
}

void InstallHooks(uintptr_t moduleBase) {
    LPVOID targetAddr = (LPVOID)(moduleBase + 0x3E7D10);

    if (MH_Initialize() != MH_OK) return;

    if (MH_CreateHook(targetAddr, &Hooked_OnEvent, (LPVOID*)&fpOnEventOriginal) == MH_OK) {
        MH_EnableHook(targetAddr);
        std::cout << "[SDK] OnEvent hook successfully activated!" << std::endl;
    }

    // Hook Game::OnMouseClick
    uintptr_t offsetOnMouseClick = 0x42DB50;
    if (MH_CreateHook((LPVOID)(moduleBase + offsetOnMouseClick), &Hooked_OnMouseClick, (LPVOID*)&fpOnMouseClickOriginal) == MH_OK) {
        MH_EnableHook((LPVOID)(moduleBase + offsetOnMouseClick));
        std::cout << "[+] Game::OnMouseClick hook activated!" << std::endl;
    }
    else {
        printf("[-] Error: Failed to hook Game::OnMouseClick!\n");
    }

    // Hook World::OnMouseClick
    uintptr_t offsetWorldOnMouseClick = 0x8484C0;
    if (MH_CreateHook((LPVOID)(moduleBase + offsetWorldOnMouseClick), &Hooked_WorldOnMouseClick, (LPVOID*)&fpWorldOnMouseClickOriginal) == MH_OK) {
        MH_EnableHook((LPVOID)(moduleBase + offsetWorldOnMouseClick));
        std::cout << "[+] World::OnMouseClick hook activated!" << std::endl;
    }
    else {
        printf("[-] Error: Failed to hook World::OnMouseClick!\n");
    }

    // Hook StartDialog::OnEvent (main menu button intercept)
    uintptr_t addrStartDialogOnEvent = moduleBase + 0x6C1450;
    if (MH_CreateHook((LPVOID)addrStartDialogOnEvent, &Hooked_StartDialog_OnEvent, (LPVOID*)&fpStartDialog_OnEventOriginal) == MH_OK) {
        MH_EnableHook((LPVOID)addrStartDialogOnEvent);
        printf("[+] Main Menu hook (StartDialog::OnEvent) active\n");
    }

    // Hook MonstersManager::AddCreature — log every creature spawned
    LPVOID addrAddCreature = (LPVOID)(moduleBase + Offsets::offset_AddCreature);
    if (MH_CreateHook(addrAddCreature, &Hooked_AddCreature, (LPVOID*)&fpAddCreatureOriginal) == MH_OK) {
        MH_EnableHook(addrAddCreature);
        printf("[+] AddCreature hook active (creature spawn logging enabled)\n");
    }
    else {
        printf("[-] Error: Failed to hook AddCreature!\n");
    }

    // Hook World::CrashBlock
    LPVOID addrCrashBlock = (LPVOID)(moduleBase + Offsets::offset_CrashBlock);
    if (MH_CreateHook(addrCrashBlock, &Hooked_CrashBlock, (LPVOID*)&fpCrashBlockOriginal) == MH_OK) {
        MH_EnableHook(addrCrashBlock);
        printf("[+] CrashBlock hook active (block destruction logging/Lua enabled)\n");
    }
    else {
        printf("[-] Error: Failed to hook CrashBlock!\n");
    }

    // Hook Properties::SetPropertyValue
    LPVOID addrSetProp = (LPVOID)(moduleBase + Offsets::offset_SetPropertyValue);
    if (MH_CreateHook(addrSetProp, &Hooked_SetPropertyValue, (LPVOID*)&fpSetPropertyValueOriginal) == MH_OK) {
        MH_EnableHook(addrSetProp);
        printf("[+] SetPropertyValue hook active\n");
    }
    else {
        printf("[-] Error: Failed to hook SetPropertyValue!\n");
    }

    printf("[+] Modloader system initialized\n");
    ModManager::GetInstance().Init("mods");
}


void GiveResource(uintptr_t moduleBase, void* pWorld, int id, int amount) {
    if (pWorld == nullptr) return;
    ChangeResourceCount_t ChangeResCount = (ChangeResourceCount_t)(moduleBase + Offsets::offsetChange);
    ChangeResCount(pWorld, id, amount, false, true);
    std::cout << "[+] Added resource ID " << std::dec << id << ": " << amount << std::endl;
}

int FGetResourceCount(uintptr_t moduleBase, void* currentWorld, int id) {
    if (currentWorld == nullptr) return -1;
    GetResourceCount_t GetResourceCount = (GetResourceCount_t)(moduleBase + Offsets::offsetGetResourceCount);
    int count = GetResourceCount(currentWorld, id);
    return count;
}

void GetNameALLResource(uintptr_t moduleBase, void* currentWorld) {
    if (currentWorld == nullptr) return;
    GetResNameById_t GetNameFunc = (GetResNameById_t)(moduleBase + Offsets::offsetGetName);
    std::cout << "--- STARTING DEEP SCAN ---" << std::endl;
    for (int i = 0; i < 1873; i++) {
        StringStorage myStorage;
        GetNameFunc(currentWorld, &myStorage, i);
        char* finalName = nullptr;
        uint32_t cap = *(uint32_t*)((uintptr_t)&myStorage + 20);
        if (cap > 15) {
            finalName = *(char**)&myStorage;
        }
        else {
            finalName = (char*)&myStorage;
        }
        if (finalName != nullptr && strlen(finalName) > 0) {
            std::cout << "[ID " << std::dec << i << "]: " << finalName << std::endl;
        }
    }
}

void TriggerCallDay(uintptr_t moduleBase, void* currentWorld) {
    if (currentWorld == nullptr) {
        std::cout << "[-] Error: world not load" << std::endl;
        return;
    }
    CallDay_t callDayFunc = (CallDay_t)(moduleBase + Offsets::offsetCallDay);
    callDayFunc(currentWorld);
    std::cout << "[+] Set Day!" << std::endl;
}

void TriggerCalNight(uintptr_t moduleBase, void* currentWorld) {
    if (currentWorld == nullptr) {
        std::cout << "[-] Error: world not load" << std::endl;
        return;
    }
    CallNight_t callNightFunc = (CallNight_t)(moduleBase + Offsets::offsetCallNight);
    callNightFunc(currentWorld);
    std::cout << "[+] Set Night" << std::endl;
}

void BuildBlockAt(int x, int y, int blockID, bool isFront) {
    uintptr_t base = (uintptr_t)GetModuleHandle(NULL);
    void* pWorld = *(void**)(base + Offsets::offsetWorldInstance);
    if (pWorld) {
        __int64 packedCoords = ((__int64)y << 32) | (unsigned int)x;
        BuildBlock_t BuildFunc = (BuildBlock_t)(base + Offsets::BuildBlock);
        BuildFunc(pWorld, packedCoords, blockID, 0, false, isFront);
        printf("[BUILD] Block %d placed at [%d, %d]\n", blockID, x, y);
    }
}
typedef bool(__thiscall* StartPortaling_t)(void* pWarehouse, void* pName);
void SpawnDwarf(uintptr_t moduleBase, int count) {
    StartPortaling_t portalFunc = (StartPortaling_t)(moduleBase + 0x55B8A0);
    uint8_t* updateFunc = (uint8_t*)(moduleBase + 0x397400);
    void** pWarehouseInstance = nullptr;
    for (int i = 0; i < 0x800; i++) {
        if (updateFunc[i] == 0x83 && updateFunc[i + 1] == 0x3D && updateFunc[i + 6] == 0x00) {
            uintptr_t addr = *(uintptr_t*)(updateFunc + i + 2);
            if (addr > moduleBase + 0xC00000 && addr < moduleBase + 0xF00000) {
                pWarehouseInstance = (void**)addr;
                printf("[+] MainWarehouseEntity::m_instance found at: 0x%X\n", (unsigned int)addr);
                break;
            }
        }
    }

    if (pWarehouseInstance == nullptr || *pWarehouseInstance == nullptr) {
        printf("[-] Error: MainWarehouseEntity::m_instance not found or null!\n");
        printf("    (Is world loaded? Portal built?)\n");
        return;
    }

    void* warehouse = *pWarehouseInstance;

    //    +0x10: size_t size

    for (int i = 0; i < count; i++) {
        char fakeString[28];
        memset(fakeString, 0, sizeof(fakeString));

        const char* workerName = "worker";
        strcpy_s(fakeString, 16, workerName);

        // size = 6
        *(unsigned int*)(fakeString + 16) = 6;
        // capacity = 15 (SSO)
        *(unsigned int*)(fakeString + 20) = 15;

        bool result = portalFunc(warehouse, fakeString);

        if (result) {
            printf("[+] Dwarf #%d spawned via portal!\n", i + 1);
        }
        else {
            printf("[!] Portal busy, dwarf #%d waiting...\n", i + 1);
            break;
        }
    }
}

void SpawnDwarfDirect(uintptr_t moduleBase, void* pAIManager) {
    SpawnDwarf(moduleBase, 100);
}

void SpawnZombie(uintptr_t moduleBase, int x, int y, bool boss) {
    GetMonstersManager_t getManagerFunc = (GetMonstersManager_t)(moduleBase + Offsets::offset_GetMonstersManagerInstance);
    void* pManager = *(void**)(moduleBase + Offsets::offset_GetMonstersManagerInstance);
    if (pManager == nullptr) {
        std::cout << "[-] Error: MonstersManager not found!" << std::endl;
        return;
    }

    const char* creatureName = boss ? "zombie_boss" : "zombie";
    BaseString32 creatureNameStr{};
    strcpy_s(creatureNameStr.buffer, sizeof(creatureNameStr.buffer), creatureName);
    creatureNameStr.size = (unsigned int)strlen(creatureName);
    creatureNameStr.capacity = 15;

    Vec2i position = { x, y };
    AddCreature_t addCreatureFunc = (AddCreature_t)(moduleBase + Offsets::offset_AddCreature);
    void* outObject = nullptr;
    void* spawned = addCreatureFunc(pManager, creatureNameStr, &position, false, &outObject);
    if (spawned) {
        std::cout << "[+] " << creatureName << " spawned at [" << x << ", " << y << "]" << std::endl;
    }
    else {
        std::cout << "[-] Error: Failed to spawn " << creatureName << " at [" << x << ", " << y << "]" << std::endl;
    }
}

void SpawnSheep(uintptr_t moduleBase, int x, int y) {
    GetMonstersManager_t getManagerFunc = (GetMonstersManager_t)(moduleBase + Offsets::offset_GetMonstersManagerInstance);
    void* pManager = *(void**)(moduleBase + Offsets::offset_GetMonstersManagerInstance);
    if (pManager == nullptr) {
        std::cout << "[-] Error: MonstersManager not found!" << std::endl;
        return;
    }

    const char* creatureName = "sheep";
    BaseString32 creatureNameStr{};
    strcpy_s(creatureNameStr.buffer, sizeof(creatureNameStr.buffer), creatureName);
    creatureNameStr.size = (unsigned int)strlen(creatureName);
    creatureNameStr.capacity = 15;

    Vec2i position = { x, y };
    AddCreature_t addCreatureFunc = (AddCreature_t)(moduleBase + Offsets::offset_AddCreature);
    void* outObject = nullptr;
    void* spawned = addCreatureFunc(pManager, creatureNameStr, &position, false, &outObject);
    if (spawned) {
        std::cout << "[+] Sheep spawned at [" << x << ", " << y << "]" << std::endl;
    }
    else {
        std::cout << "[-] Error: Failed to spawn sheep at [" << x << ", " << y << "]" << std::endl;
    }
}

void DropItem(int resId, int count, float x, float y) {
    uintptr_t base = (uintptr_t)GetModuleHandle(NULL);
    void* pWorld = *(void**)(base + Offsets::offsetWorldInstance);
    if (!pWorld) return;

    CreateDynamicObjectRes_t CreateFunc = (CreateDynamicObjectRes_t)(base + Offsets::offset_CreateDynamicObjectRes);

    // Dummy array for iqArray<DynamicEntity*> result
    // CTW's iqArray is ~24-32 bytes, using a buffer for safety
    char dummyResult[256];
    memset(dummyResult, 0, sizeof(dummyResult));

    // recipeId = -1 (not from recipe)
    // assignment = 0 (ASSIGN_COLLECT - makes dwarfs want to pick it up)
    CreateFunc(pWorld, dummyResult, resId, -1, count, x, y, 0);

    printf("[DropItem] ID: %d, Count: %d at [%.1f, %.1f]\n", resId, count, x, y);
}

// -------------------------------------------------------
// Property API (SetPropertyValue wrapper)
// -------------------------------------------------------
typedef void(__thiscall* SetPropertyValue_t)(void* pProperties, int propEnum, int ruleEnum, float value);

void SetPropertyValue(uintptr_t moduleBase, void* pProperties, int propEnum, int ruleEnum, float value) {
    if (!pProperties) return;

    SetPropertyValue_t SetPropFunc = (SetPropertyValue_t)(moduleBase + Offsets::offset_SetPropertyValue);
    SetPropFunc(pProperties, propEnum, ruleEnum, value);
    
    printf("[Property] Set Property %d to %f (Rule %d)\n", propEnum, value, ruleEnum);
}