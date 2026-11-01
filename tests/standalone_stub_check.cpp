// tests/standalone_stub_check.cpp
// Self-contained unit and regression test runner for Dynamic Edge v1.7.0.
// Directly tests production code and policies: launchers, opacity, bluetooth, shutdown,
// profiles, context engine, notifications, performance, clipboard, focus timer, gestures, and layout.
#ifndef DYNAMIC_ISLAND_STANDALONE
#define DYNAMIC_ISLAND_STANDALONE 1
#endif

#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <string>

#include "standalone_shim.h"
#include "launchers.h"
#include "island_policy.h"
#include "profile_policy.h"
#include "profile_manager.h"
#include "performance_policy.h"
#include "context_policy.h"
#include "notification_policy.h"
#include "notification_history.h"
#include "quick_controls.h"
#include "clipboard_manager.h"
#include "focus_timer.h"
#include "gesture_manager.h"
#include "island_layout.h"

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

    WorkerWaitResult resExited = ClassifyWorkerWait(0x00000000UL /* WAIT_OBJECT_0 */);
    if (resExited != WorkerWaitResult::Exited) return 401;
    if (!ShouldCloseWorkerHandle(resExited)) return 402;

    WorkerWaitResult resTimeout = ClassifyWorkerWait(0x00000102UL /* WAIT_TIMEOUT */);
    if (resTimeout != WorkerWaitResult::TimedOut) return 403;
    if (ShouldCloseWorkerHandle(resTimeout)) return 404;

    WorkerWaitResult resFailed = ClassifyWorkerWait(0xFFFFFFFFUL /* WAIT_FAILED */);
    if (resFailed != WorkerWaitResult::Failed) return 405;
    if (ShouldCloseWorkerHandle(resFailed)) return 406;

    WorkerWaitResult resAbandoned = ClassifyWorkerWait(0x00000080UL /* WAIT_ABANDONED_0 */);
    if (resAbandoned != WorkerWaitResult::Unexpected) return 407;
    if (ShouldCloseWorkerHandle(resAbandoned)) return 408;

    return 0;
}

// Test 6: Production profile system checks (profile_policy.h)
int RunProfilePolicyTest() {
    using namespace island_profile;

    // 1. All 6 built-ins exist and validate
    auto profiles = GetAllBuiltInProfiles();
    if (profiles.size() != 6) return 501;

    // 2. Obsidian has 16% opacity, notch style, balanced performance
    ProfileData obs = CreateBuiltInProfile(BuiltInProfileType::Obsidian);
    if (obs.pillOpacity != 16) return 502;
    if (obs.shapeStyle != L"notch") return 503;
    if (obs.performanceMode != L"balanced") return 504;

    // 3. Frost has optical glass enabled
    ProfileData frost = CreateBuiltInProfile(BuiltInProfileType::Frost);
    if (!frost.realLiquidGlass) return 505;

    // 4. Minimal has reduced motion & battery saver
    ProfileData minimal = CreateBuiltInProfile(BuiltInProfileType::Minimal);
    if (!minimal.reducedMotion || minimal.performanceMode != L"battery_saver") return 506;

    // 5. Name sanitization
    std::wstring safe = SanitizeProfileName(L"  My:Cool*Profile?123  ");
    if (safe != L"MyCoolProfile123") return 507;

    return 0;
}

// Test 7: Production context engine checks (context_policy.h)
int RunContextEngineTest() {
    using namespace island_context;

    ContextEngine engine;
    std::wstring profile;

    // 1. Built-in app rules: Code.exe -> Focus
    ContextMode c1 = engine.Evaluate(L"code.exe", profile);
    if (c1 != ContextMode::Focus) return 601;
    if (profile != L"Studio") return 602;

    // 2. Manual lock overrides app rules
    engine.SetManualLock(ContextMode::Gaming);
    ContextMode c2 = engine.Evaluate(L"code.exe", profile);
    if (c2 != ContextMode::Gaming) return 603;

    // 3. Clear manual lock restores auto evaluation
    engine.ClearManualLock();
    ContextMode c3 = engine.Evaluate(L"spotify.exe", profile);
    if (c3 != ContextMode::Normal) return 604;
    if (profile != L"Aurora") return 605;

    return 0;
}

