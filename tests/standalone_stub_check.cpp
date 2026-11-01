// tests/standalone_stub_check.cpp
// Self-contained unit and regression test runner for Dynamic Island.
// Directly tests production code from launchers.h and island_policy.h (single source of truth).
#ifndef DYNAMIC_ISLAND_STANDALONE
#define DYNAMIC_ISLAND_STANDALONE 1
#endif

#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "standalone_shim.h"
#include "launchers.h"
#include "island_policy.h"

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

// Test 1: Production launcher parsing & alias resolution checks
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

// Test 2: Production opacity policy checks (island_policy.h)
// Verifies that 16% crystal clear obsidian theme is preserved at 0.16f and alpha byte 41, never clamped to 35%
int RunOpacityPolicyTest() {
    float v16 = island_policy::CalculatePillOpacity(16);
    if (std::abs(v16 - 0.16f) > 0.001f) {
        printf("FAIL: Opacity 16%% clamped incorrectly: %f\n", v16);
        return 101;
    }
    float v100 = island_policy::CalculatePillOpacity(100);
    if (std::abs(v100 - 1.0f) > 0.001f) return 102;

    float v5 = island_policy::CalculatePillOpacity(5);
    if (std::abs(v5 - 0.10f) > 0.001f) return 103;

    // Verify Win32 BLENDFUNCTION alpha byte conversion
    unsigned char alpha16 = island_policy::PillOpacityToAlphaByte(0.16f);
    if (alpha16 != 41) { // 0.16 * 255 = 40.8 -> 41
        printf("FAIL: PillOpacityToAlphaByte(0.16f) gave %u, expected 41\n", alpha16);
        return 104;
    }
    unsigned char alpha100 = island_policy::PillOpacityToAlphaByte(1.0f);
    if (alpha100 != 255) return 105;

    return 0;
}

// Test 3: Production Bluetooth disconnect suppression checks (island_policy.h)
int RunBluetoothPolicyTest() {
    // 1. Device that was never connected -> must suppress disconnect card
    if (!island_policy::ShouldSuppressBluetoothDisconnect(false, true, BluetoothDeviceCategory::Headphones, false)) return 201;

    // 2. Generic BLE peripheral present at launch (wasConnected=true) -> suppressed by default to prevent sleep polling spam
    if (!island_policy::ShouldSuppressBluetoothDisconnect(true, true, BluetoothDeviceCategory::Generic, false)) return 202;

    // 3. Generic BLE peripheral present at launch when user sets Modules.BluetoothAlertOnGenericBle=1 -> must NOT suppress
    if (island_policy::ShouldSuppressBluetoothDisconnect(true, true, BluetoothDeviceCategory::Generic, true)) return 203;

    // 4. BLE Audio headset (e.g. AirPods / Sony WH-1000XM) -> must NOT suppress
    if (island_policy::ShouldSuppressBluetoothDisconnect(true, true, BluetoothDeviceCategory::Headphones, false)) return 204;

    // 5. Classic Bluetooth Audio speaker -> must NOT suppress
    if (island_policy::ShouldSuppressBluetoothDisconnect(true, false, BluetoothDeviceCategory::Speaker, false)) return 205;

    // 6. Bluetooth mouse / keyboard / gamepad -> must NOT suppress
    if (island_policy::ShouldSuppressBluetoothDisconnect(true, true, BluetoothDeviceCategory::Mouse, false)) return 206;
    if (island_policy::ShouldSuppressBluetoothDisconnect(true, true, BluetoothDeviceCategory::Keyboard, false)) return 207;
    if (island_policy::ShouldSuppressBluetoothDisconnect(true, true, BluetoothDeviceCategory::Gamepad, false)) return 208;

    // 7. Explicit gamepad edge cases: disconnected initially -> suppress; classic gamepad -> must NOT suppress
    if (!island_policy::ShouldSuppressBluetoothDisconnect(false, true, BluetoothDeviceCategory::Gamepad, false)) return 209;
    if (island_policy::ShouldSuppressBluetoothDisconnect(true, false, BluetoothDeviceCategory::Gamepad, false)) return 210;

    return 0;
}

