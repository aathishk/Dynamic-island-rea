// standalone_app.h - entry point and tray integration for the standalone EXE.
//
// Included once at the very end of dynamic-island-ultra.cpp (inside
// #ifdef DYNAMIC_ISLAND_STANDALONE), so it can use the island's own globals:
// g_hwnd, g_renderThread, StartThreads(), StopThreads(), LoadSettings(),
// g_layoutDirty and ID_HIDE_SHOW_HOTKEY.
//
// Provides: single-instance guard, tray icon with a context menu (show/hide,
// position, run at startup, open settings/log folder, exit) and a message loop
// that keeps the tray responsive.
#pragma once

namespace standalone_app {

constexpr UINT kTrayMessage = WM_APP + 500;
constexpr UINT kTrayId = 1001;
constexpr wchar_t kTrayClass[] = L"DynamicIsland.TrayWindow";
constexpr wchar_t kRunKeyPath[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
constexpr wchar_t kRunValueName[] = L"DynamicIsland";

enum MenuId : UINT {
    kCmdToggle = 1,
    kCmdStartup,
    kCmdOpenSettings,
    kCmdOpenFolder,
    kCmdExit,
    kCmdPosBase = 100,  // 100..106
};

struct PositionItem {
    const wchar_t* label;
    const wchar_t* iniValue;
};
constexpr PositionItem kPositions[] = {
    {L"Top center", L"top-center"},
    {L"Top left", L"top-left"},
    {L"Top right", L"top-right"},
    {L"Bottom center", L"bottom-center"},
    {L"Bottom left", L"bottom-left"},
    {L"Bottom right", L"bottom-right"},
    {L"Adaptive edge (auto)", L"auto"},
};

inline NOTIFYICONDATAW g_trayData = {};
inline HWND g_trayWindow = nullptr;
inline UINT g_taskbarCreated = 0;
inline HANDLE g_instanceMutex = nullptr;

inline std::wstring ExePath() {
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    return path;
}

inline bool IsStartupEnabled() {
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunKeyPath, 0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS) {
        return false;
    }
    const bool present = RegQueryValueExW(key, kRunValueName, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS;
    RegCloseKey(key);
    return present;
}

inline void SetStartupEnabled(bool enable) {
    HKEY key = nullptr;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kRunKeyPath, 0, nullptr, 0, KEY_SET_VALUE, nullptr, &key, nullptr) !=
        ERROR_SUCCESS) {
        return;
    }
    if (enable) {
        const std::wstring value = L"\"" + ExePath() + L"\"";
        RegSetValueExW(key, kRunValueName, 0, REG_SZ, reinterpret_cast<const BYTE*>(value.c_str()),
                       static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
    } else {
        RegDeleteValueW(key, kRunValueName);
    }
    RegCloseKey(key);
}

// Creates settings.ini with documented, commented-out examples on first use so
// "Open settings file" always has something useful to show.
inline void EnsureSettingsFile() {
    const std::wstring ini = GetIniFilePath();
    if (GetFileAttributesW(ini.c_str()) != INVALID_FILE_ATTRIBUTES) return;
    static const char kTemplate[] =
        "; Dynamic Island settings. Lines starting with ';' are ignored.\r\n"
        "; Restart the app after editing this file.\r\n"
        "[Settings]\r\n"
        "; Appearance.Position = top-center | top-left | top-right | bottom-center | bottom-left | bottom-right | auto\r\n"
        "; Appearance.SizeScale = 1.0\r\n"
        "; Appearance.OffsetX = 0\r\n"
        "; Appearance.OffsetY = 0\r\n"
        "; Themes.ThemePreset = transparent\r\n"
        "; Animations.AnimationStyle = applespring\r\n"
        "; Debug.Log = 1          ; write debug.log next to this file\r\n"
        "\r\n"
        "[Launchers]\r\n"
        "; label | command | aliases (comma separated). Type an alias in Quick Lookup (Ctrl+Alt+Space).\r\n"
        "; 1=Windows Terminal|wt.exe|terminal,wt\r\n"
        "; 2=File Explorer|explorer.exe|explorer,files\r\n"
        "; 3=Web Browser|msedge.exe|browser,edge\r\n";
    HANDLE f = CreateFileW(ini.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (f != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(f, kTemplate, static_cast<DWORD>(sizeof(kTemplate) - 1), &written, nullptr);
        CloseHandle(f);
    }
}

}  // namespace standalone_app

#include "settings_window.h"

namespace standalone_app {


inline std::wstring StorageDir() {
    wchar_t dir[MAX_PATH] = {};
    Wh_GetModStoragePath(dir, MAX_PATH);
    return dir;
}

inline void AddTrayIcon() {
    ZeroMemory(&g_trayData, sizeof(g_trayData));
    g_trayData.cbSize = sizeof(g_trayData);
    g_trayData.hWnd = g_trayWindow;
    g_trayData.uID = kTrayId;
    g_trayData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_trayData.uCallbackMessage = kTrayMessage;
    g_trayData.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wcscpy_s(g_trayData.szTip, L"Dynamic Island");
    Shell_NotifyIconW(NIM_ADD, &g_trayData);
}

inline void RemoveTrayIcon() {
    if (g_trayData.hWnd) {
        Shell_NotifyIconW(NIM_DELETE, &g_trayData);
        g_trayData.hWnd = nullptr;
    }
}

inline void ApplyPosition(const wchar_t* iniValue) {
    WritePrivateProfileStringW(L"Settings", L"Appearance.Position", iniValue, GetIniFilePath().c_str());
    WritePrivateProfileStringW(nullptr, nullptr, nullptr, GetIniFilePath().c_str());  // flush
    LoadSettings();
    g_layoutDirty = true;
}

inline void ShowTrayMenu() {
    HMENU menu = CreatePopupMenu();
    HMENU posMenu = CreatePopupMenu();

    wchar_t current[64] = {};
    GetPrivateProfileStringW(L"Settings", L"Appearance.Position", L"top-center", current, ARRAYSIZE(current),
                             GetIniFilePath().c_str());
    for (UINT i = 0; i < ARRAYSIZE(kPositions); ++i) {
        const bool checked = _wcsicmp(current, kPositions[i].iniValue) == 0;
        AppendMenuW(posMenu, MF_STRING | (checked ? MF_CHECKED : 0), kCmdPosBase + i, kPositions[i].label);
    }

    AppendMenuW(menu, MF_STRING, kCmdToggle, L"Show / hide island");
    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(posMenu), L"Position");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kCmdOpenSettings, L"Settings...");
    AppendMenuW(menu, MF_STRING | (IsStartupEnabled() ? MF_CHECKED : 0), kCmdStartup, L"Run at startup");
    AppendMenuW(menu, MF_STRING, kCmdOpenFolder, L"Open settings folder");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kCmdExit, L"Exit");

    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(g_trayWindow);  // required so the menu closes when clicking elsewhere
    const UINT cmd = static_cast<UINT>(TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY,
                                                      pt.x, pt.y, 0, g_trayWindow, nullptr));
    PostMessageW(g_trayWindow, WM_NULL, 0, 0);
    DestroyMenu(menu);  // also destroys the attached submenu

    if (cmd >= kCmdPosBase && cmd < kCmdPosBase + ARRAYSIZE(kPositions)) {
        ApplyPosition(kPositions[cmd - kCmdPosBase].iniValue);
        return;
    }
    switch (cmd) {
        case kCmdToggle:
            if (g_hwnd) PostMessageW(g_hwnd, WM_HOTKEY, ID_HIDE_SHOW_HOTKEY, 0);
            break;
        case kCmdStartup:
            SetStartupEnabled(!IsStartupEnabled());
            break;
        case kCmdOpenSettings:
            settings_ui::ShowSettingsWindow(g_hwnd);
            break;
        case kCmdOpenFolder:
            ShellExecuteW(nullptr, L"open", StorageDir().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
            break;
        case kCmdExit:
            PostQuitMessage(0);
            break;
        default:
            break;
    }
}

