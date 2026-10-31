#pragma once
#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstdarg>

// When compiling as a standalone Windows app (outside Windhawk injection)
#ifdef DYNAMIC_ISLAND_STANDALONE

static std::wstring GetIniFilePath() {
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path))) {
        std::wstring dir = std::wstring(path) + L"\\DynamicIsland";
        CreateDirectoryW(dir.c_str(), nullptr);
        return dir + L"\\settings.ini";
    }
    return L"settings.ini";
}

inline BOOL Wh_GetModStoragePath(wchar_t* buffer, size_t size) {
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path))) {
        swprintf_s(buffer, size, L"%s\\DynamicIsland", path);
        CreateDirectoryW(buffer, nullptr);
        return TRUE;
    }
    return FALSE;
}

inline PCWSTR Wh_GetStringSetting(PCWSTR name) {
    std::wstring ini = GetIniFilePath();
    wchar_t buf[512] = {};
    GetPrivateProfileStringW(L"Settings", name, L"", buf, ARRAYSIZE(buf), ini.c_str());
    if (buf[0] != L'\0') {
        return _wcsdup(buf);
    }

    // Sensible defaults when not explicitly set
    if (wcscmp(name, L"Appearance.Position") == 0) return _wcsdup(L"top-center");
    if (wcscmp(name, L"Appearance.ShapeStyle") == 0) return _wcsdup(L"pill");
    if (wcscmp(name, L"Animations.AnimationStyle") == 0) return _wcsdup(L"applespring");
    if (wcscmp(name, L"Appearance.SizeScale") == 0) return _wcsdup(L"1.0");
    if (wcscmp(name, L"Themes.ThemePreset") == 0) return _wcsdup(L"transparent");
    if (wcscmp(name, L"Themes.BackdropMaterial") == 0) return _wcsdup(L"none");
    if (wcscmp(name, L"Themes.AccentColorMode") == 0) return _wcsdup(L"auto");
    if (wcscmp(name, L"Themes.ContourBorderMode") == 0) return _wcsdup(L"auto");
    if (wcscmp(name, L"Animations.TargetFPS") == 0) return _wcsdup(L"auto");
    if (wcscmp(name, L"Animations.AnimationSpeed") == 0) return _wcsdup(L"normal");
    if (wcscmp(name, L"Appearance.ProgressStyle") == 0) return _wcsdup(L"slim");
    if (wcscmp(name, L"Appearance.SpectrumStyle") == 0) return _wcsdup(L"bars");

    return nullptr;
}

inline void Wh_FreeStringSetting(PCWSTR str) {
    if (str) free((void*)str);
}

inline int Wh_GetIntSetting(PCWSTR name) {
    std::wstring ini = GetIniFilePath();
    int val = GetPrivateProfileIntW(L"Settings", name, -999999, ini.c_str());
    if (val != -999999) return val;

    // Sensible numerical & coordinate defaults:
    if (wcscmp(name, L"Appearance.OffsetX") == 0) return 0;
    if (wcscmp(name, L"Appearance.OffsetY") == 0) return 0;
    if (wcscmp(name, L"Appearance.OffsetYExpanded") == 0) return 0;
    if (wcscmp(name, L"Appearance.SeparateExpandedOffsetY") == 0) return 0;
    if (wcscmp(name, L"Appearance.StripMode") == 0) return 0;
    if (wcscmp(name, L"Themes.BackdropTint") == 0) return 30;
    if (wcscmp(name, L"Themes.BackdropFillOpacity") == 0) return 100;
    if (wcscmp(name, L"Themes.AccentBloom") == 0) return 100;
    if (wcscmp(name, L"Themes.TextScale") == 0) return 100;
    if (wcscmp(name, L"Themes.TintIntensity") == 0) return 0;
    if (wcscmp(name, L"Themes.PillOpacity") == 0) return 100;
    if (wcscmp(name, L"Modules.FileTrayMaxItems") == 0) return 10;
    if (wcscmp(name, L"Modules.LyricsOffsetMs") == 0) return 0;
    if (wcscmp(name, L"Modules.LyricsOffsetBluetoothMs") == 0) return 0;
    if (wcscmp(name, L"Modules.DoNotDisturbIndicator") == 0) return 0;
    if (wcscmp(name, L"Behavior.AutoHideIdleSeconds") == 0) return 0;
    if (wcscmp(name, L"Themes.RealLiquidGlass") == 0) return 0;
    if (wcscmp(name, L"Debug.Log") == 0) return 0;

    // All enable toggles default to 1 (enabled)
    return 1;
}

inline int Wh_GetIntValue(PCWSTR name, int defaultValue) {
    std::wstring ini = GetIniFilePath();
    return GetPrivateProfileIntW(L"Values", name, defaultValue, ini.c_str());
}

inline BOOL Wh_SetIntValue(PCWSTR name, int value) {
    std::wstring ini = GetIniFilePath();
    wchar_t buf[32];
    swprintf_s(buf, L"%d", value);
    WritePrivateProfileStringW(L"Values", name, buf, ini.c_str());
    return TRUE;
}

// Logging is opt-in: set "Debug.Log=1" under [Settings] in settings.ini, or the
// environment variable DYNAMIC_ISLAND_DEBUG=1. The log lives next to the
// settings file (%APPDATA%/DynamicIsland/debug.log) and is rotated at 512 KB,
// so it never grows without bound or depends on the working directory.
inline bool DiLogEnabled() {
    wchar_t env[8] = {};
    if (GetEnvironmentVariableW(L"DYNAMIC_ISLAND_DEBUG", env, ARRAYSIZE(env)) > 0 && env[0] == L'1') {
        return true;
    }
    return GetPrivateProfileIntW(L"Settings", L"Debug.Log", 0, GetIniFilePath().c_str()) != 0;
}

inline std::wstring DiLogPath() {
    wchar_t dir[MAX_PATH] = {};
    Wh_GetModStoragePath(dir, MAX_PATH);
    return std::wstring(dir) + L"\\debug.log";
}

inline void Wh_Log(PCWSTR format, ...) {
    if (!DiLogEnabled()) return;

    va_list args;
    va_start(args, format);
    wchar_t buf[1024];
    vswprintf_s(buf, ARRAYSIZE(buf), format, args);
    va_end(args);
    OutputDebugStringW(buf);

    static std::wstring path = DiLogPath();
    WIN32_FILE_ATTRIBUTE_DATA info{};
    if (GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &info) && info.nFileSizeLow > 512 * 1024) {
        MoveFileExW(path.c_str(), (path + L".old").c_str(), MOVEFILE_REPLACE_EXISTING);
    }
    FILE* fp = _wfopen(path.c_str(), L"a, ccs=UTF-8");
    if (fp) {
        fwprintf(fp, L"%s\n", buf);
        fclose(fp);
    }
}

inline void* Wh_SetFunctionHook(void* target, void* hook, void** original) {
    return nullptr;
}

#endif
