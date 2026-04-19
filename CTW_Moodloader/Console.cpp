#include "Console.h"
#include <stdarg.h>
#include <iostream>
#include <streambuf>
#include <string>
#include <sstream>
#include "GameAPI.h"

namespace Console {
    static ImGuiTextBuffer    Buf;
    static ImGuiTextFilter    Filter;
    static ImVector<int>      LineOffsets;
    static bool               AutoScroll = true;
    static bool               ScrollToBottom = false;
    static std::string        LineBuffer;
    static std::streambuf*    OldCoutBuf = nullptr;
    static std::streambuf*    OldCerrBuf = nullptr;
    static std::streambuf*    OldClogBuf = nullptr;
    static char               InputBuf[256] = "";

    void ExecuteCommand(const char* command_line) {
        Console::AddLog("# %s\n", command_line);
        std::string cmd_str(command_line);
        std::istringstream iss(cmd_str);
        std::string command;
        iss >> command;

        uintptr_t moduleBase = (uintptr_t)GetModuleHandle(NULL);

        if (command == "time") {
            float timeVal;
            if (iss >> timeVal) {
                SetCustomTime(moduleBase, timeVal);
                Console::AddLog("[+] Time successfully changed to %.3f!\n", timeVal);
            } else {
                Console::AddLog("[-] Error. Usage: time <float>\n");
            }
        }
        else if (command == "lvlup") {
            TriggerLevelUp(moduleBase);
            Console::AddLog("[+] Level up dialog triggered!\n");
        }
        else if (command == "spawn_dwarf") {
            int count;
            if (iss >> count) {
                SpawnDwarf(moduleBase, count);
            } else {
                Console::AddLog("[-] Error. Usage: spawn_dwarf <count>\n");
            }
        }
        else {
            Console::AddLog("[-] Unknown command: '%s'\n", command.c_str());
        }
    }

    struct ConsoleStreamBuf : std::streambuf {
        int overflow(int c) override {
            if (c == EOF) return EOF;
            char ch = static_cast<char>(c);
            if (ch == '\r') return c;
            LineBuffer.push_back(ch);
            if (ch == '\n') {
                AddLog("%s", LineBuffer.c_str());
                LineBuffer.clear();
            }
            return c;
        }

        std::streamsize xsputn(const char* s, std::streamsize count) override {
            for (std::streamsize i = 0; i < count; i++) {
                overflow(s[i]);
            }
            return count;
        }
    };

    static ConsoleStreamBuf ConsoleBuf;

    void RedirectStdStreams() {
        if (!OldCoutBuf) OldCoutBuf = std::cout.rdbuf(&ConsoleBuf);
        if (!OldCerrBuf) OldCerrBuf = std::cerr.rdbuf(&ConsoleBuf);
        if (!OldClogBuf) OldClogBuf = std::clog.rdbuf(&ConsoleBuf);
    }

    void RestoreStdStreams() {
        if (OldCoutBuf) {
            std::cout.rdbuf(OldCoutBuf);
            OldCoutBuf = nullptr;
        }
        if (OldCerrBuf) {
            std::cerr.rdbuf(OldCerrBuf);
            OldCerrBuf = nullptr;
        }
        if (OldClogBuf) {
            std::clog.rdbuf(OldClogBuf);
            OldClogBuf = nullptr;
        }
    }

    void Initialize() {
        Buf.clear();
        LineOffsets.clear();
        LineOffsets.push_back(0);
        Filter.Clear();
        AutoScroll = true;
        ScrollToBottom = false;
        LineBuffer.clear();
        RedirectStdStreams();
    }

    void Shutdown() {
        RestoreStdStreams();
    }

    void Clear() {
        Buf.clear();
        LineOffsets.clear();
        LineOffsets.push_back(0);
    }

    void AddLog(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        int old_size = Buf.size();
        Buf.appendfv(fmt, args);
        va_end(args);
        for (int new_size = Buf.size(); old_size < new_size; old_size++) {
            if (Buf[old_size] == '\n')
                LineOffsets.push_back(old_size + 1);
        }
        ScrollToBottom = true;
    }

    void Draw(const char* title, bool* p_open) {
        ImGui::SetNextWindowSize(ImVec2(640, 280), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(title, p_open, ImGuiWindowFlags_None)) {
            ImGui::End();
            return;
        }

        if (ImGui::Button("Clear"))
            Clear();
        ImGui::SameLine();
        if (ImGui::Button("Copy"))
            ImGui::LogToClipboard();
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &AutoScroll);
        ImGui::Separator();

        ImGui::BeginChild("ConsoleScrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        if (Filter.IsActive()) {
            const char* buf_begin = Buf.begin();
            for (int line_no = 0; line_no < LineOffsets.Size; line_no++) {
                const char* line_start = buf_begin + LineOffsets[line_no];
                const char* line_end = (line_no + 1 < LineOffsets.Size) ? buf_begin + LineOffsets[line_no + 1] - 1 : Buf.end();
                if (Filter.PassFilter(line_start, line_end))
                    ImGui::TextUnformatted(line_start, line_end);
            }
        } else {
            ImGui::TextUnformatted(Buf.begin(), Buf.end());
        }

        if (ScrollToBottom && (AutoScroll || ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
            ImGui::SetScrollHereY(1.0f);
        ScrollToBottom = false;

        ImGui::EndChild();

        ImGui::Separator();
        bool reclaim_focus = false;
        
        ImGui::PushItemWidth(-60);
        if (ImGui::InputText("##CommandInput", InputBuf, IM_ARRAYSIZE(InputBuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (InputBuf[0]) {
                ExecuteCommand(InputBuf);
            }
            strcpy_s(InputBuf, sizeof(InputBuf), "");
            reclaim_focus = true;
        }
        ImGui::PopItemWidth();
        
        ImGui::SameLine();
        if (ImGui::Button("Enter", ImVec2(50, 0))) {
            if (InputBuf[0]) {
                ExecuteCommand(InputBuf);
                strcpy_s(InputBuf, sizeof(InputBuf), "");
            }
            reclaim_focus = true;
        }

        ImGui::SetItemDefaultFocus();
        if (reclaim_focus) {
            ImGui::SetKeyboardFocusHere(-1); 
        }

        ImGui::End();
    }
}