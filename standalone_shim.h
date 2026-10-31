#pragma once
#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

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

inline void Wh_Log(PCWSTR format, ...) {
    va_list args;
    va_start(args, format);
    wchar_t buf[1024];
    vswprintf_s(buf, ARRAYSIZE(buf), format, args);
    va_end(args);
    OutputDebugStringW(buf);

    FILE* fp = _wfopen(L"debug.log", L"a, ccs=UTF-8");
    if (fp) {
        fwprintf(fp, L"%s\n", buf);
        fclose(fp);
    }
}

inline void* Wh_SetFunctionHook(void* target, void* hook, void** original) {
    return nullptr;
}

#endif
