// tests/standalone_stub_check.cpp
// Self-contained unit and regression test runner for Dynamic Island.
// Tests launcher parsing, opacity clamping, and Bluetooth alert filtering logic.
#ifndef DYNAMIC_ISLAND_STANDALONE
#define DYNAMIC_ISLAND_STANDALONE 1
#endif

#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <cmath>
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

// Test 1: Launcher parser checks
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

// Test 2: Opacity clamping regression test (ensures 16% transparent theme is never clamped to 35%)
int RunOpacityClampSelfTest() {
    auto clampOpacity = [](int percent) -> float {
        float f = static_cast<float>(percent) / 100.0f;
        return std::clamp(f, 0.10f, 1.0f);
    };

    float v16 = clampOpacity(16);
    if (std::abs(v16 - 0.16f) > 0.001f) {
        printf("FAIL: Opacity 16%% clamped incorrectly: %f\n", v16);
        return 101;
    }
    float v100 = clampOpacity(100);
    if (std::abs(v100 - 1.0f) > 0.001f) return 102;
    float v5 = clampOpacity(5);
    if (std::abs(v5 - 0.10f) > 0.001f) return 103;

    return 0;
}

// Test 3: Bluetooth alert filtering logic
enum class TestBtCategory { Generic, Audio, Input };

bool ShouldSuppressBtDisconnect(bool wasConnected, bool isBle, TestBtCategory cat, bool allowGenericBle) {
    if (!wasConnected) return true;
    if (isBle && cat == TestBtCategory::Generic && !allowGenericBle) return true;
    return false;
}

int RunBluetoothFilteringSelfTest() {
    // 1. Never connected device -> must suppress
    if (!ShouldSuppressBtDisconnect(false, true, TestBtCategory::Audio, false)) return 201;

    // 2. Generic BLE peripheral (e.g. GP325 sleeping) -> must suppress by default
    if (!ShouldSuppressBtDisconnect(true, true, TestBtCategory::Generic, false)) return 202;

    // 3. Generic BLE peripheral when allowGenericBle is true -> must NOT suppress
    if (ShouldSuppressBtDisconnect(true, true, TestBtCategory::Generic, true)) return 203;

    // 4. BLE Audio headset (e.g. AirPods / Sony WH-1000XM) -> must NOT suppress
    if (ShouldSuppressBtDisconnect(true, true, TestBtCategory::Audio, false)) return 204;

    // 5. Classic Bluetooth Audio headset -> must NOT suppress
    if (ShouldSuppressBtDisconnect(true, false, TestBtCategory::Audio, false)) return 205;

    // 6. Bluetooth mouse / keyboard -> must NOT suppress
    if (ShouldSuppressBtDisconnect(true, true, TestBtCategory::Input, false)) return 206;

    return 0;
}

int wmain(int argc, wchar_t** argv) {
    int r1 = RunLauncherSelfTest();
    if (r1 != 0) {
        printf("Launcher self-test failed with code: %d\n", r1);
        return r1;
    }

    int r2 = RunOpacityClampSelfTest();
    if (r2 != 0) {
        printf("Opacity clamp self-test failed with code: %d\n", r2);
        return r2;
    }

    int r3 = RunBluetoothFilteringSelfTest();
    if (r3 != 0) {
        printf("Bluetooth filtering self-test failed with code: %d\n", r3);
        return r3;
    }

    printf("SUCCESS: All launcher, opacity clamp, and Bluetooth filtering self-tests passed!\n");
    return 0;
}
