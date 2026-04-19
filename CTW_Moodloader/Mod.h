#pragma once
#include <string>
#include <vector>

struct Mod {
    std::string id;
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    std::string path;           // absolute or relative path to mod folder
    std::vector<std::string> scripts;
    std::vector<std::string> dependencies;
    bool enabled = true;
    bool loaded = false;
};
