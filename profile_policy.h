// profile_policy.h - Profile definition, built-in presets, and validation policy for Dynamic Edge v1.7.0
// Single source of truth shared between profile_manager, settings_window, and test runners.
#pragma once
#include <algorithm>
#include <string>
#include <vector>
#include <cmath>
#include <cwctype>

namespace island_profile {

constexpr int kProfileSchemaVersion = 170; // v1.7.0

enum class BuiltInProfileType {
    Obsidian,
    Frost,
    Midnight,
    Aurora,
    Minimal,
    Studio,
    Custom,
};

struct ProfileData {
    std::wstring name;
    std::wstring description;
    int schemaVersion = kProfileSchemaVersion;
    bool isBuiltIn = false;

    // Appearance & Layout
    std::wstring themePreset = L"transparent";
    std::wstring position = L"top-center";
    std::wstring shapeStyle = L"notch";
    float sizeScale = 1.0f;
    int pillOpacity = 16;
    int monitorIndex = 0; // 0 = Primary, 1+ = Secondary monitors

    // Animation
    std::wstring animationStyle = L"applespring";
    std::wstring animationSpeed = L"normal";
    bool reducedMotion = false;

    // Visual Effects
    bool realLiquidGlass = false;
    bool dynamicAlbumBloom = true;
    int tintIntensity = 0;

    // Performance & Power Policy
    std::wstring performanceMode = L"balanced"; // battery_saver, balanced, visual_quality, custom
    bool autoBatterySaver = true;

    // Context & Behaviour
    std::wstring contextMode = L"normal"; // normal, focus, gaming, presentation, custom
    bool autoContextSwitching = true;
    bool autoHideFullscreen = true;

    // Notification Policy
    bool quietHoursEnabled = false;
    int quietHoursStart = 22; // 22:00
    int quietHoursEnd = 7;    // 07:00
    bool suppressBleSensorDuplicates = true;
    int notificationDurationMs = 3500;

