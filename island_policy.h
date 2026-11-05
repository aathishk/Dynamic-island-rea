// island_policy.h - Core domain policies for Dynamic Island
// Single source of truth shared between dynamic-island-ultra.cpp and unit tests.
#pragma once
#include <algorithm>
#include <string>
#include <cmath>

#ifndef DYNAMIC_ISLAND_BT_CATEGORY_DEFINED
#define DYNAMIC_ISLAND_BT_CATEGORY_DEFINED
enum class BluetoothDeviceCategory {
    Headphones,
    Speaker,
    Mouse,
    Keyboard,
    Gamepad,
    Phone,
    Generic,
};
#endif

namespace island_policy {

// Calculates fractional opacity (0.10f to 1.0f) from an integer percentage.
// Guarantees that the 16% crystal clear transparent theme is preserved at 0.16f.
inline float CalculatePillOpacity(int percent) {
    float f = static_cast<float>(percent) / 100.0f;
    return std::clamp(f, 0.10f, 1.0f);
}

// Converts fractional pill opacity to Win32 BLENDFUNCTION SourceConstantAlpha byte (25 to 255).
inline unsigned char PillOpacityToAlphaByte(float opacity) {
    float clamped = std::clamp(opacity, 0.10f, 1.0f);
    return static_cast<unsigned char>(std::round(clamped * 255.0f));
}

// Determines whether a Bluetooth device disconnect event should be suppressed.
// - wasConnected: true if the device was actively connected prior to disconnect.
// - isBle: true if the device uses Bluetooth Low Energy transport.
// - cat: category classification (Headphones, Speaker, Mouse, Keyboard, Gamepad, Phone, Generic).
// - allowGenericBle: user configuration (Modules.BluetoothAlertOnGenericBle).
inline bool ShouldSuppressBluetoothDisconnect(
    bool wasConnected,
    bool isBle,
    BluetoothDeviceCategory cat,
    bool allowGenericBle) {
    if (!wasConnected) {
        return true;  // Never alert if device was not actively connected
    }
    // Generic BLE peripherals (e.g. GP325) poll and sleep continuously;
    // suppress disconnect cards by default unless user explicitly enabled raw BLE alerts.
    // Audio, input peripherals, and gamepads are never suppressed.
    if (isBle && cat == BluetoothDeviceCategory::Generic && !allowGenericBle) {
        return true;
    }
    return false;
}

// Fast synchronous friendly-name classifier
inline BluetoothDeviceCategory ClassifyDeviceByName(const std::wstring& nameLower) {
    if (nameLower.find(L"headphone") != std::wstring::npos ||
        nameLower.find(L"headset") != std::wstring::npos ||
        nameLower.find(L"earbud") != std::wstring::npos ||
        nameLower.find(L"buds") != std::wstring::npos ||
        nameLower.find(L"airpods") != std::wstring::npos) {
        return BluetoothDeviceCategory::Headphones;
    }
    if (nameLower.find(L"speaker") != std::wstring::npos ||
        nameLower.find(L"soundbar") != std::wstring::npos ||
        nameLower.find(L"boombox") != std::wstring::npos) {
        return BluetoothDeviceCategory::Speaker;
    }
    if (nameLower.find(L"mouse") != std::wstring::npos ||
        nameLower.find(L"trackpad") != std::wstring::npos) {
        return BluetoothDeviceCategory::Mouse;
    }
    if (nameLower.find(L"keyboard") != std::wstring::npos ||
        nameLower.find(L"keypad") != std::wstring::npos) {
        return BluetoothDeviceCategory::Keyboard;
    }
    if (nameLower.find(L"controller") != std::wstring::npos ||
        nameLower.find(L"gamepad") != std::wstring::npos ||
        nameLower.find(L"xbox") != std::wstring::npos ||
        nameLower.find(L"dualsense") != std::wstring::npos ||
        nameLower.find(L"dualshock") != std::wstring::npos) {
        return BluetoothDeviceCategory::Gamepad;
    }
    if (nameLower.find(L"iphone") != std::wstring::npos ||
        nameLower.find(L"phone") != std::wstring::npos ||
        nameLower.find(L"galaxy") != std::wstring::npos ||
        nameLower.find(L"pixel") != std::wstring::npos) {
        return BluetoothDeviceCategory::Phone;
    }
    return BluetoothDeviceCategory::Generic;
}

// Thread shutdown wait classification
enum class WorkerWaitResult {
    Exited,      // WAIT_OBJECT_0: Thread terminated successfully within timeout
    TimedOut,    // WAIT_TIMEOUT: Thread still running after timeout
    Failed,      // WAIT_FAILED: WaitForSingleObject returned WAIT_FAILED
    Unexpected,  // Any other return code (e.g. WAIT_ABANDONED)
};

inline WorkerWaitResult ClassifyWorkerWait(unsigned long waitVal) {
    if (waitVal == 0x00000000UL /* WAIT_OBJECT_0 */) {
        return WorkerWaitResult::Exited;
    }
    if (waitVal == 0x00000102UL /* WAIT_TIMEOUT */) {
        return WorkerWaitResult::TimedOut;
    }
    if (waitVal == 0xFFFFFFFFUL /* WAIT_FAILED */) {
        return WorkerWaitResult::Failed;
    }
    return WorkerWaitResult::Unexpected;
}

inline bool ShouldCloseWorkerHandle(WorkerWaitResult res) {
    return res == WorkerWaitResult::Exited;
}

// ── Shared Themes & Palettes ──────────────────────────────────────────────
struct PaletteDefinition {
    const wchar_t* id;
    const wchar_t* label;
    const wchar_t* bg;
    const wchar_t* fg;
    const wchar_t* sec;
    const wchar_t* border;
};

inline constexpr PaletteDefinition kAllThemePalettes[] = {
    {L"obsidian",    L"Obsidian (Default)",          L"#08080A",   L"#FFFFFF", L"#9B9BA5", L"#1E1E22"},
    {L"graphite",    L"Graphite",                    L"#1C1C1E",   L"#FFFFFF", L"#A8A8AE", L"#323236"},
    {L"slate",       L"Slate",                       L"#111721",   L"#E9EEF6", L"#92A2B8", L"#232C3A"},
    {L"nord",        L"Nord",                        L"#2E3440",   L"#ECEFF4", L"#A7B0C0", L"#3B4252"},
    {L"evergreen",   L"Evergreen",                   L"#0C1512",   L"#E4F1EA", L"#8CAE9D", L"#1A2A23"},
    {L"espresso",    L"Espresso",                    L"#1A1512",   L"#F6EEE7", L"#B7A395", L"#2E2420"},
    {L"plum",        L"Plum",                        L"#16111C",   L"#F1EAF6", L"#AC9BB9", L"#281F33"},
    {L"porcelain",   L"Porcelain (Light)",           L"#F5F6F8",   L"#14161A", L"#5B6270", L"#D8DBE1"},
    {L"transparent", L"Transparent Liquid Glass",    L"#08080A26", L"#FFFFFF", L"#C8C8D0", L"#FFFFFF20"},
    {L"acrylic",     L"Acrylic Glass (Legacy)",      L"#1A1E24D0", L"#F0F3F8", L"#94A3B8", L"#334155"},
    {L"fluent",      L"Fluent Mica (Legacy)",        L"#20232A",   L"#FFFFFF", L"#9DA5B4", L"#2D323F"},
    {L"colorful",    L"Colorful Vibrant (Legacy)",    L"#180E29",   L"#FAF5FF", L"#C084FC", L"#3B184C"},
    {L"classic",     L"Classic Dark (Legacy)",       L"#181818",   L"#F0F0F0", L"#9E9E9E", L"#2C2C2C"},
    {L"minimal",     L"Minimal Monochrome (Legacy)", L"#000000",   L"#FFFFFF", L"#808080", L"#1F1F1F"},
};

inline const PaletteDefinition* ResolveThemePalette(std::wstring_view id) {
    auto iequals = [](std::wstring_view a, std::wstring_view b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (towlower(a[i]) != towlower(b[i])) return false;
        }
        return true;
    };
    for (const auto& pal : kAllThemePalettes) {
        if (iequals(id, pal.id)) return &pal;
    }
    if (iequals(id, L"oled-black")) return &kAllThemePalettes[0]; // obsidian
    if (iequals(id, L"mica") || iequals(id, L"dark-gray")) return &kAllThemePalettes[1]; // graphite
    if (iequals(id, L"midnight-blue")) return &kAllThemePalettes[2]; // slate
    if (iequals(id, L"deep-purple")) return &kAllThemePalettes[6]; // plum
    return nullptr;
}