inline LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == kTrayMessage) {
        switch (LOWORD(lParam)) {
            case WM_RBUTTONUP:
            case WM_CONTEXTMENU:
                ShowTrayMenu();
                break;
            case WM_LBUTTONDBLCLK:
                if (g_hwnd) PostMessageW(g_hwnd, WM_HOTKEY, ID_HIDE_SHOW_HOTKEY, 0);
                break;
            default:
                break;
        }
        return 0;
    }
    if (g_taskbarCreated != 0 && msg == g_taskbarCreated) {
        AddTrayIcon();  // Explorer restarted: re-add the icon
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

inline bool CreateTrayWindow(HINSTANCE instance) {
    WNDCLASSW wc = {};
    wc.lpfnWndProc = TrayWndProc;
    wc.hInstance = instance;
    wc.lpszClassName = kTrayClass;
    RegisterClassW(&wc);
    g_taskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");
    g_trayWindow = CreateWindowExW(0, kTrayClass, L"Dynamic Island tray", 0, 0, 0, 0, 0, nullptr, nullptr,
                                   instance, nullptr);
    if (!g_trayWindow) return false;
    AddTrayIcon();
    return true;
}

// Returns false when another instance is already running.
inline bool AcquireSingleInstance() {
    g_instanceMutex = CreateMutexW(nullptr, TRUE, L"Local\\DynamicIsland.Standalone.SingleInstance");
    return g_instanceMutex && GetLastError() != ERROR_ALREADY_EXISTS;
}

}  // namespace standalone_app

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int) {
    if (!standalone_app::AcquireSingleInstance()) {
        return 0;  // already running; the tray icon of the first instance is the entry point
    }

    HDESK hDesk = OpenDesktopW(L"Default", 0, FALSE, GENERIC_ALL);
    if (hDesk) {
        SetThreadDesktop(hDesk);
    }

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    Wh_Log(L"wWinMain: starting Dynamic Island standalone...");
    standalone_app::EnsureSettingsFile();
    LoadSettings();

    if (!StartThreads()) {
        Wh_Log(L"wWinMain: StartThreads failed.");
        MessageBoxW(nullptr, L"Failed to start Dynamic Island.", L"Dynamic Island", MB_ICONERROR);
        CoUninitialize();
        return 1;
    }

    // Wait for the render thread to initialize and create g_hwnd.
    for (int i = 0; i < 50 && !g_hwnd; i++) {
        Sleep(50);
    }
    if (!g_hwnd) {
        Wh_Log(L"wWinMain: Warning: g_hwnd not created within 2.5s.");
    }

    standalone_app::CreateTrayWindow(hInstance);

    // Pump messages for the tray icon until the render thread ends or the user picks Exit.
    for (bool running = true; running && g_renderThread;) {
        const DWORD r = MsgWaitForMultipleObjects(1, &g_renderThread, FALSE, INFINITE, QS_ALLINPUT);
        if (r == WAIT_OBJECT_0) break;  // render thread exited
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    standalone_app::RemoveTrayIcon();
    StopThreads();
    CoUninitialize();
    if (standalone_app::g_instanceMutex) CloseHandle(standalone_app::g_instanceMutex);
    return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR, int nCmdShow) {
    return wWinMain(hInst, hPrev, GetCommandLineW(), nCmdShow);
}
