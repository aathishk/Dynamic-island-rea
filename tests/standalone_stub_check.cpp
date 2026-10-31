// Compile-only check for the standalone entry point + launcher list.
// The island's real globals live in dynamic-island-ultra.cpp (which needs the
// full C++/WinRT toolchain); this file stubs them so the Win32-only pieces
// (standalone_app.h, launchers.h, standalone_shim.h) can be verified quickly.
#ifndef DYNAMIC_ISLAND_STANDALONE
#define DYNAMIC_ISLAND_STANDALONE 1
#endif

#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include "standalone_shim.h"
#include "launchers.h"

HWND g_hwnd = nullptr;
HANDLE g_renderThread = nullptr;
std::atomic<bool> g_layoutDirty = true;
constexpr int ID_HIDE_SHOW_HOTKEY = 1;
constexpr UINT WM_APP_NEW_EVENT = WM_APP + 1;
bool StartThreads() { return true; }
void StopThreads() {}
void LoadSettings() {}
void TriggerNudge() {}

#include "standalone_app.h"

// Unit-style checks for the launcher parser (run with: wine tests.exe, or just
// compile in CI; the checks are also exposed via the --selftest switch).
int RunLauncherSelfTest() {
    DiLauncher l;
    if (!di_launchers_detail::ParseLine(L"Windows Terminal|wt.exe|terminal, WT", l)) return 1;
    if (l.label != L"Windows Terminal" || l.command != L"wt.exe") return 2;
    if (l.aliases.size() != 3 || l.aliases[1] != L"terminal" || l.aliases[2] != L"wt") return 3;
    if (di_launchers_detail::ParseLine(L"broken-line-without-command", l)) return 4;
    const auto defs = di_launchers_detail::Defaults();
    if (!DiFindLauncher(defs, L"  CALC ")) return 5;
    if (DiFindLauncher(defs, L"format c:")) return 6;
    return 0;
}