// ── Shared Accent Color Presets ──────────────────────────────────────────
struct AccentPresetDefinition {
    const wchar_t* label;
    const wchar_t* val;
};

inline constexpr AccentPresetDefinition kAllAccentPresets[] = {
    {L"Auto (Match Album Art / System)", L"auto"},
    {L"Windows System (Device Color)",   L"system"},
    {L"Electric Cyan (#4CC9F0)",         L"#4CC9F0"},
    {L"Crimson Red (#E81123)",           L"#E81123"},
    {L"Vibrant Orange (#FF5722)",        L"#FF5722"},
    {L"Amber Gold (#FFB900)",            L"#FFB900"},
    {L"Emerald Green (#107C41)",         L"#107C41"},
    {L"Royal Purple (#8E24AA)",          L"#8E24AA"},
    {L"Hot Pink (#FF2D55)",              L"#FF2D55"},
    {L"Pure White (#FFFFFF)",            L"#FFFFFF"},
};

inline const AccentPresetDefinition* ResolveAccentPreset(std::wstring_view val) {
    auto iequals = [](std::wstring_view a, std::wstring_view b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (towlower(a[i]) != towlower(b[i])) return false;
        }
        return true;
    };
    for (const auto& preset : kAllAccentPresets) {
        if (iequals(val, preset.val)) return &preset;
    }
    return nullptr;
}

// ── Active Dashboard Tab Calculation ──────────────────────────────────────
inline int CalculateActiveTabCount(bool hasCalendar, bool hasWeather, bool hasHardware, bool hasFileTray, bool hasLyrics, bool hasQuickControls, bool hasGameOverlay = false) {
    int count = 1; // Primary tab (Clock or Media)
    if (hasCalendar) ++count;
    if (hasWeather) ++count;
    if (hasHardware) ++count;
    if (hasFileTray) ++count;
    if (hasLyrics) ++count;
    if (hasQuickControls) ++count;
    if (hasGameOverlay) ++count;
    return count;
}

}  // namespace island_policy
