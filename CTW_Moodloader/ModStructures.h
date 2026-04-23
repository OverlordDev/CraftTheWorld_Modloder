#pragma once
#include <vector>
#include <string>
#include <map>
#include <cstdint>

// Forward declarations
struct CraftResource;
struct Recipe;
struct BlockType;
struct Item;
struct InventoryDialog;
struct Game;

template<typename T>
struct vec2 {
    T x, y;
};

// Internal game array structure
template<typename T>
struct iqArray {
    T* first;
    T* last;
    T* end;

    size_t size() const {
        if (!first || !last || last < first) return 0;
        return (size_t)(last - first);
    }

    T& operator[](size_t i) {
        return first[i];
    }
};

// Standard game string structure
struct BaseString {
    struct _Mypair_type {
        struct _Myval2_type {
            union _Bx_type {
                char _Buf[16];
                char* _Ptr;
            } _Bx;
            uint32_t _Mysize;
            uint32_t _Myres;
        } _Myval2;
    } m_data;

    const char* c_str() const {
        return (m_data._Myval2._Myres > 15) ? m_data._Myval2._Bx._Ptr : m_data._Myval2._Bx._Buf;
    }
};

struct iqHashStringBase {
    uint32_t m_hash;
    BaseString m_data;
};

// Refined CraftResource based on research
struct CraftResource {
    iqArray<void*> m_properties; // 0x00
    uint32_t m_bits;             // 0x0C
    int Id;                      // 0x10
    int Order;                   // 0x14
    bool Allow;                  // 0x18
    char pad_0[3];               // 0x19
    iqHashStringBase Name;       // 0x1C
    BaseString FileName;         // 0x38
    int Count;                   // 0x50
    int Application;             // 0x54
    int Class;                   // 0x58
    BaseString Description;      // 0x5C
    BaseString DescParam1;       // 0x74
    BaseString DescParam2;       // 0x8C
    BaseString LocalForDescription; // 0xA4
    BaseString LocalForTitle;    // 0xBC
    int BuildBlockId;            // 0xD4
    iqHashStringBase BuildBlock; // 0xD8
    void* BuildBlockPtr;         // 0xF4
    BaseString Title;            // 0xF8
    char pad_rest[532 - 0x110];  // 0xF8 + 24 = 0x110
};

struct BlockType {
    iqArray<void*> m_properties; // 0x00
    char pad_rest_0[104];        // 0x0C to 0x74
    char m_params[28];           // 0x74
    iqHashStringBase Name;       // 0x80
    char pad_rest_1[272 - 0x9C];
    float Extract;               // 0x110 (272)
    int Build;                   // 0x114 (276)
    char pad_rest_2[368 - 0x118];
    int Application;             // 0x170 (368)
};

struct Recipe {
    int Id;                      // 0x00
    iqHashStringBase Name;       // 0x04
    bool Enabled;                // 0x20
    char pad_0[3];
    int CraftNeed;               // 0x24
    int Group;                   // 0x28
    struct {
        iqHashStringBase group;
        int priority;
    } PriGroup;                  // 0x2C
    iqArray<void*> Ingredients;  // 0x4C
    iqArray<void*> IngredientsForRandom; // 0x58
    int ProduceId;               // 0x64
    int ProduceCount;            // 0x68
    iqArray<void*> RequireRule;  // 0x6C
    int SaveToNextLevel;         // 0x78
    int NeedCounter;             // 0x7C
    bool NeedOk;                 // 0x80
    char pad_1[3];
    int RuleCounter;             // 0x84
    iqHashStringBase SearchGroup; // 0x88 (Wait! searchgroup starts at 0x84 actually)
    char pad_internal[12];       // Padding to reach 0xA0
    bool Allow;                  // 0xA0
    bool AllowOrig;              // 0xA1
    bool ReadyForEnable;         // 0xA2
    bool CraftTreeEnabled;       // 0xA3
    // Add more if needed to reach 216
    char pad_rest[216 - 0xA4];
};

#pragma pack(push, 1)

