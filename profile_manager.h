// profile_manager.h - Profile persistence, atomic switching, and export/import for Dynamic Edge v1.7.0
#pragma once
#include <windows.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include "profile_policy.h"

namespace island_profile {

class ProfileManager {
public:
    static std::wstring GetProfilesDirectory() {
        wchar_t path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path))) {
            std::wstring dir = std::wstring(path) + L"\\DynamicIsland\\profiles";
            CreateDirectoryW((std::wstring(path) + L"\\DynamicIsland").c_str(), nullptr);
            CreateDirectoryW(dir.c_str(), nullptr);
            return dir;
        }
        return L"profiles";
    }

    static std::wstring GetProfileFilePath(const std::wstring& profileName) {
        std::wstring safe = SanitizeProfileName(profileName);
        return GetProfilesDirectory() + L"\\" + safe + L".ini";
    }

    static bool SaveProfileToFile(const ProfileData& profile, const std::wstring& targetPath) {
        ProfileData p = profile;
        ValidateProfile(p);

        // Atomic write via temporary file
        std::wstring tempPath = targetPath + L".tmp";
        auto writeKey = [&](const wchar_t* sec, const wchar_t* key, const std::wstring& val) {
            WritePrivateProfileStringW(sec, key, val.c_str(), tempPath.c_str());
        };
        auto writeInt = [&](const wchar_t* sec, const wchar_t* key, int val) {
            wchar_t b[32];
            swprintf_s(b, L"%d", val);
            WritePrivateProfileStringW(sec, key, b, tempPath.c_str());
        };
        auto writeFloat = [&](const wchar_t* sec, const wchar_t* key, float val) {
            wchar_t b[32];
            swprintf_s(b, L"%.2f", val);
            WritePrivateProfileStringW(sec, key, b, tempPath.c_str());
        };

        // Write header & metadata
        writeInt(L"Profile", L"SchemaVersion", p.schemaVersion);
        writeKey(L"Profile", L"Name", p.name);
        writeKey(L"Profile", L"Description", p.description);
        writeInt(L"Profile", L"IsBuiltIn", p.isBuiltIn ? 1 : 0);

        // Appearance
        writeKey(L"Appearance", L"ThemePreset", p.themePreset);
        writeKey(L"Appearance", L"Position", p.position);
        writeKey(L"Appearance", L"ShapeStyle", p.shapeStyle);
        writeFloat(L"Appearance", L"SizeScale", p.sizeScale);
        writeInt(L"Appearance", L"PillOpacity", p.pillOpacity);
        writeInt(L"Appearance", L"MonitorIndex", p.monitorIndex);

        // Animation
        writeKey(L"Animations", L"AnimationStyle", p.animationStyle);
        writeKey(L"Animations", L"AnimationSpeed", p.animationSpeed);
        writeInt(L"Animations", L"ReducedMotion", p.reducedMotion ? 1 : 0);

        // Effects
        writeInt(L"Themes", L"RealLiquidGlass", p.realLiquidGlass ? 1 : 0);
        writeInt(L"Themes", L"DynamicAlbumBloom", p.dynamicAlbumBloom ? 1 : 0);
        writeInt(L"Themes", L"TintIntensity", p.tintIntensity);

        // Performance & Context
        writeKey(L"Performance", L"PerformanceMode", p.performanceMode);
        writeInt(L"Performance", L"AutoBatterySaver", p.autoBatterySaver ? 1 : 0);
        writeKey(L"Context", L"ContextMode", p.contextMode);
        writeInt(L"Context", L"AutoContextSwitching", p.autoContextSwitching ? 1 : 0);
        writeInt(L"Context", L"AutoHideFullscreen", p.autoHideFullscreen ? 1 : 0);

        // Notifications
        writeInt(L"Notifications", L"QuietHoursEnabled", p.quietHoursEnabled ? 1 : 0);
        writeInt(L"Notifications", L"QuietHoursStart", p.quietHoursStart);
        writeInt(L"Notifications", L"QuietHoursEnd", p.quietHoursEnd);
        writeInt(L"Notifications", L"SuppressBleSensorDuplicates", p.suppressBleSensorDuplicates ? 1 : 0);
        writeInt(L"Notifications", L"NotificationDurationMs", p.notificationDurationMs);

        // Modules
        writeInt(L"Modules", L"Media", p.moduleMedia ? 1 : 0);
        writeInt(L"Modules", L"Volume", p.moduleVolume ? 1 : 0);
        writeInt(L"Modules", L"Brightness", p.moduleBrightness ? 1 : 0);
        writeInt(L"Modules", L"Bluetooth", p.moduleBluetooth ? 1 : 0);
        writeInt(L"Modules", L"Weather", p.moduleWeather ? 1 : 0);
        writeInt(L"Modules", L"Calendar", p.moduleCalendar ? 1 : 0);
        writeInt(L"Modules", L"Hardware", p.moduleHardware ? 1 : 0);
        writeInt(L"Modules", L"FileTray", p.moduleFileTray ? 1 : 0);
        writeInt(L"Modules", L"FocusTimer", p.moduleFocusTimer ? 1 : 0);
        writeInt(L"Modules", L"Clipboard", p.moduleClipboard ? 1 : 0);

        // Flush cache
        WritePrivateProfileStringW(nullptr, nullptr, nullptr, tempPath.c_str());

        // Atomic replace
        if (!MoveFileExW(tempPath.c_str(), targetPath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
            DeleteFileW(tempPath.c_str());
            return false;
        }
        return true;
    }

    static bool LoadProfileFromFile(const std::wstring& filePath, ProfileData& outProfile) {
        if (GetFileAttributesW(filePath.c_str()) == INVALID_FILE_ATTRIBUTES) {
            return false;
        }

        wchar_t buf[512];
        auto readStr = [&](const wchar_t* sec, const wchar_t* key, const wchar_t* def) -> std::wstring {
            buf[0] = L'\0';
            GetPrivateProfileStringW(sec, key, def, buf, ARRAYSIZE(buf), filePath.c_str());
            return buf;
        };
        auto readInt = [&](const wchar_t* sec, const wchar_t* key, int def) -> int {
            return GetPrivateProfileIntW(sec, key, def, filePath.c_str());
        };
        auto readFloat = [&](const wchar_t* sec, const wchar_t* key, float def) -> float {
            std::wstring s = readStr(sec, key, L"");
            if (s.empty()) return def;
            try { return std::stof(s); } catch (...) { return def; }
        };

        outProfile.schemaVersion = readInt(L"Profile", L"SchemaVersion", kProfileSchemaVersion);
        outProfile.name = readStr(L"Profile", L"Name", L"Custom");
        outProfile.description = readStr(L"Profile", L"Description", L"");
        outProfile.isBuiltIn = (readInt(L"Profile", L"IsBuiltIn", 0) != 0);

        // Appearance
        outProfile.themePreset = readStr(L"Appearance", L"ThemePreset", L"transparent");
        outProfile.position = readStr(L"Appearance", L"Position", L"top-center");
        outProfile.shapeStyle = readStr(L"Appearance", L"ShapeStyle", L"notch");
        outProfile.sizeScale = readFloat(L"Appearance", L"SizeScale", 1.0f);
        outProfile.pillOpacity = readInt(L"Appearance", L"PillOpacity", 16);
        outProfile.monitorIndex = readInt(L"Appearance", L"MonitorIndex", 0);

        // Animation
        outProfile.animationStyle = readStr(L"Animations", L"AnimationStyle", L"applespring");
        outProfile.animationSpeed = readStr(L"Animations", L"AnimationSpeed", L"normal");
        outProfile.reducedMotion = (readInt(L"Animations", L"ReducedMotion", 0) != 0);

        // Effects
        outProfile.realLiquidGlass = (readInt(L"Themes", L"RealLiquidGlass", 0) != 0);
        outProfile.dynamicAlbumBloom = (readInt(L"Themes", L"DynamicAlbumBloom", 1) != 0);
        outProfile.tintIntensity = readInt(L"Themes", L"TintIntensity", 0);

        // Performance & Context
        outProfile.performanceMode = readStr(L"Performance", L"PerformanceMode", L"balanced");
        outProfile.autoBatterySaver = (readInt(L"Performance", L"AutoBatterySaver", 1) != 0);
        outProfile.contextMode = readStr(L"Context", L"ContextMode", L"normal");
        outProfile.autoContextSwitching = (readInt(L"Context", L"AutoContextSwitching", 1) != 0);
        outProfile.autoHideFullscreen = (readInt(L"Context", L"AutoHideFullscreen", 1) != 0);

        // Notifications
        outProfile.quietHoursEnabled = (readInt(L"Notifications", L"QuietHoursEnabled", 0) != 0);
        outProfile.quietHoursStart = readInt(L"Notifications", L"QuietHoursStart", 22);
        outProfile.quietHoursEnd = readInt(L"Notifications", L"QuietHoursEnd", 7);
        outProfile.suppressBleSensorDuplicates = (readInt(L"Notifications", L"SuppressBleSensorDuplicates", 1) != 0);
        outProfile.notificationDurationMs = readInt(L"Notifications", L"NotificationDurationMs", 3500);

        // Modules
        outProfile.moduleMedia = (readInt(L"Modules", L"Media", 1) != 0);
        outProfile.moduleVolume = (readInt(L"Modules", L"Volume", 1) != 0);
        outProfile.moduleBrightness = (readInt(L"Modules", L"Brightness", 1) != 0);
        outProfile.moduleBluetooth = (readInt(L"Modules", L"Bluetooth", 1) != 0);
        outProfile.moduleWeather = (readInt(L"Modules", L"Weather", 1) != 0);
        outProfile.moduleCalendar = (readInt(L"Modules", L"Calendar", 1) != 0);
        outProfile.moduleHardware = (readInt(L"Modules", L"Hardware", 1) != 0);
        outProfile.moduleFileTray = (readInt(L"Modules", L"FileTray", 1) != 0);
        outProfile.moduleFocusTimer = (readInt(L"Modules", L"FocusTimer", 1) != 0);
        outProfile.moduleClipboard = (readInt(L"Modules", L"Clipboard", 0) != 0);

        ValidateProfile(outProfile);
        return true;
    }

    static std::vector<ProfileData> EnumerateAllProfiles() {
        std::vector<ProfileData> list = GetAllBuiltInProfiles();

        // Enumerate custom profiles from directory
        std::wstring dir = GetProfilesDirectory();
        WIN32_FIND_DATAW fd;
        HANDLE hFind = FindFirstFileW((dir + L"\\*.ini").c_str(), &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    ProfileData custom;
                    std::wstring fullPath = dir + L"\\" + fd.cFileName;
                    if (LoadProfileFromFile(fullPath, custom)) {
                        // Check if it already exists as a built-in
                        bool exists = false;
                        for (const auto& b : list) {
                            if (_wcsicmp(b.name.c_str(), custom.name.c_str()) == 0) {
                                exists = true;
                                break;
                            }
                        }
                        if (!exists) {
                            list.push_back(custom);
                        }
                    }
                }
            } while (FindNextFileW(hFind, &fd));
            FindClose(hFind);
        }
        return list;
    }

    static bool ApplyProfileToSettingsIni(const ProfileData& profile, const std::wstring& settingsIniPath) {
        ProfileData p = profile;
        ValidateProfile(p);

        // Safe backup before modifying active settings.ini
        std::wstring backupPath = settingsIniPath + L".bak";
        CopyFileW(settingsIniPath.c_str(), backupPath.c_str(), FALSE);

        auto setStr = [&](const wchar_t* key, const std::wstring& val) {
            WritePrivateProfileStringW(L"Settings", key, val.c_str(), settingsIniPath.c_str());
        };
        auto setInt = [&](const wchar_t* key, int val) {
            wchar_t b[32];
            swprintf_s(b, L"%d", val);
            WritePrivateProfileStringW(L"Settings", key, b, settingsIniPath.c_str());
        };
        auto setFloat = [&](const wchar_t* key, float val) {
            wchar_t b[32];
            swprintf_s(b, L"%.2f", val);
            WritePrivateProfileStringW(L"Settings", key, b, settingsIniPath.c_str());
        };

        // Profile metadata
        setStr(L"Profile.ActiveName", p.name);

        // Appearance
        setStr(L"Themes.ThemePreset", p.themePreset);
        setStr(L"Appearance.Position", p.position);
        setStr(L"Appearance.ShapeStyle", p.shapeStyle);
        setFloat(L"Appearance.SizeScale", p.sizeScale);
        setInt(L"Themes.PillOpacity", p.pillOpacity);
        setInt(L"Appearance.MonitorIndex", p.monitorIndex);

        // Animations
        setStr(L"Animations.AnimationStyle", p.animationStyle);
        setStr(L"Animations.AnimationSpeed", p.animationSpeed);
        setInt(L"Animations.ReducedMotion", p.reducedMotion ? 1 : 0);

        // Themes & Glass
        setInt(L"Themes.RealLiquidGlass", p.realLiquidGlass ? 1 : 0);
        setInt(L"Themes.DynamicAlbumBloom", p.dynamicAlbumBloom ? 1 : 0);
        setInt(L"Themes.TintIntensity", p.tintIntensity);

        // Performance & Context
        setStr(L"Performance.PerformanceMode", p.performanceMode);
        setInt(L"Performance.AutoBatterySaver", p.autoBatterySaver ? 1 : 0);
        setStr(L"Context.ContextMode", p.contextMode);
        setInt(L"Context.AutoContextSwitching", p.autoContextSwitching ? 1 : 0);
        setInt(L"Context.AutoHideFullscreen", p.autoHideFullscreen ? 1 : 0);

        // Notifications
        setInt(L"Notifications.QuietHoursEnabled", p.quietHoursEnabled ? 1 : 0);
        setInt(L"Notifications.QuietHoursStart", p.quietHoursStart);
        setInt(L"Notifications.QuietHoursEnd", p.quietHoursEnd);
        setInt(L"Notifications.SuppressBleSensorDuplicates", p.suppressBleSensorDuplicates ? 1 : 0);
        setInt(L"Notifications.NotificationDurationMs", p.notificationDurationMs);

        // Modules
        setInt(L"Modules.Media", p.moduleMedia ? 1 : 0);
        setInt(L"Modules.Volume", p.moduleVolume ? 1 : 0);
        setInt(L"Modules.Brightness", p.moduleBrightness ? 1 : 0);
        setInt(L"Modules.Bluetooth", p.moduleBluetooth ? 1 : 0);
        setInt(L"Modules.Weather", p.moduleWeather ? 1 : 0);
        setInt(L"Modules.Calendar", p.moduleCalendar ? 1 : 0);
        setInt(L"Modules.Hardware", p.moduleHardware ? 1 : 0);
        setInt(L"Modules.FileTray", p.moduleFileTray ? 1 : 0);
        setInt(L"Modules.FocusTimer", p.moduleFocusTimer ? 1 : 0);
        setInt(L"Modules.Clipboard", p.moduleClipboard ? 1 : 0);

        // Flush
        WritePrivateProfileStringW(nullptr, nullptr, nullptr, settingsIniPath.c_str());
        return true;
    }

    static std::wstring GetActiveProfileName(const std::wstring& settingsIniPath) {
        wchar_t buf[128] = {};
        GetPrivateProfileStringW(L"Settings", L"Profile.ActiveName", L"Obsidian", buf, ARRAYSIZE(buf), settingsIniPath.c_str());
        if (buf[0] == L'\0') {
            return L"Obsidian";
        }
        return buf;
    }
};

} // namespace island_profile
