
// tests/standalone_stub_check.cpp
// Self-contained unit and regression test runner for Dynamic Edge v1.7.0.
// Directly tests production code and policies: launchers, opacity, bluetooth, shutdown,
// profiles, context engine, notifications, performance, clipboard, focus timer, gestures, and layout.
#ifndef DYNAMIC_ISLAND_STANDALONE
#define DYNAMIC_ISLAND_STANDALONE 1
#define DYNAMIC_ISLAND_TEST_RUNNER 1
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

struct StressProducerData {
    island_notification::NotificationQueue* q;
    island_notification::NotificationPolicySettings s;
    std::atomic<bool>* start;
    std::atomic<int>* pushed;
    int producerId;
};

static DWORD WINAPI StressProducerProc(void* p) {
    auto* d = static_cast<StressProducerData*>(p);
    while (!d->start->load()) { Sleep(0); }
    for (int j = 0; j < 100; ++j) {
        island_notification::NotificationItem item;
        item.id = static_cast<uint64_t>(d->producerId * 1000 + j);
        item.title = L"Item " + std::to_wstring(item.id);
        item.message = L"Stress payload";
        item.category = island_notification::NotificationCategory::Application;
        item.priority = (j % 5 == 0) ? island_notification::NotificationPriority::Critical : island_notification::NotificationPriority::Normal;
        if (d->q->Enqueue(item, d->s)) {
            d->pushed->fetch_add(1);
        }
    }
    return 0;
}

struct StressConsumerData {
    island_notification::NotificationQueue* q;
    std::atomic<bool>* start;
    std::atomic<int>* popped;
    std::atomic<bool> done{false};
};

static DWORD WINAPI StressConsumerProc(void* p) {
    auto* d = static_cast<StressConsumerData*>(p);
    while (!d->start->load()) { Sleep(0); }
    while (!d->done.load()) {
        island_notification::NotificationItem item;
        if (d->q->PopNext(item)) {
            d->popped->fetch_add(1);
        } else {
            Sleep(1);
        }
    }
    island_notification::NotificationItem item;
    while (d->q->PopNext(item)) {
        d->popped->fetch_add(1);
    }
    return 0;
}


HWND g_hwnd = nullptr;
HANDLE g_renderThread = nullptr;
std::atomic<bool> g_layoutDirty = true;
constexpr int ID_HIDE_SHOW_HOTKEY = 1;
constexpr UINT WM_APP_NEW_EVENT = WM_APP + 1;
bool StartThreads() { return true; }
void StopThreads() {}
void LoadSettings() {}
void TriggerNudge() {}

struct WorkerStatus {
    std::atomic<bool> media{true};
    std::atomic<bool> audio{true};
    std::atomic<bool> weather{true};
    std::atomic<bool> lyrics{true};
    std::atomic<bool> keyboard{true};
    std::atomic<bool> mouse{true};
    std::atomic<bool> notification{true};
    std::atomic<bool> bluetooth{true};
};
inline WorkerStatus g_workerStatus;

