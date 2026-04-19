#pragma once

#include "imgui/imgui.h"

namespace Console {
    void Initialize();
    void Shutdown();
    void Clear();
    void AddLog(const char* fmt, ...) IM_FMTARGS(1);
    void Draw(const char* title, bool* p_open = nullptr);
}
