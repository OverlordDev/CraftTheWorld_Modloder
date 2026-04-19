#pragma comment(linker, "/export:MiniDumpWriteDump=C:\\Windows\\System32\\dbghelp.MiniDumpWriteDump")
#pragma comment(linker, "/export:StackWalk64=C:\\Windows\\System32\\dbghelp.StackWalk64")
#pragma comment(linker, "/export:SymCleanup=C:\\Windows\\System32\\dbghelp.SymCleanup")
#pragma comment(linker, "/export:SymFunctionTableAccess64=C:\\Windows\\System32\\dbghelp.SymFunctionTableAccess64")
#pragma comment(linker, "/export:SymGetLineFromAddr64=C:\\Windows\\System32\\dbghelp.SymGetLineFromAddr64")
#pragma comment(linker, "/export:SymGetModuleBase64=C:\\Windows\\System32\\dbghelp.SymGetModuleBase64")
#pragma comment(linker, "/export:SymGetSymFromAddr64=C:\\Windows\\System32\\dbghelp.SymGetSymFromAddr64")
#pragma comment(linker, "/export:SymInitialize=C:\\Windows\\System32\\dbghelp.SymInitialize")
#pragma comment(linker, "/export:SymLoadModule64=C:\\Windows\\System32\\dbghelp.SymLoadModule64")
#pragma comment(linker, "/export:SymSetOptions=C:\\Windows\\System32\\dbghelp.SymSetOptions")
#include "pch.h" 
#include "Offsets.h"
#include <windows.h>
#include <iostream>
#include <MinHook.h>
#include <intrin.h>
#include "GameAPI.h"
#include "kiero/kiero.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include <d3d11.h>
#include "Menu.h"
#include "Console.h"
#include "LuaManager.h"
#define printf(...) Console::AddLog(__VA_ARGS__)
typedef HRESULT(__stdcall* Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

extern Present oPresent;
extern HWND window;
extern WNDPROC oWndProc;
extern ID3D11Device* pDevice;
extern ID3D11DeviceContext* pContext;
extern ID3D11RenderTargetView* mainRenderTargetView;
extern bool init;

Present oPresent = NULL;
HWND window = NULL;
WNDPROC oWndProc = NULL;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView = NULL;
bool init = false;
bool isUnloading = false; 
ImGuiContext* g_pImGuiContext = nullptr;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (!isUnloading && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;

    if (g_pImGuiContext != nullptr && !isUnloading) {
        ImGuiIO& io = ImGui::GetIO();
        
        if (io.WantCaptureMouse) {
            if (uMsg == WM_MOUSEWHEEL || uMsg == WM_MOUSEHWHEEL || 
               (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)) {
                return true;
            }
        }
        
        if (io.WantCaptureKeyboard) {
            if (uMsg == WM_KEYDOWN || uMsg == WM_KEYUP || 
                uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP || uMsg == WM_CHAR) {
                return true;
            }
        }
    }

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}
HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    if (!init)
    {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
        {
            pDevice->GetImmediateContext(&pContext);
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            window = sd.OutputWindow;

            ID3D11Texture2D* pBackBuffer;
            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
            pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
            pBackBuffer->Release();

            oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);

            g_pImGuiContext = ImGui::CreateContext();
            ImGui::SetCurrentContext(g_pImGuiContext);
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

            ImFont* cyrillicFont = io.Fonts->AddFontFromFileTTF(
                "C:\\Windows\\Fonts\\segoeui.ttf",
                18.0f,
                nullptr,
                io.Fonts->GetGlyphRangesCyrillic()
            );
            if (cyrillicFont == nullptr) {
                std::cout << "[-] ImGui Error: failed to load font" << std::endl;
            }

            ImGui_ImplWin32_Init(window);
            ImGui_ImplDX11_Init(pDevice, pContext);

            init = true;
        }
        else {
            return oPresent(pSwapChain, SyncInterval, Flags);
        }
    }

    if (!isUnloading && g_pImGuiContext) {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        
        LuaManager::GetInstance().OnUpdate(ImGui::GetIO().DeltaTime);
        
        Menu::Draw();
        ImGui::Render();

        pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    return oPresent(pSwapChain, SyncInterval, Flags);
}

static void Cleanup() {
    if (!init) {
        return;
    }

    if (g_pImGuiContext) {
        ImGui::SetCurrentContext(g_pImGuiContext);
    }

    if (mainRenderTargetView) {
        mainRenderTargetView->Release();
        mainRenderTargetView = NULL;
    }

    if (g_pImGuiContext) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        g_pImGuiContext = nullptr;
    }
    
    Console::Shutdown();

    if (oWndProc) {
        SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)oWndProc);
        oWndProc = NULL;
    }

    if (oPresent) {
        kiero::unbind(8);
    }
    kiero::shutdown();

    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    init = false;
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    AllocConsole();
    freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    FILE* fIn;
    FILE* fOut;
    freopen_s(&fIn, "CONIN$", "r", stdin);
    freopen_s(&fOut, "CONOUT$", "w", stdout);
    SetConsoleOutputCP(65001);
    Console::Initialize();

    uintptr_t moduleBase = (uintptr_t)GetModuleHandle(NULL);

    LuaManager::GetInstance().Init();

    std::cout << "[+] Developer console successfully loaded!" << std::endl;
    Console::AddLog("[+] Developer console successfully loaded!\n");
    std::cout << "[+] Base Adress: 0x" << std::hex << moduleBase << std::endl;
    Console::AddLog("[+] Base Address: 0x%p\n", (void*)moduleBase);
    std::cout << "[+] Craft the World Mod Loader v1.0" << std::endl;
    Console::AddLog("[+] Craft the World Mod Loader v1.0\n");
    std::cout << "[!] INSERT - Menu | F6 - Reload Mods | END - Exit" << std::endl;
    Console::AddLog("[!] INSERT - Menu | F6 - Reload Mods | END - Exit\n");

    bool init_hook = false;
    InstallHooks(moduleBase);

    do
    {
        if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
        {
            kiero::bind(8, (void**)&oPresent, hkPresent); 
            init_hook = true;
        }
    } while (!init_hook);

    while (!GetAsyncKeyState(VK_END))
    {
        Sleep(10); 

        if (GetAsyncKeyState(VK_INSERT) & 1)
        {
            Menu::Toggle();
        }

        // Hotkey: Reload Lua Mods
        if (GetAsyncKeyState(VK_F6) & 1)
        {
            LuaManager::GetInstance().ReloadAll();
            Console::AddLog("[!] Lua scripts successfully reloaded (F6)\n");
        }
    }

    std::cout << "Unloading..." << std::endl;
    Console::AddLog("[!] Unloading...\n");
    isUnloading = true;  
    Cleanup();
    FreeLibraryAndExitThread((HMODULE)lpParam, 0);
    return 0;
}
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, MainThread, hModule, 0, NULL);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}