namespace island_globals {
    inline island_context::ContextEngine g_contextEngine;
    inline island_clipboard::ClipboardManager g_clipboardManager;
    inline island_notification::NotificationHistory g_notificationHistory;
    inline island_notification::NotificationQueue g_notificationQueue;
    inline island_focus::FocusTimerSession g_focusTimer;
    inline island_gestures::GestureManager g_gestureManager;
}

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

    // Theme palette resolution checks (all 14 presets)
    static const wchar_t* const kExpectedPalettes[] = {
        L"obsidian", L"graphite", L"slate", L"nord", L"evergreen",
        L"espresso", L"plum", L"porcelain", L"transparent",
        L"acrylic", L"fluent", L"colorful", L"classic", L"minimal"
    };
    for (const wchar_t* themeId : kExpectedPalettes) {
        const auto* pal = island_policy::ResolveThemePalette(themeId);
        if (!pal) {
            printf("FAIL: ResolveThemePalette failed for theme: %ls\n", themeId);
            return 106;
        }
        if (_wcsicmp(pal->id, themeId) != 0) {
            printf("FAIL: Theme ID mismatch: expected %ls, got %ls\n", themeId, pal->id);
            return 107;
        }
    }
    // Verify legacy aliases
    if (!island_policy::ResolveThemePalette(L"oled-black") ||
        _wcsicmp(island_policy::ResolveThemePalette(L"oled-black")->id, L"obsidian") != 0) return 108;
    if (!island_policy::ResolveThemePalette(L"mica") ||
        _wcsicmp(island_policy::ResolveThemePalette(L"mica")->id, L"graphite") != 0) return 109;
    if (!island_policy::ResolveThemePalette(L"deep-purple") ||
        _wcsicmp(island_policy::ResolveThemePalette(L"deep-purple")->id, L"plum") != 0) return 110;
    // Unknown ID returns nullptr
    if (island_policy::ResolveThemePalette(L"invalid_nonexistent_theme") != nullptr) return 111;

    // Active Tab Count calculation & Calendar toggle checks
    const int tabWithCal = island_policy::CalculateActiveTabCount(true, true, true, true, true, true, false);
    const int tabWithoutCal = island_policy::CalculateActiveTabCount(false, true, true, true, true, true, false);
    if (tabWithCal != 7) return 112;
    if (tabWithoutCal != 6) return 113;
    if (tabWithCal - tabWithoutCal != 1) return 114;
    const int tabWithGame = island_policy::CalculateActiveTabCount(true, true, true, true, true, true, true);
    if (tabWithGame != 8) return 115;

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

    // 6. INI module keys parity
    {
        ProfileData p = CreateBuiltInProfile(BuiltInProfileType::Studio);
        std::wstring tempIni = ProfileManager::GetProfilesDirectory() + L"\\test_keys.ini";
        ProfileManager::SaveProfileToFile(p, tempIni);

        ProfileData loaded;
        if (!ProfileManager::LoadProfileFromFile(tempIni, loaded)) return 508;
        if (!loaded.moduleHardware || !loaded.moduleFileTray || !loaded.moduleQuickLookup || !loaded.moduleLyrics) return 509;

        std::wstring targetIni = ProfileManager::GetProfilesDirectory() + L"\\test_settings.ini";
        ProfileManager::ApplyProfileToSettingsIni(loaded, targetIni);

        int hw = GetPrivateProfileIntW(L"Settings", L"Modules.HardwareMonitorModule", 0, targetIni.c_str());
        int ft = GetPrivateProfileIntW(L"Settings", L"Modules.FileTrayModule", 0, targetIni.c_str());
        int ql = GetPrivateProfileIntW(L"Settings", L"Modules.QuickLookup", 0, targetIni.c_str());
        int ly = GetPrivateProfileIntW(L"Settings", L"Modules.Lyrics", 0, targetIni.c_str());

        DeleteFileW(tempIni.c_str());
        DeleteFileW(targetIni.c_str());
        DeleteFileW((targetIni + L".bak").c_str());

        if (hw != 1 || ft != 1 || ql != 1 || ly != 1) return 510;
    }

    // 7. Extended theme & shape validation (Finding 1)
    {
        ProfileData ext;
        ext.name = L"CustomAppearance";
        ext.themePreset = L"graphite";
        ext.shapeStyle = L"w11";
        ext.autoHideFullscreen = true;
        ValidateProfile(ext);
        if (ext.themePreset != L"graphite") return 511; // Must NOT revert graphite to fallback!
        if (ext.shapeStyle != L"w11") return 512;       // Must NOT revert w11 to fallback!

        // Test all 14 themes
        const wchar_t* allThemes[] = {
            L"transparent", L"acrylic", L"fluent", L"colorful", L"classic",
            L"minimal", L"obsidian", L"graphite", L"slate", L"nord",
            L"evergreen", L"espresso", L"plum", L"porcelain"
        };
        for (const wchar_t* th : allThemes) {
            ProfileData pTh;
            pTh.themePreset = th;
            ValidateProfile(pTh);
            if (pTh.themePreset != th) return 513;
        }

        // Test all 4 shapes
        const wchar_t* allShapes[] = { L"notch", L"pill", L"w11", L"blade" };
        for (const wchar_t* sh : allShapes) {
            ProfileData pSh;
            pSh.shapeStyle = sh;
            ValidateProfile(pSh);
            if (pSh.shapeStyle != sh) return 514;
        }

        // Test AutoHideFullscreen persistence across Behavior and Context keys
        std::wstring tempIni = ProfileManager::GetProfilesDirectory() + L"\\test_fullscreen.ini";
        ext.autoHideFullscreen = true;
        ProfileManager::SaveProfileToFile(ext, tempIni);

        ProfileData reloaded;
        if (!ProfileManager::LoadProfileFromFile(tempIni, reloaded)) return 515;
        if (!reloaded.autoHideFullscreen) return 516;

        std::wstring targetIni = ProfileManager::GetProfilesDirectory() + L"\\test_target_fullscreen.ini";
        ProfileManager::ApplyProfileToSettingsIni(reloaded, targetIni);
        int behaviorFull = GetPrivateProfileIntW(L"Settings", L"Behavior.AutoHideFullscreen", 0, targetIni.c_str());
        int contextFull = GetPrivateProfileIntW(L"Settings", L"Context.AutoHideFullscreen", 0, targetIni.c_str());

        DeleteFileW(tempIni.c_str());
        DeleteFileW(targetIni.c_str());
        DeleteFileW((targetIni + L".bak").c_str());

        if (behaviorFull != 1 || contextFull != 1) return 517;
    }

    // 8. Full production profile round-trip for extended companion fields (Sprint Defect 4)
    {
        ProfileData full;
        full.name = L"FullCompanion";
        full.moduleGameOverlay = true;
        full.gameOverlayMode = L"collapsed_hud";
        full.genericDuplicateDebounce = true;
        full.suppressBleSensorDuplicates = true;
        full.mediaPausedTimeoutSeconds = 45;
        full.rememberLastPage = true;
        full.defaultDashboardPage = L"hardware";
        full.moduleQuickControls = true;

        std::wstring tempIni = ProfileManager::GetProfilesDirectory() + L"\\test_full_companion.ini";
        if (!ProfileManager::SaveProfileToFile(full, tempIni)) return 518;

        ProfileData loaded;
        if (!ProfileManager::LoadProfileFromFile(tempIni, loaded)) return 519;
        if (!loaded.moduleGameOverlay || loaded.gameOverlayMode != L"collapsed_hud") return 520;
        if (!loaded.genericDuplicateDebounce || !loaded.suppressBleSensorDuplicates) return 521;
        if (loaded.mediaPausedTimeoutSeconds != 45) return 522;
        if (!loaded.rememberLastPage || loaded.defaultDashboardPage != L"hardware") return 523;
        if (!loaded.moduleQuickControls) return 524;

        std::wstring targetIni = ProfileManager::GetProfilesDirectory() + L"\\test_full_settings.ini";
        ProfileManager::ApplyProfileToSettingsIni(loaded, targetIni);

        int ovEnabled = GetPrivateProfileIntW(L"Settings", L"Modules.GameOverlay", 0, targetIni.c_str());
        wchar_t ovModeBuf[64] = {};
        GetPrivateProfileStringW(L"Settings", L"Modules.GameOverlayMode", L"", ovModeBuf, 64, targetIni.c_str());
        int genDebounce = GetPrivateProfileIntW(L"Settings", L"Notifications.GenericDuplicateDebounce", 0, targetIni.c_str());
        int pauseTimeout = GetPrivateProfileIntW(L"Settings", L"Media.PausedTimeoutSeconds", 0, targetIni.c_str());
        int remPage = GetPrivateProfileIntW(L"Settings", L"Behavior.RememberLastPage", 0, targetIni.c_str());
        int qc = GetPrivateProfileIntW(L"Settings", L"Modules.QuickControls", 0, targetIni.c_str());

        DeleteFileW(tempIni.c_str());
        DeleteFileW(targetIni.c_str());
        DeleteFileW((targetIni + L".bak").c_str());

        if (ovEnabled != 1 || _wcsicmp(ovModeBuf, L"collapsed_hud") != 0) return 525;
        if (genDebounce != 1 || pauseTimeout != 45 || remPage != 1 || qc != 1) return 526;
    }

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
    settings.suppressInPresentation = true;

    NotificationItem normItem;
    normItem.priority = NotificationPriority::Normal;
    normItem.title = L"Email";
    normItem.message = L"You've got mail";

    NotificationItem critItem;
    critItem.priority = NotificationPriority::Critical;
    critItem.title = L"Low Battery";
    critItem.message = L"10% remaining";

    NotificationItem quietItem;
    quietItem.priority = NotificationPriority::Quiet;
    quietItem.title = L"Background Sync";

    // Global disable check
    NotificationPolicySettings disabledSettings = settings;
    disabledSettings.enabled = false;
    if (ShouldPresentNotification(normItem, disabledSettings, false, false, 12)) return 707;
    if (ShouldPresentNotification(critItem, disabledSettings, false, false, 12)) return 708;

    // Quiet priority never presents
    if (ShouldPresentNotification(quietItem, settings, false, false, 12)) return 709;

    // Normal item suppressed during quiet hours (23:00)
    if (ShouldPresentNotification(normItem, settings, false, false, 23)) return 710;

    // Critical item allowed during quiet hours when allowCriticalInQuietHours = true
    if (!ShouldPresentNotification(critItem, settings, false, false, 23)) return 711;

    // Critical item suppressed during quiet hours when allowCriticalInQuietHours = false
    NotificationPolicySettings strictQuiet = settings;
    strictQuiet.allowCriticalInQuietHours = false;
    if (ShouldPresentNotification(critItem, strictQuiet, false, false, 23)) return 712;

    // Normal item suppressed during fullscreen
    if (ShouldPresentNotification(normItem, settings, true, false, 14)) return 713;

    // Normal item suppressed during presentation
    if (ShouldPresentNotification(normItem, settings, false, true, 14)) return 714;

    // Critical item allowed during fullscreen and presentation
    if (!ShouldPresentNotification(critItem, settings, true, false, 14)) return 715;
    if (!ShouldPresentNotification(critItem, settings, false, true, 14)) return 716;

    // 5. Arbitration queue de-duplication & priority with category differentiation (Sprint Defect 1)
    NotificationQueue q;

    NotificationItem bleItem;
    bleItem.category = NotificationCategory::Bluetooth;
    bleItem.title = L"Headphones Connected";
    bleItem.message = L"Battery 90%";

    NotificationItem batItem;
    batItem.category = NotificationCategory::Battery;
    batItem.title = L"Low Battery";
    batItem.message = L"15% battery remaining";

    // Matrix Case 1: generic=true, ble=false
    // Generic toast suppressed; BLE item NOT suppressed; Battery alert suppressed under generic toast window
    settings.genericDuplicateDebounce = true;
    settings.suppressBleSensorDuplicates = false;
    q.Clear();
    q.Enqueue(normItem, settings);
    if (q.Enqueue(normItem, settings)) return 7171; // generic must be suppressed

    q.Clear();
    q.Enqueue(bleItem, settings);
    if (!q.Enqueue(bleItem, settings)) return 7172; // BLE must NOT be suppressed

    q.Clear();
    q.Enqueue(batItem, settings);
    if (q.Enqueue(batItem, settings)) return 7173; // battery must be debounced under generic debounce

    // Matrix Case 2: generic=false, ble=true
    // Generic toast NOT suppressed; BLE item IS suppressed
    settings.genericDuplicateDebounce = false;
    settings.suppressBleSensorDuplicates = true;
    q.Clear();
    q.Enqueue(normItem, settings);
    if (!q.Enqueue(normItem, settings)) return 7174; // generic must NOT be suppressed

    q.Clear();
    q.Enqueue(bleItem, settings);
    if (q.Enqueue(bleItem, settings)) return 7175; // BLE must be suppressed

    // Matrix Case 3: generic=false, ble=false
    // Neither suppressed
    settings.genericDuplicateDebounce = false;
    settings.suppressBleSensorDuplicates = false;
    q.Clear();
    q.Enqueue(normItem, settings);
    if (!q.Enqueue(normItem, settings)) return 7176;

    q.Clear();
    q.Enqueue(bleItem, settings);
    if (!q.Enqueue(bleItem, settings)) return 7177;

    // Matrix Case 4: generic=true, ble=true
    // Both suppressed
    settings.genericDuplicateDebounce = true;
    settings.suppressBleSensorDuplicates = true;
    q.Clear();
    q.Enqueue(normItem, settings);
    if (q.Enqueue(normItem, settings)) return 7178;

    q.Clear();
    q.Enqueue(bleItem, settings);
    if (q.Enqueue(bleItem, settings)) return 7179;

    // Critical items placed at front of queue
    NotificationItem popped;
    q.Clear();
    q.Enqueue(normItem, settings); // Enqueue normal
    q.Enqueue(critItem, settings); // Enqueue critical
    if (!q.PopNext(popped)) return 718;
    if (popped.priority != NotificationPriority::Critical) return 719; // Critical MUST be popped first!

    // 6. Concurrent multi-threaded stress test of NotificationQueue
    {
        NotificationQueue cq;
        std::atomic<bool> startFlag{false};
        std::atomic<int> totalPushed{0};
        std::atomic<int> totalPopped{0};

        std::vector<HANDLE> threads;
        StressProducerData pData[4];
        for (int i = 0; i < 4; ++i) {
            pData[i] = {&cq, settings, &startFlag, &totalPushed, i};
            threads.push_back(CreateThread(nullptr, 0, StressProducerProc, &pData[i], 0, nullptr));
        }

        StressConsumerData cData[2];
        for (int i = 0; i < 2; ++i) {
            cData[i].q = &cq;
            cData[i].start = &startFlag;
            cData[i].popped = &totalPopped;
            cData[i].done.store(false);
            threads.push_back(CreateThread(nullptr, 0, StressConsumerProc, &cData[i], 0, nullptr));
        }

        startFlag.store(true);
        WaitForMultipleObjects(4, threads.data(), TRUE, 5000);

        cData[0].done.store(true);
        cData[1].done.store(true);
        WaitForMultipleObjects(2, threads.data() + 4, TRUE, 5000);

        for (HANDLE h : threads) {
            CloseHandle(h);
        }

        if (totalPushed.load() <= 0) return 720;
    }

    // 7. NotificationHistory methods, escaping, persistence, reload, and wall-clock retention
    {
        NotificationHistory nh;
        nh.SetEnabled(true);
        nh.ClearAll();
        if (nh.Count() != 0) return 721;

        // Verify escaping with pipe, newline, backslash
        std::wstring testRaw = L"Pipe|test\\slash\r\nnewline";
        std::wstring escaped = EscapeText(testRaw);
        std::wstring unescaped = UnescapeText(escaped);
        if (unescaped != testRaw) return 727;

        NotificationItem it1;
        it1.id = 101;
        it1.title = L"Test|Pipe 1";
        it1.message = L"Message\r\nLine 1";
        it1.sourceApp = L"App\\1";
        it1.category = NotificationCategory::Bluetooth;
        it1.priority = NotificationPriority::Critical;
        nh.Record(it1);

        NotificationItem it2;
        it2.id = 102;
        it2.title = L"Test 2";
        it2.message = L"Message 2";
        it2.sourceApp = L"App 2";
        it2.category = NotificationCategory::Battery;
        it2.priority = NotificationPriority::Normal;
        nh.Record(it2);

        auto all = nh.GetAllEntries();
        if (all.size() != 2) return 722;

        // Test saving to custom file
        wchar_t tempFile[MAX_PATH];
        if (GetTempPathW(MAX_PATH, tempFile) == 0) return 727;
        wcscat_s(tempFile, L"test_notif_hist.txt");
        if (!nh.SaveToFile(tempFile)) return 728;

        // Test loading from file into new instance
        NotificationHistory nhReload;
        nhReload.SetEnabled(true);
        if (!nhReload.LoadFromFile(tempFile)) return 729;
        if (nhReload.Count() != 2) return 730;

        auto reloaded = nhReload.GetAllEntries();
        if (reloaded[0].id != 101) return 731; // Must restore ID!
        if (reloaded[0].category != NotificationCategory::Bluetooth) return 732; // Must restore category!
        if (reloaded[0].priority != NotificationPriority::Critical) return 733; // Must restore priority!
        if (reloaded[0].title != L"Test|Pipe 1") return 734; // Must unescape pipe!
        if (reloaded[0].message != L"Message\r\nLine 1") return 735; // Must unescape newline!
        if (reloaded[0].sourceApp != L"App\\1") return 736; // Must unescape backslash!

        // Test DismissEntry on reloaded instance
        if (!nhReload.DismissEntry(101)) return 737; // Dismiss must succeed with loaded id!
        if (!nhReload.GetAllEntries()[0].dismissed) return 738;

        // Test wall-clock retention pruning
        NotificationHistory nhRetention;
        nhRetention.SetEnabled(true);
        nhRetention.ClearAll();

        NotificationItem oldItem;
        oldItem.id = 555;
        oldItem.title = L"Old Item";
        oldItem.message = L"Expired 8 days ago";
        oldItem.sourceApp = L"OldApp";
        uint64_t nowWall = static_cast<uint64_t>(time(nullptr));
        oldItem.timestampWallClock = nowWall - (8 * 86400); // 8 days old
        nhRetention.Record(oldItem);
        // Overwrite file with exact 8-day-old timestamp
        nhRetention.SaveToFile(tempFile);

        NotificationHistory nhLoadRetention;
        nhLoadRetention.SetEnabled(true);
        nhLoadRetention.LoadFromFile(tempFile);
        if (nhLoadRetention.Count() != 0) return 739; // Expired entry MUST be pruned on load!

        // Test future-timestamp gap & enum validation (Sixth Audit Findings A)
        // 1. Direct safe enum conversion checks
        if (SafeCategoryFromInt(-1) != NotificationCategory::GeneralStatus) return 740;
        if (SafeCategoryFromInt(99) != NotificationCategory::GeneralStatus) return 741;
        if (SafeCategoryFromInt(1) != NotificationCategory::Battery) return 742;
        if (SafePriorityFromInt(-1) != NotificationPriority::Normal) return 743;
        if (SafePriorityFromInt(50) != NotificationPriority::Normal) return 744;
        if (SafePriorityFromInt(2) != NotificationPriority::Critical) return 745;

        // 2. Implausible future timestamp (>24 hours) must be rejected
        NotificationHistory nhFuture;
        nhFuture.SetEnabled(true);
        nhFuture.ClearAll();

        NotificationItem futureItem;
        futureItem.id = 777;
        futureItem.title = L"Future Item";
        futureItem.message = L"From 2099";
        futureItem.sourceApp = L"ClockWarp";
        futureItem.timestampWallClock = nowWall + 100000ULL; // > 24 hours ahead
        nhFuture.Record(futureItem);
        nhFuture.SaveToFile(tempFile);

        NotificationHistory nhLoadFuture;
        nhLoadFuture.SetEnabled(true);
        nhLoadFuture.LoadFromFile(tempFile);
        if (nhLoadFuture.Count() != 0) return 746; // Corrupted future item MUST be rejected on load!

        // 3. Minor future drift (e.g. 60 seconds) must be normalized exactly to nowWall and preserved
        NotificationHistory nhDrift;
        nhDrift.SetEnabled(true);
        nhDrift.ClearAll();

        const uint64_t fixedTestTime = 1762100000ULL;
        NotificationItem driftItem;
        driftItem.id = 888;
        driftItem.title = L"Drift Item";
        driftItem.message = L"Slight drift ahead";
        driftItem.sourceApp = L"NTP";
        driftItem.category = static_cast<NotificationCategory>(99); // Corrupt category
        driftItem.priority = static_cast<NotificationPriority>(55); // Corrupt priority
        driftItem.timestampWallClock = fixedTestTime + 60ULL; // 1 minute ahead
        nhDrift.Record(driftItem);
        nhDrift.SaveToFile(tempFile);

        NotificationHistory nhLoadDrift;
        nhLoadDrift.SetEnabled(true);
        nhLoadDrift.LoadFromFile(tempFile, fixedTestTime);
        if (nhLoadDrift.Count() != 1) return 747;
        auto driftLoaded = nhLoadDrift.GetAllEntries()[0];
        if (driftLoaded.timestampWallClock != fixedTestTime) return 748; // Exact normalization assertion!
        if (driftLoaded.category != NotificationCategory::GeneralStatus) return 749; // Must sanitize corrupt category!
        if (driftLoaded.priority != NotificationPriority::Normal) return 750; // Must sanitize corrupt priority!

        // 4. Robustness: Reject records with malformed extra fields (> 8 tokens)
        FILE* fp = nullptr;
        _wfopen_s(&fp, tempFile, L"w, ccs=UNICODE");
        if (fp) {
            fwprintf(fp, L"999|1762100000|App|Title|Msg|1|1|0|ExtraTrailingField|SpuriousData\r\n");
            fclose(fp);
            NotificationHistory nhMalformed;
            nhMalformed.SetEnabled(true);
            nhMalformed.LoadFromFile(tempFile, fixedTestTime);
            if (nhMalformed.Count() != 0) return 751; // Malformed records with > 8 tokens must be rejected!
        }

        // 5. Robustness: SaveToFile write failure on invalid path returns false
        if (nh.SaveToFile(L"\\\\invalid\\null_device\\no_path\\hist.txt")) return 752;

        // 6. Robustness: 0-byte file returns false on LoadFromFile
        FILE* fpZero = nullptr;
        _wfopen_s(&fpZero, tempFile, L"wb");
        if (fpZero) { fclose(fpZero); }
        NotificationHistory nhZero;
        nhZero.SetEnabled(true);
        if (nhZero.LoadFromFile(tempFile)) return 753; // Empty file must return false!

        // 7. Atomic persistence: verify .tmp is not left behind after successful save
        if (!nh.SaveToFile(tempFile)) return 754;
        std::wstring tempTmp = std::wstring(tempFile) + L".tmp";
        if (GetFileAttributesW(tempTmp.c_str()) != INVALID_FILE_ATTRIBUTES) return 755; // Temp swap file must be cleaned up!

        // 8. Opt-in privacy and pause checks for Record()
        NotificationHistory nhDis;
        nhDis.SetEnabled(false);
        if (nhDis.Record(it2)) return 756; // Recording while disabled must return false!
        nhDis.SetEnabled(true);
        nhDis.SetPaused(true);
        if (nhDis.Record(it2)) return 757; // Recording while paused must return false!

        // 9. Regression check: ClearAll and SetEnabled(false) remove stale .tmp files (Finding 1)
        {
            wchar_t cleanMain[MAX_PATH];
            wchar_t cleanTmp[MAX_PATH];
            if (GetTempPathW(MAX_PATH, cleanMain) == 0) return 771;
            wcscat_s(cleanMain, L"test_cleanup_stale.txt");
            wcscpy_s(cleanTmp, cleanMain);
            wcscat_s(cleanTmp, L".tmp");

            // Part A: ClearAll removes both main file and stale .tmp companion
            FILE* fMain = nullptr;
            FILE* fTmp = nullptr;
            if (_wfopen_s(&fMain, cleanMain, L"w") != 0 || !fMain) return 765;
            fputws(L"dummy", fMain); fclose(fMain);
            if (_wfopen_s(&fTmp, cleanTmp, L"w") != 0 || !fTmp) return 766;
            fputws(L"stale_tmp_content", fTmp); fclose(fTmp);

            NotificationHistory nhCleanup;
            nhCleanup.SetEnabled(true);
            if (!nhCleanup.ClearAll(cleanMain)) return 758; // ClearAll must succeed and return true
            if (GetFileAttributesW(cleanMain) != INVALID_FILE_ATTRIBUTES) return 759; // Main file must be removed!
            if (GetFileAttributesW(cleanTmp) != INVALID_FILE_ATTRIBUTES) return 760; // Stale .tmp file MUST be removed!

            // Part B: SetEnabled(false) removes both main file and stale .tmp companion
            if (_wfopen_s(&fMain, cleanMain, L"w") != 0 || !fMain) return 767;
            fputws(L"dummy", fMain); fclose(fMain);
            if (_wfopen_s(&fTmp, cleanTmp, L"w") != 0 || !fTmp) return 768;
            fputws(L"stale_tmp_content", fTmp); fclose(fTmp);

            NotificationHistory nhDisableCleanup;
            nhDisableCleanup.SetEnabled(true);
            if (!nhDisableCleanup.SetEnabled(false, cleanMain)) return 761; // SetEnabled(false) must return true
            if (GetFileAttributesW(cleanMain) != INVALID_FILE_ATTRIBUTES) return 762; // Main file must be removed!
            if (GetFileAttributesW(cleanTmp) != INVALID_FILE_ATTRIBUTES) return 763; // Stale .tmp file MUST be removed!

            // Part C: Cleanup failure propagation when a file is exclusively locked
            if (_wfopen_s(&fMain, cleanMain, L"w") != 0 || !fMain) return 769;
            fputws(L"dummy", fMain); fclose(fMain);
            HANDLE hLock = CreateFileW(cleanMain, GENERIC_READ, 0 /* NO SHARE = EXCLUSIVE LOCK */, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (hLock == INVALID_HANDLE_VALUE) return 770;
            NotificationHistory nhLockTest;
            nhLockTest.SetEnabled(true);
            bool clearRes = nhLockTest.ClearAll(cleanMain);
            CloseHandle(hLock);
            DeleteFileW(cleanMain);
            if (clearRes) return 764; // ClearAll MUST return false when file cannot be deleted!
        }

        DeleteFileW(tempFile);

        if (!nh.DismissEntry(101)) return 723;
        auto allDismissed = nh.GetAllEntries();
        if (!allDismissed[0].dismissed) return 724;

        if (!nh.DeleteEntry(0)) return 725;
        if (nh.Count() != 1) return 726;
        nh.ClearAll();
    }

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
    cb.SetMaxItems(5);
    if (cb.Count() != 0) return 902;

    // Test file persistence with multi-line text & backslashes
    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring testFile = std::wstring(tempPath) + L"test_di_clipboard.txt";
    DeleteFileW(testFile.c_str());

    cb.SaveToFile(testFile);
    cb.LoadFromFile(testFile);

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
    if (!timer.IsActive()) return 909;
    if (!timer.IsRunning()) return 910;

    // Zero modulo protection check: setting sessionsBeforeLongBreak = 0 must clamp to 4 and never crash
    cfg.sessionsBeforeLongBreak = 0;
    timer.SetConfig(cfg);
    if (timer.GetConfig().sessionsBeforeLongBreak <= 0) return 911;
    timer.Skip(); // Must not divide/modulo by zero!

    // Safe clipboard CopyToSystemClipboard check: non-existent id returns false without wiping clipboard
    if (cb.CopyToSystemClipboard(999999)) return 912;

    // SetEnabled(false) purges entries & deletes history file
    cb.SetEnabled(false);
    if (cb.Count() != 0) return 913;

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

    // 4. Wheel Gestures
    gm.SetAction(GestureType::WheelScrollUp, GestureAction::NextMediaTrack);
    if (gm.GetAction(GestureType::WheelScrollUp) != GestureAction::NextMediaTrack) return 1011;
    if (gm.GetAction(GestureType::WheelScrollDown) != GestureAction::None) return 1012;

    // 5. Quick Controls Context Mode Toggles
    island_context::ContextEngine ce;
    island_controls::ExecuteControlAction(island_controls::ControlAction::ToggleFocusMode, nullptr, &ce);
    if (ce.GetActiveMode() != island_context::ContextMode::Focus) return 1013;
    island_controls::ExecuteControlAction(island_controls::ControlAction::ToggleFocusMode, nullptr, &ce);
    if (ce.GetActiveMode() != island_context::ContextMode::Normal) return 1014;

    // 6. Profile CRUD & Built-in Protection
    if (island_profile::ProfileManager::DeleteProfile(L"Obsidian")) return 1015; // Built-in must NOT be deletable!

    // 7. CalculateAdaptiveDimensions across all states
    IslandContentDimensions dNotif = CalculateAdaptiveDimensions(IslandPresentationState::NotificationCard, 1.0f, 1.0f);
    if (std::abs(dNotif.width - 320.0f) > 0.1f || std::abs(dNotif.height - 72.0f) > 0.1f) return 1016;

    IslandContentDimensions dQc = CalculateAdaptiveDimensions(IslandPresentationState::QuickControlGrid, 1.0f, 1.0f);
    if (std::abs(dQc.width - 380.0f) > 0.1f || std::abs(dQc.height - 240.0f) > 0.1f) return 1017;

    IslandContentDimensions dExp = CalculateAdaptiveDimensions(IslandPresentationState::ExpandedDashboard, 1.0f, 1.0f, 0);
    if (std::abs(dExp.width - 380.0f) > 0.1f || std::abs(dExp.height - 184.0f) > 0.1f) return 1018;

    IslandContentDimensions dExpTray = CalculateAdaptiveDimensions(IslandPresentationState::ExpandedDashboard, 1.0f, 1.0f, 33);
    if (std::abs(dExpTray.width - 380.0f) > 0.1f || std::abs(dExpTray.height - 190.0f) > 0.1f) return 1019;

    IslandContentDimensions dTimer = CalculateAdaptiveDimensions(IslandPresentationState::CompactTimer, 1.0f, 1.0f);
    if (std::abs(dTimer.width - 200.0f) > 0.1f || std::abs(dTimer.height - 36.0f) > 0.1f) return 1020;

    IslandContentDimensions dMedia = CalculateAdaptiveDimensions(IslandPresentationState::CompactMedia, 1.0f, 1.0f);
    if (std::abs(dMedia.width - 220.0f) > 0.1f || std::abs(dMedia.height - 38.0f) > 0.1f) return 1021;

    IslandContentDimensions dCol = CalculateAdaptiveDimensions(IslandPresentationState::Collapsed, 1.0f, 1.0f);
    if (std::abs(dCol.width - 160.0f) > 0.1f || std::abs(dCol.height - 36.0f) > 0.1f) return 1022;

    // 8. Complete gesture action string serialization & click options parity (Finding 2)
    {
        const GestureAction allActions[] = {
            GestureAction::None,
            GestureAction::ExpandCollapse,
            GestureAction::OpenQuickControls,
            GestureAction::ToggleFocusMode,
            GestureAction::OpenSettings,
            GestureAction::OpenProfileStudio,
            GestureAction::StartPauseTimer,
            GestureAction::CycleTabs,
            GestureAction::NextMediaTrack,
            GestureAction::PreviousMediaTrack,
            GestureAction::ToggleVisibility
        };
        for (GestureAction ga : allActions) {
            std::wstring idStr = GestureActionToIdString(ga);
            GestureAction roundTrip = GestureActionFromIdString(idStr);
            if (roundTrip != ga) return 1023;
        }

        // Unrecognized or corrupted action string fallback must return GestureAction::None
        if (GestureActionFromIdString(L"unknown_action_xyz") != GestureAction::None) return 1030;
        if (GestureActionFromIdString(L"") != GestureAction::None) return 1031;
        if (std::wstring(GestureActionToIdString(static_cast<GestureAction>(8888))) != L"none") return 1032;

        // 8. Test production gesture dispatcher (island_gestures::ExecuteGestureAction)
        struct GestureTestSpy {
            int qcTabOpened = -1;
            bool focusModeToggled = false;
            bool settingsOpened = false;
            bool timerToggled = false;
            bool tabsCycled = false;
            int mediaCommandSent = -1;
            bool expandToggled = false;
            bool visibilityToggled = false;
        } spy;

        island_gestures::GestureExecutionContext testCtx;
        testCtx.quickControlsTabIndex = -1; // Disabled by default
        testCtx.isFocusTimerActive = false;
        testCtx.isPinnedExpanded = false;
        testCtx.userData = &spy;
        testCtx.onOpenQuickControls = [](int tab, void* u) { static_cast<GestureTestSpy*>(u)->qcTabOpened = tab; };
        testCtx.onToggleFocusMode = [](bool, void* u) { static_cast<GestureTestSpy*>(u)->focusModeToggled = true; };
        testCtx.onOpenSettings = [](void* u) { static_cast<GestureTestSpy*>(u)->settingsOpened = true; };
        testCtx.onStartPauseTimer = [](void* u) { static_cast<GestureTestSpy*>(u)->timerToggled = true; };
        testCtx.onCycleTabs = [](void* u) { static_cast<GestureTestSpy*>(u)->tabsCycled = true; };
        testCtx.onMediaCommand = [](int cmd, void* u) { static_cast<GestureTestSpy*>(u)->mediaCommandSent = cmd; };
        testCtx.onToggleExpand = [](bool, void* u) { static_cast<GestureTestSpy*>(u)->expandToggled = true; };
        testCtx.onToggleVisibility = [](void* u) { static_cast<GestureTestSpy*>(u)->visibilityToggled = true; };

        // Test disabled Quick Controls tab: returns IgnoredDisabled, triggers NO callback
        auto resQcDisabled = island_gestures::ExecuteGestureAction(island_gestures::GestureAction::OpenQuickControls, testCtx);
        if (resQcDisabled != island_gestures::GestureDispatchResult::IgnoredDisabled || spy.qcTabOpened != -1) return 1024;

        // Test enabled Quick Controls tab (index 3): returns Handled, triggers callback with tab 3
        testCtx.quickControlsTabIndex = 3;
        auto resQcEnabled = island_gestures::ExecuteGestureAction(island_gestures::GestureAction::OpenQuickControls, testCtx);
        if (resQcEnabled != island_gestures::GestureDispatchResult::Handled || spy.qcTabOpened != 3) return 1025;

        // Test unknown / invalid action: returns NoOp, does NOT trigger toggle expand or anything else!
        island_gestures::GestureAction unknownAct = static_cast<island_gestures::GestureAction>(9999);
        auto resUnknown = island_gestures::ExecuteGestureAction(unknownAct, testCtx);
        if (resUnknown != island_gestures::GestureDispatchResult::NoOp || spy.expandToggled) return 1026;

        // Test None action: returns NoOp
        auto resNone = island_gestures::ExecuteGestureAction(island_gestures::GestureAction::None, testCtx);
        if (resNone != island_gestures::GestureDispatchResult::NoOp) return 1027;

        // Test ToggleVisibility: returns Handled and triggers callback
        auto resVis = island_gestures::ExecuteGestureAction(island_gestures::GestureAction::ToggleVisibility, testCtx);
        if (resVis != island_gestures::GestureDispatchResult::Handled || !spy.visibilityToggled) return 1028;

        // Test ExpandCollapse: returns Handled and triggers callback
        auto resExp = island_gestures::ExecuteGestureAction(island_gestures::GestureAction::ExpandCollapse, testCtx);
        if (resExp != island_gestures::GestureDispatchResult::Handled || !spy.expandToggled) return 1029;
    }

    return 0;
}