// Test 8: Production notification policy & quiet hours (notification_policy.h)
int RunNotificationPolicyTest() {
    using namespace island_notification;

    // 1. Midnight wrap-around: 22:00 to 07:00
    if (!IsInQuietHours(23, 22, 7)) return 701;
    if (!IsInQuietHours(2, 22, 7)) return 702;
    if (IsInQuietHours(12, 22, 7)) return 703;

    // 2. Daytime quiet hours: 13:00 to 15:00
    if (!IsInQuietHours(14, 13, 15)) return 704;
    if (IsInQuietHours(16, 13, 15)) return 705;

    // 3. Equal hours -> disabled
    if (IsInQuietHours(10, 10, 10)) return 706;

    // 4. Notification presentation logic
    NotificationPolicySettings settings;
    settings.enabled = true;
    settings.quietHoursEnabled = true;
    settings.quietHoursStart = 22;
    settings.quietHoursEnd = 7;
    settings.allowCriticalInQuietHours = true;
    settings.suppressInFullscreen = true;

    NotificationItem normItem;
    normItem.priority = NotificationPriority::Normal;
    normItem.title = L"Email";

    NotificationItem critItem;
    critItem.priority = NotificationPriority::Critical;
    critItem.title = L"Low Battery";

    // Normal item suppressed during quiet hours (23:00)
    if (ShouldPresentNotification(normItem, settings, false, false, 23)) return 707;
    // Critical item allowed during quiet hours
    if (!ShouldPresentNotification(critItem, settings, false, false, 23)) return 708;

    // Normal item suppressed during fullscreen
    if (ShouldPresentNotification(normItem, settings, true, false, 14)) return 709;

    // 5. Arbitration queue de-duplication
    NotificationQueue q;
    q.Enqueue(normItem, settings);
    bool dupeAdded = q.Enqueue(normItem, settings); // same item immediately
    if (dupeAdded) return 710; // Duplicate should be suppressed!

    return 0;
}

// Test 9: Production performance & power policy checks (performance_policy.h)
int RunPerformancePolicyTest() {
    using namespace island_performance;

    PowerStateInfo acPower;
    acPower.isAcConnected = true;
    acPower.hasBattery = true;
    acPower.batteryPercent = 95;
    acPower.isBatterySaverActive = false;

    PowerStateInfo batteryLow;
    batteryLow.isAcConnected = false;
    batteryLow.hasBattery = true;
    batteryLow.batteryPercent = 15;
    batteryLow.isBatterySaverActive = true;

    // AC Power preserves visual quality
    EffectivePowerMode m1 = DetermineEffectivePowerMode(L"visual_quality", true, acPower);
    if (m1 != EffectivePowerMode::VisualQuality) return 801;
    if (!IsLiveBackdropCaptureAllowed(m1, true)) return 802;

    // Low battery automatically drops to BatterySaver
    EffectivePowerMode m2 = DetermineEffectivePowerMode(L"visual_quality", true, batteryLow);
    if (m2 != EffectivePowerMode::BatterySaver) return 803;
    // Live backdrop capture strictly disallowed in BatterySaver mode
    if (IsLiveBackdropCaptureAllowed(m2, true)) return 804;

    return 0;
}

// Test 10: Production clipboard manager & focus timer checks (clipboard_manager.h, focus_timer.h)
int RunClipboardAndFocusTimerPolicyTest() {
    // 1. Clipboard Manager
    island_clipboard::ClipboardManager cb;
    if (cb.IsEnabled()) return 901; // MUST be disabled by default for privacy
    cb.SetEnabled(true);
    cb.SetMaxItems(3);
    if (cb.Count() != 0) return 902;

    // 2. Focus Timer
    island_focus::FocusConfig cfg;
    cfg.workMinutes = 25;
    cfg.shortBreakMinutes = 5;
    cfg.longBreakMinutes = 15;
    cfg.sessionsBeforeLongBreak = 2;

    island_focus::FocusTimerSession timer(cfg);
    if (timer.GetState() != island_focus::FocusState::Idle) return 903;
    timer.StartWork(25);
    if (timer.GetState() != island_focus::FocusState::WorkSession) return 904;

    timer.Pause();
    if (timer.GetState() != island_focus::FocusState::Paused) return 905;
    timer.Resume();
    if (timer.GetState() != island_focus::FocusState::WorkSession) return 906;

    // Skip completes a session
    timer.Skip();
    if (timer.GetCompletedSessions() != 1) return 907;
    if (timer.GetState() != island_focus::FocusState::ShortBreak) return 908;

    return 0;
}

