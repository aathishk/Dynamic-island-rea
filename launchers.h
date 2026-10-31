// launchers.h - user-editable quick-launch list for the Quick Lookup panel.
//
// Launchers are read from the [Launchers] section of settings.ini
// (%APPDATA%\DynamicIsland\settings.ini in the standalone build):
//
//   [Launchers]
//   1=Windows Terminal|wt.exe|terminal,wt
//   2=File Explorer|explorer.exe|explorer,files
//   3=Browser|msedge.exe|browser,edge
//
// Format:  label | command | comma-separated aliases (aliases optional)
// Only entries listed here (or the built-in defaults when the section is
// absent) can be launched from the lookup box; arbitrary typed text is never
// passed to ShellExecute.
#pragma once
#include <windows.h>
#include <algorithm>
#include <mutex>
#include <string>
#include <vector>

#ifndef DYNAMIC_ISLAND_STANDALONE
// Windhawk-mod build: there is no settings.ini, so persist next to the mod's
// storage and fall back to defaults.
inline std::wstring GetIniFilePath() {
    wchar_t buf[MAX_PATH] = {};
    if (Wh_GetModStoragePath(buf, MAX_PATH)) {
        return std::wstring(buf) + L"\\settings.ini";
    }
    return L"settings.ini";
}
#endif

struct DiLauncher {
    std::wstring label;
    std::wstring command;
    std::vector<std::wstring> aliases;  // lower-case
};

namespace di_launchers_detail {

inline std::wstring Trim(const std::wstring& s) {
    const size_t a = s.find_first_not_of(L" \t");
    if (a == std::wstring::npos) return L"";
    const size_t b = s.find_last_not_of(L" \t");
    return s.substr(a, b - a + 1);
}

inline std::wstring Lower(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(), [](wchar_t c) {
        return (c >= L'A' && c <= L'Z') ? static_cast<wchar_t>(c + 32) : c;
    });
    return s;
}

// Parses "label|command|alias1,alias2". Returns false for unusable lines.
inline bool ParseLine(const std::wstring& line, DiLauncher& out) {
    std::vector<std::wstring> parts;
    size_t start = 0;
    while (true) {
        const size_t bar = line.find(L'|', start);
        parts.push_back(Trim(line.substr(start, bar == std::wstring::npos ? bar : bar - start)));
        if (bar == std::wstring::npos) break;
        start = bar + 1;
    }
    if (parts.size() < 2 || parts[0].empty() || parts[1].empty()) return false;
    out.label = parts[0];
    out.command = parts[1];
    out.aliases.clear();
    out.aliases.push_back(Lower(out.command));
    if (parts.size() >= 3) {
        size_t s = 0;
        const std::wstring& a = parts[2];
        while (s <= a.size()) {
            const size_t comma = a.find(L',', s);
            std::wstring token = Lower(Trim(a.substr(s, comma == std::wstring::npos ? comma : comma - s)));
            if (!token.empty()) out.aliases.push_back(token);
            if (comma == std::wstring::npos) break;
            s = comma + 1;
        }
    }
    return true;
}

inline std::vector<DiLauncher> Defaults() {
    std::vector<DiLauncher> v;
    DiLauncher l;
    ParseLine(L"Windows Terminal|wt.exe|terminal,wt", l); v.push_back(l);
    ParseLine(L"File Explorer|explorer.exe|explorer,files", l); v.push_back(l);
    ParseLine(L"Web Browser|msedge.exe|browser,edge", l); v.push_back(l);
    ParseLine(L"Chrome|chrome.exe|chrome", l); v.push_back(l);
    ParseLine(L"Command Prompt|cmd.exe|cmd", l); v.push_back(l);
    ParseLine(L"Notepad|notepad.exe|notepad", l); v.push_back(l);
    ParseLine(L"Calculator|calc.exe|calc", l); v.push_back(l);
    return v;
}

}  // namespace di_launchers_detail

// Reads the launcher list (at most 12 entries). Falls back to the built-in
// defaults when settings.ini has no usable [Launchers] entries.
inline std::vector<DiLauncher> DiLoadLaunchers() {
    std::vector<DiLauncher> result;
    const std::wstring ini = GetIniFilePath();
    for (int i = 1; i <= 12; ++i) {
        wchar_t key[16];
        wsprintfW(key, L"%d", i);
        wchar_t buf[512] = {};
        GetPrivateProfileStringW(L"Launchers", key, L"", buf, 512, ini.c_str());
        if (buf[0] == L'\0') continue;
        DiLauncher l;
        if (di_launchers_detail::ParseLine(buf, l)) result.push_back(std::move(l));
    }
    if (result.empty()) return di_launchers_detail::Defaults();
    return result;
}

// Returns the launcher whose command or alias equals `typed` (case-insensitive).
inline const DiLauncher* DiFindLauncher(const std::vector<DiLauncher>& list, const std::wstring& typed) {
    const std::wstring t = di_launchers_detail::Lower(di_launchers_detail::Trim(typed));
    for (const DiLauncher& l : list) {
        for (const std::wstring& a : l.aliases) {
            if (a == t) return &l;
        }
    }
    return nullptr;
}

// Cached variant for per-frame callers (the lookup panel rebuilds its rows often):
// re-reads settings.ini at most once every 3 seconds. Returns a copy.
inline std::vector<DiLauncher> DiCachedLaunchers() {
    static std::mutex mutex;
    static std::vector<DiLauncher> cache;
    static ULONGLONG loadedAt = 0;
    std::lock_guard<std::mutex> lock(mutex);
    const ULONGLONG now = GetTickCount64();
    if (cache.empty() || now - loadedAt > 3000) {
        cache = DiLoadLaunchers();
        loadedAt = now;
    }
    return cache;
}