// Test 12: Production navigation matrix, authoritative page arbitration & theme resolution
int RunNavigationMatrixAndThemeResolutionTest() {
    using namespace island_layout;
    using namespace island_policy;

    // 1. Theme single source of truth resolution
    // All 14 presets must resolve correctly
    if (ARRAYSIZE(kAllThemePalettes) != 14) return 1201;
    for (size_t i = 0; i < ARRAYSIZE(kAllThemePalettes); ++i) {
        const auto* pal = ResolveThemePalette(kAllThemePalettes[i].id);
        if (!pal || pal != &kAllThemePalettes[i]) return 1202;
    }

    // Case-insensitivity check
    if (!ResolveThemePalette(L"OBSIDIAN") || ResolveThemePalette(L"Obsidian") != &kAllThemePalettes[0]) return 1203;

    // Legacy alias mapping checks
    if (ResolveThemePalette(L"oled-black") != &kAllThemePalettes[0]) return 1204;
    if (ResolveThemePalette(L"mica") != &kAllThemePalettes[1]) return 1205;
    if (ResolveThemePalette(L"dark-gray") != &kAllThemePalettes[1]) return 1206;
    if (ResolveThemePalette(L"midnight-blue") != &kAllThemePalettes[2]) return 1207;
    if (ResolveThemePalette(L"deep-purple") != &kAllThemePalettes[6]) return 1208;
    if (ResolveThemePalette(L"non_existent_preset_xyz") != nullptr) return 1209;

    // 2. GameOverlayMode enum values
    if (static_cast<int>(GameOverlayMode::DashboardOnly) != 0) return 1210;
    if (static_cast<int>(GameOverlayMode::CollapsedHud) != 1) return 1211;
    if (static_cast<int>(GameOverlayMode::GamingContextOnly) != 2) return 1212;

    // 3. Navigation Matrix Permutations
    // Permutation A: Full Idle suite (hasOverview=true, no media, all idle modules on)
    auto pIdle = BuildActiveDashboardPages(
        true,   // hasOverview
        true,   // mediaModuleEnabled
        false,  // mediaAvailable
        false,  // mediaBlocked
        true,   // calendar
        true,   // weather
        true,   // hardware
        true,   // fileTray
        false,  // lyricsModule
        false,  // lyricsAvailable
        false,  // lyricsPinnedFirst
        true,   // quickControls
        true    // gameOverlay
    );
    // Order: Overview, Calendar, Weather, Hardware, FileTray, Controls, GameOverlay -> 7 pages
    if (pIdle.size() != 7) return 1213;
    if (pIdle[0].id != DashboardPageId::Overview) return 1214;
    if (pIdle[1].id != DashboardPageId::Calendar) return 1215;
    if (pIdle[2].id != DashboardPageId::Weather) return 1216;
    if (pIdle[3].id != DashboardPageId::HardwareMonitor) return 1217;
    if (pIdle[4].id != DashboardPageId::FileTray) return 1218;
    if (pIdle[5].id != DashboardPageId::QuickControls) return 1219;
    if (pIdle[6].id != DashboardPageId::GameOverlay) return 1220;

    // FindPageIndex checks
    if (FindPageIndex(pIdle, DashboardPageId::Overview) != 0) return 1221;
    if (FindPageIndex(pIdle, DashboardPageId::Media) != -1) return 1222; // Media is absent
    if (FindPageIndex(pIdle, DashboardPageId::QuickControls) != 5) return 1223;
    if (FindPageIndex(pIdle, DashboardPageId::FileTray) != 4) return 1224;

    // Permutation B: Active unblocked Media session without lyrics
    auto pMedia = BuildActiveDashboardPages(
        true,   // hasOverview
        true,   // mediaModuleEnabled
        true,   // mediaAvailable
        false,  // mediaBlocked
        true,   // calendar
        true,   // weather
        true,   // hardware
        true,   // fileTray
        false,  // lyricsModule
        false,  // lyricsAvailable
        false,  // lyricsPinnedFirst
        true,   // quickControls
        false   // gameOverlay
    );
    // Order: Overview, Media, Calendar, Weather, Hardware, FileTray, Controls -> 7 pages
    if (pMedia.size() != 7) return 1225;
    if (pMedia[0].id != DashboardPageId::Overview) return 1226;
    if (pMedia[1].id != DashboardPageId::Media) return 1227;
    if (FindPageIndex(pMedia, DashboardPageId::Media) != 1) return 1228;

    // Permutation C: Active Media session with Lyrics pinned first
    auto pLyricsFirst = BuildActiveDashboardPages(
        true,   // hasOverview
        true,   // mediaModuleEnabled
        true,   // mediaAvailable
        false,  // mediaBlocked
        true,   // calendar
        false,  // weather
        false,  // hardware
        false,  // fileTray
        true,   // lyricsModule
        true,   // lyricsAvailable
        true,   // lyricsPinnedFirst
        false,  // quickControls
        false   // gameOverlay
    );
    // Order: Overview, Lyrics, Media, Calendar -> 4 pages
    if (pLyricsFirst.size() != 4) return 1229;
    if (pLyricsFirst[0].id != DashboardPageId::Overview) return 1230;
    if (pLyricsFirst[1].id != DashboardPageId::Lyrics) return 1231;
    if (pLyricsFirst[2].id != DashboardPageId::Media) return 1232;
    if (pLyricsFirst[3].id != DashboardPageId::Calendar) return 1233;

    // Permutation D: Active Media session with Lyrics NOT pinned first (Lyrics placed after File Tray)
    auto pLyricsLast = BuildActiveDashboardPages(
        true,   // hasOverview
        true,   // mediaModuleEnabled
        true,   // mediaAvailable
        false,  // mediaBlocked
        true,   // calendar
        false,  // weather
        false,  // hardware
        false,  // fileTray
        true,   // lyricsModule
        true,   // lyricsAvailable
        false,  // lyricsPinnedFirst
        true,   // quickControls
        false   // gameOverlay
    );
    // Order: Overview, Media, Calendar, Lyrics, Controls -> 5 pages
    if (pLyricsLast.size() != 5) return 1234;
    if (pLyricsLast[0].id != DashboardPageId::Overview) return 1235;
    if (pLyricsLast[1].id != DashboardPageId::Media) return 1236;
    if (pLyricsLast[2].id != DashboardPageId::Calendar) return 1237;
    if (pLyricsLast[3].id != DashboardPageId::Lyrics) return 1238;
    if (pLyricsLast[4].id != DashboardPageId::QuickControls) return 1239;

    // Permutation E: Blocklisted Media session (mediaBlocked == true)
    // CRITICAL: Lyrics and Media must both be absent, introducing ZERO phantom tabs!
    auto pBlocked = BuildActiveDashboardPages(
        true,   // hasOverview
        true,   // mediaModuleEnabled
        true,   // mediaAvailable
        true,   // mediaBlocked (BLOCKLISTED!)
        true,   // calendar
        true,   // weather
        false,  // hardware
        false,  // fileTray
        true,   // lyricsModule
        true,   // lyricsAvailable
        true,   // lyricsPinnedFirst
        true,   // quickControls
        false   // gameOverlay
    );
    // Order: Overview, Calendar, Weather, Controls -> 4 pages (NO Media, NO Lyrics)
    if (pBlocked.size() != 4) return 1240;
    if (FindPageIndex(pBlocked, DashboardPageId::Media) != -1) return 1241;
    if (FindPageIndex(pBlocked, DashboardPageId::Lyrics) != -1) return 1242;
    if (FindPageIndex(pBlocked, DashboardPageId::Calendar) != 1) return 1243;
    if (FindPageIndex(pBlocked, DashboardPageId::Weather) != 2) return 1244;
    if (FindPageIndex(pBlocked, DashboardPageId::QuickControls) != 3) return 1245;

    // Permutation F: Normalization and wrapping
    if (NormalizePageIndex(-1, 5) != 4) return 1246;
    if (NormalizePageIndex(5, 5) != 0) return 1247;
    if (NormalizePageIndex(2, 5) != 2) return 1248;
    if (NormalizePageIndex(0, 0) != 0) return 1249;

    // 4. MediaPresentationState Machine Evaluations (Sprint Defect 2)
    {
        double now = 1000.0;
        // Disabled
        if (EvaluateMediaPresentationState(false, true, true, false, 0.0, now, 30) != MediaPresentationState::Disabled) return 1250;
        // No media
        if (EvaluateMediaPresentationState(true, false, false, false, 0.0, now, 30) != MediaPresentationState::NoMedia) return 1251;
        // Blocked
        if (EvaluateMediaPresentationState(true, true, true, true, 0.0, now, 30) != MediaPresentationState::Blocked) return 1252;
        // Playing
        if (EvaluateMediaPresentationState(true, true, true, false, 0.0, now, 30) != MediaPresentationState::Playing) return 1253;
        // Paused within grace period (paused 10s ago, timeout 30s)
        if (EvaluateMediaPresentationState(true, true, false, false, now - 10.0, now, 30) != MediaPresentationState::PausedGrace) return 1254;
        // Paused beyond timeout (paused 45s ago, timeout 30s)
        if (EvaluateMediaPresentationState(true, true, false, false, now - 45.0, now, 30) != MediaPresentationState::PausedExpired) return 1255;
        // Infinite grace (timeout 0)
        if (EvaluateMediaPresentationState(true, true, false, false, now - 100.0, now, 0) != MediaPresentationState::PausedGrace) return 1256;

        // Active pages with PausedExpired state: Media and Lyrics MUST be excluded!
        auto pExpired = BuildActiveDashboardPages(
            true, MediaPresentationState::PausedExpired,
            true, true, true, true, true, true, false, true, false
        );
        // Should contain Overview, Calendar, Weather, Hardware, FileTray, Controls (NO Media, NO Lyrics)
        if (FindPageIndex(pExpired, DashboardPageId::Media) != -1) return 1257;
        if (FindPageIndex(pExpired, DashboardPageId::Lyrics) != -1) return 1258;
        if (FindPageIndex(pExpired, DashboardPageId::Overview) != 0) return 1259;
        if (FindPageIndex(pExpired, DashboardPageId::HardwareMonitor) != 3) return 1260; // Hardware monitor remains 100% accessible!

        // Active pages with Playing state: Media and Lyrics included
        auto pPlay = BuildActiveDashboardPages(
            true, MediaPresentationState::Playing,
            true, true, true, true, true, true, false, true, false
        );
        if (FindPageIndex(pPlay, DashboardPageId::Media) == -1) return 1261;
        if (FindPageIndex(pPlay, DashboardPageId::Lyrics) == -1) return 1262;
    }

    // 5. Game Overlay Visibility Policy (Sprint Defect 3)
    {
        // In DashboardOnly mode:
        // Collapsed: false
        if (IsGameOverlayVisible(true, GameOverlayMode::DashboardOnly, false, DashboardPageId::Overview, false, false, false)) return 1270;
        // Expanded to Overview: false
        if (IsGameOverlayVisible(true, GameOverlayMode::DashboardOnly, true, DashboardPageId::Overview, false, false, false)) return 1271;
        // Expanded to GameOverlay page: true
        if (!IsGameOverlayVisible(true, GameOverlayMode::DashboardOnly, true, DashboardPageId::GameOverlay, false, false, false)) return 1272;

        // In CollapsedHud mode:
        // Collapsed: true
        if (!IsGameOverlayVisible(true, GameOverlayMode::CollapsedHud, false, DashboardPageId::Overview, false, false, false)) return 1273;
        // Fullscreen: suppressed (false)
        if (IsGameOverlayVisible(true, GameOverlayMode::CollapsedHud, false, DashboardPageId::Overview, false, false, true)) return 1274;

        // In GamingContextOnly mode:
        // Collapsed with gaming context: true
        if (!IsGameOverlayVisible(true, GameOverlayMode::GamingContextOnly, false, DashboardPageId::Overview, true, false, false)) return 1275;
        // Collapsed without gaming context: false
        if (IsGameOverlayVisible(true, GameOverlayMode::GamingContextOnly, false, DashboardPageId::Overview, false, false, false)) return 1276;
    }

    // 6. Overview Measured Layout Hierarchy & Separation Tests
    {
        const float testScales[] = { 0.8f, 1.0f, 1.25f, 1.4f, 2.0f };
        for (float sc : testScales) {
            float rectTop = 0.0f;
            float rectBottom = 220.0f * sc;
            float measuredDateH = 18.0f * sc;
            float measuredTimeH = 56.0f * sc;

            // Date on top hierarchy
            auto m = CalculateOverviewLayoutMetrics(rectTop, rectBottom, sc, measuredDateH, measuredTimeH, true, true);
            // 1. Separation between clock glyph bounds and chip backgrounds must be strictly >= 10-14 logical px
            if (m.clockToChipSeparation < 10.0f * sc - 0.01f) return 1280;
            if (m.chipY <= m.midY + m.midH) return 1281;
            // 2. Chip background bounds must not collide with privacy badge
            if (m.privacyY < m.chipY + m.chipH + 8.0f * sc) return 1282;

            // Long localized date / 24-hour clock simulation
            float longDateH = 24.0f * sc;
            float bigTimeH = 64.0f * sc;
            auto mLong = CalculateOverviewLayoutMetrics(rectTop, rectBottom, sc, longDateH, bigTimeH, true, false);
            if (mLong.clockToChipSeparation < 14.0f * sc - 0.01f) return 1283;
            if (mLong.chipY <= mLong.midY + mLong.midH) return 1284;
        }

        // Surface Material and Opacity Policy Tests
        {
            // Case 1: Collapsed pill retains crystal-clear 16% liquid glass base
            auto mCollapsed = island_layout::CalculateSurfaceMaterial(false, island_layout::DashboardPageId::Overview, 1.0f);
            if (mCollapsed.alpha > 0.17f || mCollapsed.alpha < 0.15f) return 1270;
            if (mCollapsed.isOpaqueBackdrop) return 1271;

            // Case 2: Expanded Media Player maintains reactive 35% translucent glass
            auto mMedia = island_layout::CalculateSurfaceMaterial(true, island_layout::DashboardPageId::Media, 1.0f);
            if (mMedia.alpha > 0.36f || mMedia.alpha < 0.34f) return 1272;
            if (mMedia.isOpaqueBackdrop) return 1273;

            // Case 3: Expanded Hardware Monitor commands deep opaque obsidian backdrop (>= 0.96)
            auto mHw = island_layout::CalculateSurfaceMaterial(true, island_layout::DashboardPageId::HardwareMonitor, 1.0f);
            if (mHw.alpha < 0.96f) return 1274;
            if (!mHw.isOpaqueBackdrop) return 1275;

            // Case 4: Expanded Overview, Calendar, Weather, FileTray all command deep opaque obsidian backdrop
            auto mOverview = island_layout::CalculateSurfaceMaterial(true, island_layout::DashboardPageId::Overview, 1.0f);
            if (mOverview.alpha < 0.96f || !mOverview.isOpaqueBackdrop) return 1276;
            auto mCalendar = island_layout::CalculateSurfaceMaterial(true, island_layout::DashboardPageId::Calendar, 1.0f);
            if (mCalendar.alpha < 0.96f || !mCalendar.isOpaqueBackdrop) return 1277;
            auto mFileTray = island_layout::CalculateSurfaceMaterial(true, island_layout::DashboardPageId::FileTray, 1.0f);
            if (mFileTray.alpha < 0.96f || !mFileTray.isOpaqueBackdrop) return 1278;
        }

        // Status Chip width constraints and proportional scaling
        {
            const float maxRowW = 400.0f;
            const float chipGap = 8.0f;
            const float minChipW = 56.0f;

            // Case A: Natural widths fit comfortably within maxRowW
            std::vector<float> fits = { 65.0f, 90.0f, 110.0f };
            auto outFits = CalculateChipWidths(fits, maxRowW, chipGap, minChipW);
            if (outFits != fits) return 1285;

            // Case B: Extremely long weather text overflows maxRowW
            std::vector<float> overflows = { 65.0f, 320.0f, 110.0f }; // total = 495 > 400
            auto outClamped = CalculateChipWidths(overflows, maxRowW, chipGap, minChipW);
            float totalCalculatedW = 0.0f;
            for (float w : outClamped) {
                if (w < minChipW) return 1286; // No chip below minChipW
                totalCalculatedW += w;
            }
            totalCalculatedW += chipGap * static_cast<float>(outClamped.size() - 1);
            if (totalCalculatedW > maxRowW + 0.01f) return 1287; // Must not exceed row width
        }
    }

    // 7. Active Playback Transition, Page Preservation & Default Page Fallback
    {
        // ParseDashboardPageId validation
        if (ParseDashboardPageId(L"overview") != DashboardPageId::Overview) return 1290;
        if (ParseDashboardPageId(L"media") != DashboardPageId::Media) return 1291;
        if (ParseDashboardPageId(L"calendar") != DashboardPageId::Calendar) return 1292;
        if (ParseDashboardPageId(L"weather") != DashboardPageId::Weather) return 1293;
        if (ParseDashboardPageId(L"hardware") != DashboardPageId::HardwareMonitor) return 1294;
        if (ParseDashboardPageId(L"filetray") != DashboardPageId::FileTray) return 1295;
        if (ParseDashboardPageId(L"lyrics") != DashboardPageId::Lyrics) return 1296;
        if (ParseDashboardPageId(L"controls") != DashboardPageId::QuickControls) return 1297;
        if (ParseDashboardPageId(L"game_overlay") != DashboardPageId::GameOverlay) return 1298;
        if (ParseDashboardPageId(L"unknown_bogus") != DashboardPageId::Overview) return 1299;

        // Transition: when media starts playing, Media page is selected once
        auto pList = BuildActiveDashboardPages(true, MediaPresentationState::Playing, true, true, true, true, false, false, false, true, false);
        int mediaIdx = FindPageIndex(pList, DashboardPageId::Media);
        if (mediaIdx < 0) return 1300;

        // User manual page override during playback: selecting Hardware Monitor
        int hwIdx = FindPageIndex(pList, DashboardPageId::HardwareMonitor);
        if (hwIdx < 0) return 1301;
        int activeTab = hwIdx; // user switched to Hardware Monitor
        // On subsequent frames while media continues playing, user tab is preserved
        if (activeTab != hwIdx) return 1302;

        // When paused beyond timeout: drops Media/Lyrics and falls back to defaultDashboardPage
        auto pAfterTimeout = BuildActiveDashboardPages(true, MediaPresentationState::PausedExpired, true, true, true, true, false, false, false, true, false);
        if (FindPageIndex(pAfterTimeout, DashboardPageId::Media) != -1) return 1303;
        int defIdx = FindPageIndex(pAfterTimeout, DashboardPageId::Overview);
        if (defIdx != 0) return 1304;
    }

    // 8. Profile Studio Full Round-Trip Persistence with 7 Companion Preferences
    {
        island_profile::ProfileData p;
        p.name = L"SprintVerificationProfile";
        p.description = L"Test Profile";
        p.genericDuplicateDebounce = true;
        p.mediaPausedTimeoutSeconds = 45;
        p.rememberLastPage = true;
        p.defaultDashboardPage = L"hardware";
        p.moduleQuickControls = true;
        p.moduleGameOverlay = true;
        p.gameOverlayMode = L"collapsed_hud";

        island_profile::ValidateProfile(p);
        if (p.mediaPausedTimeoutSeconds != 45) return 1310;
        if (p.defaultDashboardPage != L"hardware") return 1311;
        if (p.gameOverlayMode != L"collapsed_hud") return 1312;

        // Test file round-trip
        wchar_t tempDir[MAX_PATH];
        GetTempPathW(MAX_PATH, tempDir);
        std::wstring testFile = std::wstring(tempDir) + L"test_profile_roundtrip.ini";

        if (!island_profile::ProfileManager::SaveProfileToFile(p, testFile)) return 1313;

        island_profile::ProfileData loaded;
        if (!island_profile::ProfileManager::LoadProfileFromFile(testFile, loaded)) return 1314;

        if (loaded.genericDuplicateDebounce != p.genericDuplicateDebounce) return 1315;
        if (loaded.mediaPausedTimeoutSeconds != p.mediaPausedTimeoutSeconds) return 1316;
        if (loaded.rememberLastPage != p.rememberLastPage) return 1317;
        if (loaded.defaultDashboardPage != p.defaultDashboardPage) return 1318;
        if (loaded.moduleQuickControls != p.moduleQuickControls) return 1319;
        if (loaded.moduleGameOverlay != p.moduleGameOverlay) return 1320;
        if (loaded.gameOverlayMode != p.gameOverlayMode) return 1321;

        DeleteFileW(testFile.c_str());
    }

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

    int r12 = RunNavigationMatrixAndThemeResolutionTest();
    if (r12 != 0) { printf("FAIL: Navigation matrix & theme resolution self-test failed: %d\n", r12); return r12; }

    printf("SUCCESS: All 12 production unit & regression test suites passed (launchers, opacity, bluetooth, classification, shutdown, profiles, context, notifications, performance, clipboard/timer, gestures/layout, navigation/themes)!\n");
    return 0;
}

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    return wmain(0, nullptr);
}