    // Active Modules
    bool moduleMedia = true;
    bool moduleVolume = true;
    bool moduleBrightness = true;
    bool moduleBluetooth = true;
    bool moduleWeather = true;
    bool moduleCalendar = true;
    bool moduleHardware = true;
    bool moduleFileTray = true;
    bool moduleFocusTimer = true;
    bool moduleClipboard = false; // Opt-in privacy default
    bool moduleQuickLookup = true;
    bool moduleLyrics = true;
};

// Sanitizes a profile name for safe file naming on Windows (letters, digits, spaces, dashes, underscores)
inline std::wstring SanitizeProfileName(const std::wstring& rawName) {
    std::wstring out;
    out.reserve(rawName.size());
    for (wchar_t ch : rawName) {
        if (std::iswalnum(ch) || ch == L' ' || ch == L'-' || ch == L'_') {
            out.push_back(ch);
        }
    }
    // Trim leading/trailing whitespace
    while (!out.empty() && out.front() == L' ') out.erase(out.begin());
    while (!out.empty() && out.back() == L' ') out.pop_back();
    if (out.empty()) {
        return L"CustomProfile";
    }
    if (out.size() > 40) {
        out.resize(40);
    }
    return out;
}

// Validates and bounds all values in a ProfileData struct
inline void ValidateProfile(ProfileData& p) {
    p.name = SanitizeProfileName(p.name);
    p.sizeScale = std::clamp(p.sizeScale, 0.50f, 2.0f);
    p.pillOpacity = std::clamp(p.pillOpacity, 10, 100);
    p.monitorIndex = std::max(0, p.monitorIndex);
    p.quietHoursStart = std::clamp(p.quietHoursStart, 0, 23);
    p.quietHoursEnd = std::clamp(p.quietHoursEnd, 0, 23);
    p.notificationDurationMs = std::clamp(p.notificationDurationMs, 1000, 15000);

    // Validate enum string properties
    auto validOr = [](const std::wstring& val, const std::vector<std::wstring>& allowed, const std::wstring& def) -> std::wstring {
        for (const auto& a : allowed) {
            if (_wcsicmp(val.c_str(), a.c_str()) == 0) return a;
        }
        return def;
    };

    p.themePreset = validOr(p.themePreset, {
        L"transparent", L"acrylic", L"fluent", L"colorful", L"classic", L"minimal", L"obsidian",
        L"graphite", L"slate", L"nord", L"evergreen", L"espresso", L"plum", L"porcelain"
    }, L"transparent");
    p.position = validOr(p.position, {L"top-center", L"top-left", L"top-right", L"bottom-center", L"bottom-left", L"bottom-right", L"auto"}, L"top-center");
    p.shapeStyle = validOr(p.shapeStyle, {L"notch", L"pill", L"w11", L"blade"}, L"notch");
    p.animationStyle = validOr(p.animationStyle, {L"applespring", L"smooth", L"snappy", L"bouncy"}, L"applespring");
    p.animationSpeed = validOr(p.animationSpeed, {L"normal", L"fast", L"relaxed"}, L"normal");
    p.performanceMode = validOr(p.performanceMode, {L"battery_saver", L"balanced", L"visual_quality", L"custom"}, L"balanced");
    p.contextMode = validOr(p.contextMode, {L"normal", L"focus", L"gaming", L"presentation", L"custom"}, L"normal");
}

inline ProfileData CreateBuiltInProfile(BuiltInProfileType type) {
    ProfileData p;
    p.isBuiltIn = true;
    p.schemaVersion = kProfileSchemaVersion;

    switch (type) {
    case BuiltInProfileType::Obsidian:
        p.name = L"Obsidian";
        p.description = L"Deep black, crystal glass base with subtle specular glow; maximum battery efficiency.";
        p.themePreset = L"transparent";
        p.shapeStyle = L"notch";
        p.position = L"top-center";
        p.sizeScale = 1.0f;
        p.pillOpacity = 16; // 16% crystal transparency
        p.animationStyle = L"applespring";
        p.animationSpeed = L"normal";
        p.realLiquidGlass = false;
        p.dynamicAlbumBloom = true;
        p.performanceMode = L"balanced";
        p.moduleFocusTimer = true;
        p.moduleFileTray = true;
        p.moduleHardware = false;
        break;

    case BuiltInProfileType::Frost:
        p.name = L"Frost";
        p.description = L"Cool translucent frosted glass with soft specular contour for light and bright desks.";
        p.themePreset = L"acrylic";
        p.shapeStyle = L"pill";
        p.position = L"top-center";
        p.sizeScale = 1.0f;
        p.pillOpacity = 45;
        p.animationStyle = L"smooth";
        p.animationSpeed = L"normal";
        p.realLiquidGlass = true; // Optical live refraction
        p.dynamicAlbumBloom = false;
        p.performanceMode = L"visual_quality";
        p.moduleFocusTimer = true;
        p.moduleFileTray = true;
        p.moduleHardware = false;
        break;

    case BuiltInProfileType::Midnight:
        p.name = L"Midnight";
        p.description = L"Deep navy base with electric blue media accents designed for night productivity.";
        p.themePreset = L"fluent";
        p.shapeStyle = L"notch";
        p.position = L"top-center";
        p.sizeScale = 1.0f;
        p.pillOpacity = 25;
        p.animationStyle = L"applespring";
        p.animationSpeed = L"normal";
        p.realLiquidGlass = false;
        p.dynamicAlbumBloom = true;
        p.performanceMode = L"balanced";
        p.moduleFocusTimer = true;
        p.moduleFileTray = true;
        p.moduleHardware = false;
        break;

    case BuiltInProfileType::Aurora:
        p.name = L"Aurora";
        p.description = L"Expressive dynamic gradients that fluidly react to album art and system palette.";
        p.themePreset = L"colorful";
        p.shapeStyle = L"pill";
        p.position = L"top-center";
        p.sizeScale = 1.05f;
        p.pillOpacity = 35;
        p.animationStyle = L"bouncy";
        p.animationSpeed = L"normal";
        p.realLiquidGlass = false;
        p.dynamicAlbumBloom = true;
        p.performanceMode = L"visual_quality";
        p.moduleFocusTimer = true;
        p.moduleFileTray = true;
        p.moduleHardware = false;
        break;

    case BuiltInProfileType::Minimal:
        p.name = L"Minimal";
        p.description = L"Ultra-compact, low-motion profile that recedes when idle and preserves maximum laptop battery.";
        p.themePreset = L"transparent";
        p.shapeStyle = L"pill";
        p.position = L"top-center";
        p.sizeScale = 0.80f;
        p.pillOpacity = 12;
        p.animationStyle = L"snappy";
        p.animationSpeed = L"fast";
        p.reducedMotion = true;
        p.realLiquidGlass = false;
        p.dynamicAlbumBloom = false;
        p.performanceMode = L"battery_saver";
        p.moduleHardware = false;
        p.moduleWeather = false;
        p.moduleCalendar = false;
        p.moduleFileTray = false;
        p.moduleFocusTimer = true;
        p.moduleQuickLookup = false;
        p.moduleLyrics = false;
        break;

    case BuiltInProfileType::Studio:
        p.name = L"Studio";
        p.description = L"Information-dense workstation layout with hardware monitor, file shelf, and launcher utilities.";
        p.themePreset = L"obsidian";
        p.shapeStyle = L"notch";
        p.position = L"top-center";
        p.sizeScale = 1.15f;
        p.pillOpacity = 30;
        p.animationStyle = L"smooth";
        p.animationSpeed = L"normal";
        p.realLiquidGlass = false;
        p.dynamicAlbumBloom = true;
        p.performanceMode = L"visual_quality";
        p.moduleHardware = true;
        p.moduleFileTray = true;
        p.moduleFocusTimer = true;
        p.moduleWeather = true;
        p.moduleCalendar = true;
        p.moduleQuickLookup = true;
        p.moduleLyrics = true;
        break;

    default:
        p.name = L"Custom";
        p.description = L"User-configured profile.";
        p.isBuiltIn = false;
        break;
    }

    ValidateProfile(p);
    return p;
}

inline std::vector<ProfileData> GetAllBuiltInProfiles() {
    return {
        CreateBuiltInProfile(BuiltInProfileType::Obsidian),
        CreateBuiltInProfile(BuiltInProfileType::Frost),
        CreateBuiltInProfile(BuiltInProfileType::Midnight),
        CreateBuiltInProfile(BuiltInProfileType::Aurora),
        CreateBuiltInProfile(BuiltInProfileType::Minimal),
        CreateBuiltInProfile(BuiltInProfileType::Studio),
    };
}

} // namespace island_profile