// Test 11: Production gestures and adaptive layout checks (gesture_manager.h, island_layout.h)
int RunGesturesAndLayoutPolicyTest() {
    using namespace island_gestures;
    using namespace island_layout;

    // 1. Gestures
    GestureManager gm;
    if (gm.GetAction(GestureType::SingleClick) != GestureAction::ExpandCollapse) return 1001;
    if (gm.GetAction(GestureType::DoubleClick) != GestureAction::OpenQuickControls) return 1002;

    // 2. Dangerous script rejection in quick actions
    CustomQuickAction badCmd;
    badCmd.targetPathOrUri = L"C:\\Windows\\System32\\cmd.exe /c calc";
    if (ExecuteSafeQuickAction(badCmd)) return 1003; // MUST reject cmd.exe!

    CustomQuickAction badPs;
    badPs.targetPathOrUri = L"powershell.exe -Command Write-Host evil";
    if (ExecuteSafeQuickAction(badPs)) return 1004; // MUST reject powershell.exe!

    // 3. Adaptive Layout Priority Arbitration
    // QuickControls > Notification > Expanded > Timer > Media > Collapsed
    IslandPresentationState s1 = ArbitratePresentationState(true, true, true, true, true);
    if (s1 != IslandPresentationState::QuickControlGrid) return 1005;

    IslandPresentationState s2 = ArbitratePresentationState(false, true, true, true, true);
    if (s2 != IslandPresentationState::NotificationCard) return 1006;

    IslandPresentationState s3 = ArbitratePresentationState(false, false, true, true, true);
    if (s3 != IslandPresentationState::ExpandedDashboard) return 1007;

    IslandPresentationState s4 = ArbitratePresentationState(false, false, false, true, true);
    if (s4 != IslandPresentationState::CompactTimer) return 1008;

    IslandPresentationState s5 = ArbitratePresentationState(false, false, false, false, true);
    if (s5 != IslandPresentationState::CompactMedia) return 1009;

    IslandPresentationState s6 = ArbitratePresentationState(false, false, false, false, false);
    if (s6 != IslandPresentationState::Collapsed) return 1010;

    return 0;
}

int wmain(int argc, wchar_t** argv) {
    (void)argc;
    (void)argv;

    int r1 = RunLauncherSelfTest();
    if (r1 != 0) { printf("FAIL: Launcher self-test failed: %d\n", r1); return r1; }

    int r2 = RunOpacityPolicyTest();
    if (r2 != 0) { printf("FAIL: Opacity policy self-test failed: %d\n", r2); return r2; }

    int r3 = RunBluetoothPolicyTest();
    if (r3 != 0) { printf("FAIL: Bluetooth policy self-test failed: %d\n", r3); return r3; }

    int r4 = RunDeviceClassificationTest();
    if (r4 != 0) { printf("FAIL: Device classification self-test failed: %d\n", r4); return r4; }

    int r5 = RunWorkerWaitPolicyTest();
    if (r5 != 0) { printf("FAIL: Worker wait shutdown policy self-test failed: %d\n", r5); return r5; }

    int r6 = RunProfilePolicyTest();
    if (r6 != 0) { printf("FAIL: Profile policy self-test failed: %d\n", r6); return r6; }

    int r7 = RunContextEngineTest();
    if (r7 != 0) { printf("FAIL: Context engine self-test failed: %d\n", r7); return r7; }

    int r8 = RunNotificationPolicyTest();
    if (r8 != 0) { printf("FAIL: Notification policy self-test failed: %d\n", r8); return r8; }

    int r9 = RunPerformancePolicyTest();
    if (r9 != 0) { printf("FAIL: Performance policy self-test failed: %d\n", r9); return r9; }

    int r10 = RunClipboardAndFocusTimerPolicyTest();
    if (r10 != 0) { printf("FAIL: Clipboard & focus timer policy self-test failed: %d\n", r10); return r10; }

    int r11 = RunGesturesAndLayoutPolicyTest();
    if (r11 != 0) { printf("FAIL: Gestures & layout policy self-test failed: %d\n", r11); return r11; }

    printf("SUCCESS: All 11 production unit & regression test suites passed (launchers, opacity, bluetooth, classification, shutdown, profiles, context, notifications, performance, clipboard/timer, gestures/layout)!\n");
    return 0;
}
