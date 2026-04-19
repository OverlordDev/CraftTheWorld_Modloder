#pragma once
#include "Mod.h"
#include <vector>
#include <string>

class ModManager {
public:
    static ModManager& GetInstance() {
        static ModManager instance;
        return instance;
    }

    void Init(const std::string& modsDirectory);
    void ReloadAll();
    void ToggleMod(const std::string& modId);
    void DeleteMod(const std::string& modId);
    const std::vector<Mod>& GetMods() const { return m_Mods; }

private:
    std::vector<Mod> m_Mods;
    std::string m_ModsDirectory;

    void ScanDirectory();
    bool ParseManifest(const std::string& manifestPath, Mod& outMod);
    void TopoSort();
};