// Test 4: Production device classification heuristic checks (island_policy.h)
int RunDeviceClassificationTest() {
    if (island_policy::ClassifyDeviceByName(L"apple airpods pro") != BluetoothDeviceCategory::Headphones) return 301;
    if (island_policy::ClassifyDeviceByName(L"sony wh-1000xm5 headset") != BluetoothDeviceCategory::Headphones) return 302;
    if (island_policy::ClassifyDeviceByName(L"galaxy buds 2") != BluetoothDeviceCategory::Headphones) return 303;
    if (island_policy::ClassifyDeviceByName(L"jbl flip 6 speaker") != BluetoothDeviceCategory::Speaker) return 304;
    if (island_policy::ClassifyDeviceByName(L"logitech mx master 3 mouse") != BluetoothDeviceCategory::Mouse) return 305;
    if (island_policy::ClassifyDeviceByName(L"keychron k2 keyboard") != BluetoothDeviceCategory::Keyboard) return 306;
    if (island_policy::ClassifyDeviceByName(L"xbox wireless controller") != BluetoothDeviceCategory::Gamepad) return 308;
    if (island_policy::ClassifyDeviceByName(L"dualsense wireless controller") != BluetoothDeviceCategory::Gamepad) return 309;
    if (island_policy::ClassifyDeviceByName(L"ps4 dualshock gamepad") != BluetoothDeviceCategory::Gamepad) return 310;
    if (island_policy::ClassifyDeviceByName(L"gp325") != BluetoothDeviceCategory::Generic) return 307;
    return 0;
}

// Test 5: Production thread wait & shutdown classification checks (island_policy.h)
int RunWorkerWaitPolicyTest() {
    using namespace island_policy;

    // 1. WAIT_OBJECT_0 (0x00000000L) -> must classify as Exited and permit closing handle
    WorkerWaitResult resExited = ClassifyWorkerWait(0x00000000UL /* WAIT_OBJECT_0 */);
    if (resExited != WorkerWaitResult::Exited) return 401;
    if (!ShouldCloseWorkerHandle(resExited)) return 402;

    // 2. WAIT_TIMEOUT (0x00000102L / 258L) -> must classify as TimedOut and preserve handle
    WorkerWaitResult resTimeout = ClassifyWorkerWait(0x00000102UL /* WAIT_TIMEOUT */);
    if (resTimeout != WorkerWaitResult::TimedOut) return 403;
    if (ShouldCloseWorkerHandle(resTimeout)) return 404;

    // 3. WAIT_FAILED (0xFFFFFFFFL / (DWORD)-1) -> must classify as Failed and preserve handle
    WorkerWaitResult resFailed = ClassifyWorkerWait(0xFFFFFFFFUL /* WAIT_FAILED */);
    if (resFailed != WorkerWaitResult::Failed) return 405;
    if (ShouldCloseWorkerHandle(resFailed)) return 406;

    // 4. Unexpected / abandoned wait (e.g. WAIT_ABANDONED_0 = 0x80) -> must classify as Unexpected and preserve handle
    WorkerWaitResult resAbandoned = ClassifyWorkerWait(0x00000080UL /* WAIT_ABANDONED_0 */);
    if (resAbandoned != WorkerWaitResult::Unexpected) return 407;
    if (ShouldCloseWorkerHandle(resAbandoned)) return 408;

    return 0;
}

int wmain(int argc, wchar_t** argv) {
    (void)argc;
    (void)argv;

    int r1 = RunLauncherSelfTest();
    if (r1 != 0) {
        printf("FAIL: Launcher self-test failed with code: %d\n", r1);
        return r1;
    }

    int r2 = RunOpacityPolicyTest();
    if (r2 != 0) {
        printf("FAIL: Opacity policy self-test failed with code: %d\n", r2);
        return r2;
    }

    int r3 = RunBluetoothPolicyTest();
    if (r3 != 0) {
        printf("FAIL: Bluetooth policy self-test failed with code: %d\n", r3);
        return r3;
    }

    int r4 = RunDeviceClassificationTest();
    if (r4 != 0) {
        printf("FAIL: Device classification self-test failed with code: %d\n", r4);
        return r4;
    }

    int r5 = RunWorkerWaitPolicyTest();
    if (r5 != 0) {
        printf("FAIL: Worker wait shutdown policy self-test failed with code: %d\n", r5);
        return r5;
    }

    printf("SUCCESS: All 5 production unit & regression test suites passed (launchers, opacity policy, BT policy, gamepad classification, thread wait shutdown policy)!\n");
    return 0;
}