struct World {
    void* vftable;           // 0x00
    uint32_t m_count;        // 0x04
    int m_ambientLight;      // 0x08
    int m_ambientLightRGB[3]; // 0x0C
    void* m_ambientRGBTemplateImage; // 0x18
    iqArray<CraftResource> m_craftResources; // 0x1C
    bool m_craftResourcesChanges; // 0x28
    char pad_0x29[3];
    iqArray<Recipe> m_recipes; // 0x2C
    vec2<int> m_homePosition; // 0x38
    iqArray<vec2<int>> m_homePositions; // 0x40
    vec2<int> m_shopPosition; // 0x4C
    bool m_isGame; // 0x54
    char pad_0x55[3];
    vec2<int> m_worldAnchor; // 0x58
    int m_horizontLine; // 0x60
    int m_dayNightSwitch; // 0x64
    int m_dayTimePrevState; // 0x68
    int m_backgroundDaySkyImageId; // 0x6C
    int m_backgroundDayBgImageId; // 0x70
    int m_dayAlpha; // 0x74
    int m_backgroundStormSkyImageId; // 0x78
    int m_backgroundSunsetSkyImageId; // 0x7C
    int m_backgroundSunsetBgImageId; // 0x80
    int m_sunsetAlpha; // 0x84
    int m_backgroundNightSkyImageId; // 0x88
    int m_backgroundNightBgImageId; // 0x8C
    int m_nightAlpha; // 0x90
    float m_currentDayTime; // 0x94
    int m_banImageId; // 0x98
    int m_highlightedImageId; // 0x9C
    int m_banUpImageId; // 0xA0
    int m_ResPreviewPanelImageId; // 0xA4
    vec2<float> m_oldViewPos; // 0xA8
    char pad_0xB0[52]; // 0xB0 -> 0xE4
    iqArray<BlockType*> m_blockTypes; // 0xE4
    // Don't need more for now
};

#pragma pack(pop)

// Simplified InventoryDialog for UI refresh
struct InventoryDialog {
    void* vTable; // 0x00
    uint32_t m_count; // 0x04
    // Many more fields...
    
    // Member functions - mapped by address
    void Refresh() {
        uintptr_t base = (uintptr_t)GetModuleHandle(NULL);
        typedef void(__thiscall* Refresh_t)(InventoryDialog*);
        Refresh_t func = (Refresh_t)(base + 0x8C2010);
        func(this);
    }
};

// Simplified Game class for instance access
struct Game {
    // Member fields will be accessed by offset until layout is fully mapped
    static Game* GetInstance() {
        uintptr_t base = (uintptr_t)GetModuleHandle(NULL);
        typedef Game* (__cdecl* GetInstance_t)();
        GetInstance_t func = (GetInstance_t)(base + 0x5EEA60);
        return func();
    }
    
    InventoryDialog* GetActiveInventory() {
        // According to our research, m_invDialog is around offset 0x140+ in Game
        // We'll use a safer access pattern or search for it if offset is unknown.
        // For now, let's assume it's one of the Ref pointers near the middle.
        // Based on the constructor, it's the 11th Ref after m_directControlWorker.
        return nullptr; // Placeholder - will implement in DataAPI with search logic
    }
};

// Value container for properties
struct SimpleVariant {
    int m_type;      // 0x00
    union {
        int m_int;
        float m_float;
        bool m_bool;
        uint32_t m_uint;
        char m_raw[32]; // Data buffer
    };               // 0x04
    BaseString m_string; // 0x24 (36)
    // Total size: 36 + 24 = 60.

    SimpleVariant() {
        memset(this, 0, 36);
        m_string.m_data._Myval2._Myres = 15;
        m_string.m_data._Myval2._Bx._Buf[0] = 0;
        m_string.m_data._Myval2._Mysize = 0;
    }
};

struct PropertyStruct {
    int rule;               // 0x00
    int prop;               // 0x04
    int way;                // 0x08
    SimpleVariant variantValue; // 0x0C
    char pad_creatures[44]; // 0x48
    iqHashStringBase name;  // 0x74
};

struct Properties {
    iqArray<PropertyStruct> m_properties; // 0x00
    uint32_t m_bits[4];                    // 0x0C
};
