// settings_window.h - Streamlined, Native Dark-Themed Settings UI for Dynamic Island
#pragma once
#include <windows.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <uxtheme.h>
#include <string>
#include <vector>
#include <cmath>
#include "profile_policy.h"
#include "profile_manager.h"
#include "clipboard_manager.h"
#include "island_layout.h"

namespace standalone_app {
    bool IsStartupEnabled();
    void SetStartupEnabled(bool);
    void EnsureSettingsFile();
}

namespace settings_ui {

inline HWND g_hSettingsWnd = nullptr;
inline HWND g_hOnboardingWnd = nullptr;

struct SettingOption {
    const wchar_t* label;
    const wchar_t* val;
};

// Available configuration choices
inline const SettingOption kPosOptions[] = {
    {L"Top Center (Default)", L"top-center"},
    {L"Top Left", L"top-left"},
    {L"Top Right", L"top-right"},
    {L"Bottom Center", L"bottom-center"},
    {L"Bottom Left", L"bottom-left"},
    {L"Bottom Right", L"bottom-right"},
    {L"Adaptive Edge Blade (Auto)", L"auto"}
};

inline const SettingOption kShapeOptions[] = {
    {L"iPhone Pill Capsule (Floating)", L"pill"},
    {L"macOS Notch (Flush Bezel)", L"notch"},
    {L"Windows 11 Flyout (8px Fluent)", L"w11"},
    {L"Edge Blade (Cyber Visor)", L"blade"}
};

inline const SettingOption kAccentColorOptions[] = {
    {L"Auto (Match Album Art / System)", L"auto"},
    {L"Windows System (Device Color)",   L"system"},
    {L"Electric Cyan (#4CC9F0)",         L"#4CC9F0"},
    {L"Crimson Red (#E81123)",           L"#E81123"},
    {L"Vibrant Orange (#FF5722)",        L"#FF5722"},
    {L"Amber Gold (#FFB900)",            L"#FFB900"},
    {L"Emerald Green (#107C41)",         L"#107C41"},
    {L"Royal Purple (#8E24AA)",          L"#8E24AA"},
    {L"Hot Pink (#FF2D55)",              L"#FF2D55"},
    {L"Pure White (#FFFFFF)",            L"#FFFFFF"}
};

inline const SettingOption kThemeOptions[] = {
    {L"Obsidian (OLED Black)", L"obsidian"},
    {L"Transparent Liquid Glass", L"transparent"},
    {L"Graphite (Fluent Grey)", L"graphite"},
    {L"Slate (Midnight Blue)", L"slate"},
    {L"Nord (Arctic Slate)", L"nord"},
    {L"Evergreen (Forest)", L"evergreen"},
    {L"Espresso (Warm Bronze)", L"espresso"},
    {L"Plum (Deep Purple)", L"plum"},
    {L"Porcelain (Clean Light)", L"porcelain"}
};

inline const SettingOption kAnimOptions[] = {
    {L"AppleSpring (Fluid Dynamics)", L"applespring"},
    {L"Smooth (Ease In-Out)", L"smooth"},
    {L"Snappy (Fast Response)", L"snappy"},
    {L"Bouncy (Elastic)", L"bouncy"}
};

inline const SettingOption kScaleOptions[] = {
    {L"80% (Compact)", L"0.8"},
    {L"90% (Small)", L"0.9"},
    {L"100% (Standard)", L"1.0"},
    {L"110% (Medium)", L"1.1"},
    {L"125% (Large)", L"1.25"},
    {L"140% (Extra Large)", L"1.4"}
};

inline const SettingOption kHoverModeOptions[] = {
    {L"Click to Expand (Recommended)", L"0"},
    {L"Expand on Mouse Hover", L"1"}
};

inline const SettingOption kDefaultDashOptions[] = {
    {L"Overview (Clock & Status Chips)", L"overview"},
    {L"Now Playing Media Player", L"media"},
    {L"Desktop Quick Controls", L"controls"},
    {L"System Hardware Monitor", L"hardware"},
    {L"Live Weather & Forecast", L"weather"},
    {L"Calendar & Upcoming Schedule", L"calendar"}
};

// Control IDs
enum CtrlId : int {
    ID_TAB = 2000,

    // Page 0: Appearance
    ID_COMBO_SHAPE,
    ID_COMBO_THEME,
    ID_COMBO_ACCENT,
    ID_COMBO_SCALE,
    ID_COMBO_ANIM,
    ID_CHK_REAL_GLASS,
    ID_CHK_BLOOM,
    ID_CHK_SHADOW,
    ID_CHK_BORDER,

    // Page 1: Behavior
    ID_COMBO_HOVER_MODE,
    ID_COMBO_POS,
    ID_COMBO_DEFAULT_DASH,
    ID_CHK_STARTUP,
    ID_CHK_AUTOHIDE_FS,
    ID_CHK_REMEMBER_LAST_PAGE,
    ID_CHK_NOTIF_FS_SUPPRESS,
    ID_CHK_NOTIF_ENABLED,

    // Page 2: Features & Widgets
    ID_CHK_MOD_MEDIA,
    ID_CHK_MOD_LYRICS,
    ID_CHK_MOD_QUICK_CONTROLS,
    ID_CHK_MOD_HARDWARE,
    ID_CHK_MOD_WEATHER,
    ID_CHK_MOD_CALENDAR,
    ID_CHK_MOD_TRAY,
    ID_CHK_MOD_FOCUS_TIMER,
    ID_CHK_MOD_LOOKUP,
    ID_BTN_CLEAR_CLIPBOARD,
    ID_BTN_BROWSE_CLIPBOARD,

    // Bottom action bar
    ID_BTN_RESET_DEFAULTS,
    ID_BTN_APPLY,
    ID_BTN_CANCEL,
    ID_LBL_STATUS
};

// Controls tracking
inline HWND hTab = nullptr;
inline std::vector<HWND> g_pageControls[3];

// Tab 0: Appearance controls
inline HWND hComboShape = nullptr;
inline HWND hComboTheme = nullptr;
inline HWND hComboAccent = nullptr;
inline HWND hComboScale = nullptr;
inline HWND hComboAnim = nullptr;
inline HWND hChkRealGlass = nullptr;
inline HWND hChkBloom = nullptr;
inline HWND hChkShadow = nullptr;
inline HWND hChkBorder = nullptr;

// Tab 1: Behavior controls
inline HWND hComboHoverMode = nullptr;
inline HWND hComboPos = nullptr;
inline HWND hComboDefaultDash = nullptr;
inline HWND hChkStartup = nullptr;
inline HWND hChkAutohideFs = nullptr;
inline HWND hChkRememberLastPage = nullptr;
inline HWND hChkNotifFsSuppress = nullptr;
inline HWND hChkNotifEnabled = nullptr;

// Tab 2: Features & Widgets controls
inline HWND hChkModMedia = nullptr;
inline HWND hChkModLyrics = nullptr;
inline HWND hChkModQuickControls = nullptr;
inline HWND hChkModHardware = nullptr;
inline HWND hChkModWeather = nullptr;
inline HWND hChkModCalendar = nullptr;
inline HWND hChkModTray = nullptr;
inline HWND hChkModFocusTimer = nullptr;
inline HWND hChkModLookup = nullptr;

// Bottom controls
inline HWND hLblStatus = nullptr;

// ── CLIPBOARD VIEWER DIALOG ──────────────────────────────────────────────────
namespace dynamic_clipboard_dlg {
    inline HWND s_hList = nullptr;
    inline HWND s_hPreview = nullptr;
    inline std::vector<island_clipboard::ClipboardEntry> s_entries;

    inline void RefreshList() {
        if (!s_hList || !s_hPreview) return;
        SendMessageW(s_hList, LB_RESETCONTENT, 0, 0);
        s_entries = island_globals::g_clipboardManager.GetEntries();
        for (const auto& entry : s_entries) {
            std::wstring display = (entry.pinned ? L"[PIN] " : L"      ") + entry.preview;
            SendMessageW(s_hList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(display.c_str()));
        }
        if (!s_entries.empty()) {
            SendMessageW(s_hList, LB_SETCURSEL, 0, 0);
            SetWindowTextW(s_hPreview, s_entries[0].text.c_str());
        } else {
            SetWindowTextW(s_hPreview, L"No clipboard history items recorded.");
        }
    }

    inline LRESULT CALLBACK ClipboardWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");
            HWND lblList = CreateWindowExW(0, L"STATIC", L"Saved Clipboard Entries:", WS_CHILD | WS_VISIBLE,
                                           16, 12, 300, 20, hwnd, nullptr, nullptr, nullptr);
            SendMessageW(lblList, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            s_hList = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
                                      WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | LBS_NOTIFY,
                                      16, 36, 520, 160, hwnd, reinterpret_cast<HMENU>(301), nullptr, nullptr);
            SendMessageW(s_hList, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND lblPrev = CreateWindowExW(0, L"STATIC", L"Full Text Preview:", WS_CHILD | WS_VISIBLE,
                                           16, 206, 300, 20, hwnd, nullptr, nullptr, nullptr);
            SendMessageW(lblPrev, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            s_hPreview = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                         WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                                         16, 228, 520, 120, hwnd, reinterpret_cast<HMENU>(302), nullptr, nullptr);
            SendMessageW(s_hPreview, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND btnCopy = CreateWindowExW(0, L"BUTTON", L"📋 Copy", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                                           16, 362, 95, 28, hwnd, reinterpret_cast<HMENU>(303), nullptr, nullptr);
            SendMessageW(btnCopy, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND btnPin = CreateWindowExW(0, L"BUTTON", L"📌 Toggle Pin", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                                          118, 362, 110, 28, hwnd, reinterpret_cast<HMENU>(304), nullptr, nullptr);
            SendMessageW(btnPin, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND btnDel = CreateWindowExW(0, L"BUTTON", L"🗑️ Delete", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                                          235, 362, 95, 28, hwnd, reinterpret_cast<HMENU>(305), nullptr, nullptr);
            SendMessageW(btnDel, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND btnClear = CreateWindowExW(0, L"BUTTON", L"🧹 Clear All", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                                            337, 362, 95, 28, hwnd, reinterpret_cast<HMENU>(306), nullptr, nullptr);
            SendMessageW(btnClear, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND btnClose = CreateWindowExW(0, L"BUTTON", L"Close", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                                            441, 362, 95, 28, hwnd, reinterpret_cast<HMENU>(IDCANCEL), nullptr, nullptr);
            SendMessageW(btnClose, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            RefreshList();
            return 0;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);
            if (id == 301 && code == LBN_SELCHANGE) {
                int cur = static_cast<int>(SendMessageW(s_hList, LB_GETCURSEL, 0, 0));
                if (cur >= 0 && static_cast<size_t>(cur) < s_entries.size()) {
                    SetWindowTextW(s_hPreview, s_entries[cur].text.c_str());
                }
            } else if (id == 303) { // Copy
                int cur = static_cast<int>(SendMessageW(s_hList, LB_GETCURSEL, 0, 0));
                if (cur >= 0 && static_cast<size_t>(cur) < s_entries.size()) {
                    island_globals::g_clipboardManager.CopyToSystemClipboard(s_entries[cur].id, hwnd);
                }
            } else if (id == 304) { // Pin
                int cur = static_cast<int>(SendMessageW(s_hList, LB_GETCURSEL, 0, 0));
                if (cur >= 0 && static_cast<size_t>(cur) < s_entries.size()) {
                    island_globals::g_clipboardManager.TogglePin(s_entries[cur].id);
                    RefreshList();
                    SendMessageW(s_hList, LB_SETCURSEL, cur, 0);
                }
            } else if (id == 305) { // Delete
                int cur = static_cast<int>(SendMessageW(s_hList, LB_GETCURSEL, 0, 0));
                if (cur >= 0 && static_cast<size_t>(cur) < s_entries.size()) {
                    island_globals::g_clipboardManager.DeleteEntry(s_entries[cur].id);
                    RefreshList();
                }
            } else if (id == 306) { // Clear all
                island_globals::g_clipboardManager.ClearAll();
                RefreshList();
            } else if (id == IDCANCEL) {
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        }
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN: {
            HDC hdc = reinterpret_cast<HDC>(wParam);
            SetTextColor(hdc, RGB(235, 235, 240));
            SetBkColor(hdc, RGB(28, 28, 32));
            static HBRUSH s_darkBrush = CreateSolidBrush(RGB(28, 28, 32));
            return reinterpret_cast<INT_PTR>(s_darkBrush);
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    inline void ShowClipboardDialog(HWND hParent) {
        HINSTANCE hInst = GetModuleHandleW(nullptr);
        static bool s_registered = false;
        if (!s_registered) {
            WNDCLASSEXW wc{};
            wc.cbSize = sizeof(wc);
            wc.lpfnWndProc = ClipboardWndProc;
            wc.hInstance = hInst;
            wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
            wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
            wc.lpszClassName = L"DynamicEdgeClipboardViewer";
            RegisterClassExW(&wc);
            s_registered = true;
        }

        RECT rc = {};
        if (hParent) GetWindowRect(hParent, &rc);
        int px = rc.left + (rc.right - rc.left - 565) / 2;
        int py = rc.top + (rc.bottom - rc.top - 435) / 2;

        HWND dlg = CreateWindowExW(
            WS_EX_DLGMODALFRAME,
            L"DynamicEdgeClipboardViewer",
            L"Clipboard History Manager",
            WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
            px, py, 565, 435,
            hParent, nullptr, hInst, nullptr);

        if (dlg) {
            BOOL dark = TRUE;
            DwmSetWindowAttribute(dlg, 20, &dark, sizeof(dark));
        }
    }
}

// ── INI ACCESS HELPERS ──────────────────────────────────────────────────────
inline std::wstring ReadIniString(const wchar_t* section, const wchar_t* key, const wchar_t* def) {
    wchar_t buf[512] = {};
    GetPrivateProfileStringW(section, key, def, buf, ARRAYSIZE(buf), GetIniFilePath().c_str());
    return buf;
}

inline int ReadIniInt(const wchar_t* section, const wchar_t* key, int def) {
    return GetPrivateProfileIntW(section, key, def, GetIniFilePath().c_str());
}

inline void WriteIniString(const wchar_t* section, const wchar_t* key, const wchar_t* val) {
    WritePrivateProfileStringW(section, key, val, GetIniFilePath().c_str());
}

inline void WriteIniInt(const wchar_t* section, const wchar_t* key, int val) {
    wchar_t buf[32] = {};
    swprintf_s(buf, L"%d", val);
    WritePrivateProfileStringW(section, key, buf, GetIniFilePath().c_str());
}

// ── UI SYNC ENGINE ──────────────────────────────────────────────────────────
inline void LoadValuesIntoUi() {
    auto selectCombo = [](HWND cb, const SettingOption* opts, size_t count, const std::wstring& current, size_t defaultIdx = 0) {
        if (!cb) return;
        for (size_t i = 0; i < count; ++i) {
            if (_wcsicmp(opts[i].val, current.c_str()) == 0) {
                SendMessageW(cb, CB_SETCURSEL, i, 0);
                return;
            }
        }
        try {
            float curF = std::stof(current);
            for (size_t i = 0; i < count; ++i) {
                if (std::fabs(curF - std::stof(opts[i].val)) < 0.05f) {
                    SendMessageW(cb, CB_SETCURSEL, i, 0);
                    return;
                }
            }
        } catch (...) {}
        SendMessageW(cb, CB_SETCURSEL, defaultIdx < count ? defaultIdx : 0, 0);
    };

    // Tab 0: Appearance
    selectCombo(hComboShape, kShapeOptions, ARRAYSIZE(kShapeOptions), ReadIniString(L"Settings", L"Appearance.ShapeStyle", L"pill"));
    selectCombo(hComboTheme, kThemeOptions, ARRAYSIZE(kThemeOptions), ReadIniString(L"Settings", L"Themes.ThemePreset", L"obsidian"));
    {
        std::wstring accentMode = ReadIniString(L"Settings", L"Themes.AccentColorMode", L"auto");
        std::wstring customHex = ReadIniString(L"Settings", L"Themes.CustomAccentHex", L"#4CC9F0");
        if (_wcsicmp(accentMode.c_str(), L"system") == 0) {
            selectCombo(hComboAccent, kAccentColorOptions, ARRAYSIZE(kAccentColorOptions), L"system");
        } else if (_wcsicmp(accentMode.c_str(), L"custom") == 0) {
            selectCombo(hComboAccent, kAccentColorOptions, ARRAYSIZE(kAccentColorOptions), customHex);
        } else {
            selectCombo(hComboAccent, kAccentColorOptions, ARRAYSIZE(kAccentColorOptions), L"auto");
        }
    }
    selectCombo(hComboScale, kScaleOptions, ARRAYSIZE(kScaleOptions), ReadIniString(L"Settings", L"Appearance.SizeScale", L"1.0"), 2);
    selectCombo(hComboAnim, kAnimOptions, ARRAYSIZE(kAnimOptions), ReadIniString(L"Settings", L"Animations.AnimationStyle", L"applespring"));

    SendMessageW(hChkRealGlass, BM_SETCHECK, ReadIniInt(L"Settings", L"Themes.RealLiquidGlass", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkBloom, BM_SETCHECK, ReadIniInt(L"Settings", L"Themes.MaterialDepth", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkShadow, BM_SETCHECK, ReadIniInt(L"Settings", L"Themes.DropShadow", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkBorder, BM_SETCHECK, ReadIniInt(L"Settings", L"Themes.ContourBorderEnabled", 1) ? BST_CHECKED : BST_UNCHECKED, 0);

    // Tab 1: Behavior
    int expandOnHover = ReadIniInt(L"Settings", L"Behavior.ExpandOnHover", 0);
    selectCombo(hComboHoverMode, kHoverModeOptions, ARRAYSIZE(kHoverModeOptions), expandOnHover ? L"1" : L"0");
    selectCombo(hComboPos, kPosOptions, ARRAYSIZE(kPosOptions), ReadIniString(L"Settings", L"Appearance.Position", L"top-center"));
    selectCombo(hComboDefaultDash, kDefaultDashOptions, ARRAYSIZE(kDefaultDashOptions), ReadIniString(L"Settings", L"Behavior.DefaultDashboardPage", L"overview"));

    SendMessageW(hChkStartup, BM_SETCHECK, standalone_app::IsStartupEnabled() ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkAutohideFs, BM_SETCHECK, ReadIniInt(L"Settings", L"Behavior.AutoHideFullscreen", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkRememberLastPage, BM_SETCHECK, ReadIniInt(L"Settings", L"Behavior.RememberLastPage", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkNotifFsSuppress, BM_SETCHECK, ReadIniInt(L"Settings", L"Notifications.SuppressInFullscreen", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkNotifEnabled, BM_SETCHECK, ReadIniInt(L"Settings", L"Notifications.Enabled", 1) ? BST_CHECKED : BST_UNCHECKED, 0);

    // Tab 2: Features & Widgets
    SendMessageW(hChkModMedia, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.Media", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModLyrics, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.Lyrics", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModQuickControls, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.QuickControls", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModHardware, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.HardwareMonitorModule", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModWeather, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.Weather", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModCalendar, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.Calendar", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModTray, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.FileTrayModule", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModFocusTimer, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.FocusTimer", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModLookup, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.QuickLookup", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
}

inline void SaveValuesFromUi(HWND hwnd) {
    auto getComboVal = [](HWND cb, const SettingOption* opts, size_t count) -> const wchar_t* {
        int idx = static_cast<int>(SendMessageW(cb, CB_GETCURSEL, 0, 0));
        if (idx >= 0 && static_cast<size_t>(idx) < count) return opts[idx].val;
        return opts[0].val;
    };

    // Tab 0: Appearance
    const wchar_t* shapeVal = getComboVal(hComboShape, kShapeOptions, ARRAYSIZE(kShapeOptions));
    WriteIniString(L"Settings", L"Appearance.ShapeStyle", shapeVal);
    WriteIniInt(L"Values", L"W11StyleOverride", -1);
    WriteIniInt(L"Values", L"NotchStyleOverride", -1);
    WriteIniInt(L"Values", L"BladeStyleOverride", -1);

    const wchar_t* selectedTheme = getComboVal(hComboTheme, kThemeOptions, ARRAYSIZE(kThemeOptions));
    WriteIniString(L"Settings", L"Themes.ThemePreset", selectedTheme);
    const int tIdx = island_policy::ThemeIndexFromId(selectedTheme);
    if (tIdx >= 0) {
        WriteIniInt(L"Values", L"ColorThemeV2", tIdx);
        Wh_SetIntValue(L"ColorThemeV2", tIdx);
    }

    const wchar_t* selectedAccent = getComboVal(hComboAccent, kAccentColorOptions, ARRAYSIZE(kAccentColorOptions));
    if (_wcsicmp(selectedAccent, L"auto") == 0) {
        WriteIniString(L"Settings", L"Themes.AccentColorMode", L"auto");
    } else if (_wcsicmp(selectedAccent, L"system") == 0) {
        WriteIniString(L"Settings", L"Themes.AccentColorMode", L"system");
    } else {
        WriteIniString(L"Settings", L"Themes.AccentColorMode", L"custom");
        WriteIniString(L"Settings", L"Themes.CustomAccentHex", selectedAccent);
    }

    WriteIniString(L"Settings", L"Appearance.SizeScale", getComboVal(hComboScale, kScaleOptions, ARRAYSIZE(kScaleOptions)));
    WriteIniString(L"Settings", L"Animations.AnimationStyle", getComboVal(hComboAnim, kAnimOptions, ARRAYSIZE(kAnimOptions)));

    WriteIniInt(L"Settings", L"Themes.RealLiquidGlass", SendMessageW(hChkRealGlass, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Themes.MaterialDepth", SendMessageW(hChkBloom, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Themes.DropShadow", SendMessageW(hChkShadow, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    bool borderOn = (SendMessageW(hChkBorder, BM_GETCHECK, 0, 0) == BST_CHECKED);
    WriteIniInt(L"Settings", L"Themes.ContourBorderEnabled", borderOn ? 1 : 0);
    WriteIniString(L"Settings", L"Themes.ContourBorderMode", borderOn ? L"default" : L"borderless");

    // Tab 1: Behavior
    int hoverVal = _wtoi(getComboVal(hComboHoverMode, kHoverModeOptions, ARRAYSIZE(kHoverModeOptions)));
    WriteIniInt(L"Settings", L"Behavior.ExpandOnHover", hoverVal);
    WriteIniInt(L"Values", L"ExpandOnHoverOverride", -1);

    WriteIniString(L"Settings", L"Appearance.Position", getComboVal(hComboPos, kPosOptions, ARRAYSIZE(kPosOptions)));
    WriteIniString(L"Settings", L"Behavior.DefaultDashboardPage", getComboVal(hComboDefaultDash, kDefaultDashOptions, ARRAYSIZE(kDefaultDashOptions)));

    const bool wantStartup = (SendMessageW(hChkStartup, BM_GETCHECK, 0, 0) == BST_CHECKED);
    if (wantStartup != standalone_app::IsStartupEnabled()) {
        standalone_app::SetStartupEnabled(wantStartup);
    }

    WriteIniInt(L"Settings", L"Behavior.AutoHideFullscreen", SendMessageW(hChkAutohideFs, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Context.AutoHideFullscreen", SendMessageW(hChkAutohideFs, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Behavior.RememberLastPage", SendMessageW(hChkRememberLastPage, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Notifications.SuppressInFullscreen", SendMessageW(hChkNotifFsSuppress, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Notifications.Enabled", SendMessageW(hChkNotifEnabled, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);

    // Tab 2: Features & Widgets
    WriteIniInt(L"Settings", L"Modules.Media", SendMessageW(hChkModMedia, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Modules.Lyrics", SendMessageW(hChkModLyrics, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Modules.QuickControls", SendMessageW(hChkModQuickControls, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Modules.HardwareMonitorModule", SendMessageW(hChkModHardware, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Modules.Weather", SendMessageW(hChkModWeather, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Modules.Calendar", SendMessageW(hChkModCalendar, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Modules.FileTrayModule", SendMessageW(hChkModTray, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Modules.FocusTimer", SendMessageW(hChkModFocusTimer, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Modules.QuickLookup", SendMessageW(hChkModLookup, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);

    // Flush Windows INI cache to disk immediately
    WritePrivateProfileStringW(nullptr, nullptr, nullptr, GetIniFilePath().c_str());

    // Clear runtime overrides so persistent INI settings take authoritative effect
    Wh_SetIntValue(L"W11StyleOverride", -1);
    Wh_SetIntValue(L"NotchStyleOverride", -1);
    Wh_SetIntValue(L"BladeStyleOverride", -1);
    Wh_SetIntValue(L"PillOpacityOverride", -1);
    Wh_SetIntValue(L"ExpandOnHoverOverride", -1);

    // Instantly notify live Dynamic Island overlay
    LoadSettings();
    g_layoutDirty = true;
    TriggerNudge();
    if (g_hwnd) {
        PostMessageW(g_hwnd, WM_APP_NEW_EVENT, 0, 0);
    }
}

inline void ResetToRecommendedDefaults(HWND hwnd) {
    WriteIniString(L"Settings", L"Appearance.ShapeStyle", L"notch");
    WriteIniString(L"Settings", L"Themes.ThemePreset", L"obsidian");
    WriteIniString(L"Settings", L"Themes.AccentColorMode", L"auto");
    WriteIniString(L"Settings", L"Appearance.SizeScale", L"1.0");
    WriteIniString(L"Settings", L"Animations.AnimationStyle", L"applespring");
    WriteIniInt(L"Settings", L"Themes.RealLiquidGlass", 1);
    WriteIniInt(L"Settings", L"Themes.MaterialDepth", 1);
    WriteIniInt(L"Settings", L"Themes.DropShadow", 1);
    WriteIniInt(L"Settings", L"Themes.ContourBorderEnabled", 1);
    WriteIniString(L"Settings", L"Themes.ContourBorderMode", L"default");
    WriteIniInt(L"Settings", L"Behavior.ExpandOnHover", 0);
    WriteIniString(L"Settings", L"Appearance.Position", L"top-center");
    WriteIniString(L"Settings", L"Behavior.DefaultDashboardPage", L"overview");
    WriteIniInt(L"Settings", L"Behavior.AutoHideFullscreen", 1);
    WriteIniInt(L"Settings", L"Context.AutoHideFullscreen", 1);
    WriteIniInt(L"Settings", L"Behavior.RememberLastPage", 1);
    WriteIniInt(L"Settings", L"Notifications.SuppressInFullscreen", 1);
    WriteIniInt(L"Settings", L"Notifications.Enabled", 1);
    WriteIniInt(L"Settings", L"Modules.Media", 1);
    WriteIniInt(L"Settings", L"Modules.Lyrics", 1);
    WriteIniInt(L"Settings", L"Modules.QuickControls", 1);
    WriteIniInt(L"Settings", L"Modules.HardwareMonitorModule", 1);
    WriteIniInt(L"Settings", L"Modules.Weather", 1);
    WriteIniInt(L"Settings", L"Modules.Calendar", 1);
    WriteIniInt(L"Settings", L"Modules.FileTrayModule", 1);
    WriteIniInt(L"Settings", L"Modules.FocusTimer", 1);
    WriteIniInt(L"Settings", L"Modules.QuickLookup", 1);

    WritePrivateProfileStringW(nullptr, nullptr, nullptr, GetIniFilePath().c_str());

    Wh_SetIntValue(L"W11StyleOverride", -1);
    Wh_SetIntValue(L"NotchStyleOverride", -1);
    Wh_SetIntValue(L"BladeStyleOverride", -1);
    Wh_SetIntValue(L"PillOpacityOverride", -1);
    Wh_SetIntValue(L"ExpandOnHoverOverride", -1);
    Wh_SetIntValue(L"ColorThemeV2", 0);

    LoadSettings();
    g_layoutDirty = true;
    TriggerNudge();
    if (g_hwnd) {
        PostMessageW(g_hwnd, WM_APP_NEW_EVENT, 0, 0);
    }
    LoadValuesIntoUi();
}

inline void ShowTab(int tabIndex) {
    for (int p = 0; p < 3; ++p) {
        const int cmdShow = (p == tabIndex) ? SW_SHOW : SW_HIDE;
        for (HWND h : g_pageControls[p]) {
            ShowWindow(h, cmdShow);
        }
    }
}

inline LRESULT CALLBACK TabSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
    if (uMsg == WM_ERASEBKGND) {
        HDC hdc = reinterpret_cast<HDC>(wParam);
        RECT rc;
        GetClientRect(hWnd, &rc);
        HBRUSH hBr = CreateSolidBrush(RGB(28, 28, 32));
        FillRect(hdc, &rc, hBr);
        DeleteObject(hBr);
        return 1;
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

// ── MAIN SETTINGS WINDOW PROCEDURE ──────────────────────────────────────────
inline LRESULT CALLBACK SettingsWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            INITCOMMONCONTROLSEX icex{sizeof(icex), ICC_TAB_CLASSES | ICC_STANDARD_CLASSES};
            InitCommonControlsEx(&icex);

            HFONT hFont = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");
            HFONT hBoldFont = CreateFontW(-13, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                          CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");

            hTab = CreateWindowExW(0, WC_TABCONTROLW, L"",
                                   WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
                                   16, 12, 608, 36, hwnd, reinterpret_cast<HMENU>(ID_TAB),
                                   reinterpret_cast<LPCREATESTRUCT>(lParam)->hInstance, nullptr);
            SendMessageW(hTab, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
            SetWindowTheme(hTab, L"DarkMode_Explorer", nullptr);
            SetWindowSubclass(hTab, TabSubclassProc, 1, 0);

            TCITEMW ti = {};
            ti.mask = TCIF_TEXT;
            ti.pszText = const_cast<LPWSTR>(L"🎨 Appearance");
            TabCtrl_InsertItem(hTab, 0, &ti);
            ti.pszText = const_cast<LPWSTR>(L"⚡ Behavior");
            TabCtrl_InsertItem(hTab, 1, &ti);
            ti.pszText = const_cast<LPWSTR>(L"🧩 Features & Widgets");
            TabCtrl_InsertItem(hTab, 2, &ti);

            for (int i = 0; i < 3; ++i) g_pageControls[i].clear();

            auto makeLabel = [&](int x, int y, int w, int h, const wchar_t* text, int page, bool bold = false) -> HWND {
                HWND lbl = CreateWindowExW(0, L"STATIC", text, WS_CHILD | SS_LEFT, x, y, w, h, hwnd, nullptr, nullptr, nullptr);
                SendMessageW(lbl, WM_SETFONT, reinterpret_cast<WPARAM>(bold ? hBoldFont : hFont), TRUE);
                g_pageControls[page].push_back(lbl);
                return lbl;
            };

            auto makeCombo = [&](int x, int y, int w, int h, int id, const SettingOption* opts, size_t count, int page) -> HWND {
                HWND cb = CreateWindowExW(0, WC_COMBOBOXW, L"", WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL, x, y, w, h, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), nullptr, nullptr);
                SendMessageW(cb, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
                SetWindowTheme(cb, L"DarkMode_Explorer", nullptr);
                for (size_t i = 0; i < count; ++i) {
                    SendMessageW(cb, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(opts[i].label));
                }
                g_pageControls[page].push_back(cb);
                return cb;
            };

            auto makeCheck = [&](int x, int y, int w, int h, int id, const wchar_t* text, int page) -> HWND {
                HWND chk = CreateWindowExW(0, L"BUTTON", text, WS_CHILD | BS_AUTOCHECKBOX, x, y, w, h, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), nullptr, nullptr);
                SendMessageW(chk, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
                SetWindowTheme(chk, L"DarkMode_Explorer", nullptr);
                g_pageControls[page].push_back(chk);
                return chk;
            };

            auto makeBtn = [&](int x, int y, int w, int h, int id, const wchar_t* text, int page) -> HWND {
                HWND btn = CreateWindowExW(0, L"BUTTON", text, WS_CHILD | BS_PUSHBUTTON, x, y, w, h, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), nullptr, nullptr);
                SendMessageW(btn, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
                SetWindowTheme(btn, L"DarkMode_Explorer", nullptr);
                g_pageControls[page].push_back(btn);
                return btn;
            };

            // ── PAGE 0: Appearance ──────────────────────────────────────────────
            makeLabel(32, 64, 150, 20, L"Island Shape Style:", 0);
            hComboShape = makeCombo(190, 60, 400, 200, ID_COMBO_SHAPE, kShapeOptions, ARRAYSIZE(kShapeOptions), 0);

            makeLabel(32, 102, 150, 20, L"Theme Palette:", 0);
            hComboTheme = makeCombo(190, 98, 400, 220, ID_COMBO_THEME, kThemeOptions, ARRAYSIZE(kThemeOptions), 0);

            makeLabel(32, 140, 150, 20, L"Accent Color:", 0);
            hComboAccent = makeCombo(190, 136, 400, 220, ID_COMBO_ACCENT, kAccentColorOptions, ARRAYSIZE(kAccentColorOptions), 0);

            makeLabel(32, 178, 150, 20, L"Size Scale:", 0);
            hComboScale = makeCombo(190, 174, 400, 180, ID_COMBO_SCALE, kScaleOptions, ARRAYSIZE(kScaleOptions), 0);

            makeLabel(32, 216, 150, 20, L"Animation Physics:", 0);
            hComboAnim = makeCombo(190, 212, 400, 160, ID_COMBO_ANIM, kAnimOptions, ARRAYSIZE(kAnimOptions), 0);

            hChkRealGlass = makeCheck(32, 258, 560, 24, ID_CHK_REAL_GLASS, L"💎 Optical Backdrop Blur & Liquid Glass Refraction", 0);
            hChkBloom = makeCheck(32, 288, 560, 24, ID_CHK_BLOOM, L"🎵 Dynamic Album Art & Accent Glow (Glass tint washes with music)", 0);
            hChkShadow = makeCheck(32, 318, 560, 24, ID_CHK_SHADOW, L"🌑 Soft Curved Ambient Drop Shadow", 0);
            hChkBorder = makeCheck(32, 348, 560, 24, ID_CHK_BORDER, L"✨ Specular Hairline Contour Rim", 0);

            // ── PAGE 1: Behavior ────────────────────────────────────────────────
            makeLabel(32, 64, 160, 20, L"Interaction Mode:", 1);
            hComboHoverMode = makeCombo(200, 60, 390, 140, ID_COMBO_HOVER_MODE, kHoverModeOptions, ARRAYSIZE(kHoverModeOptions), 1);

            makeLabel(32, 102, 160, 20, L"Screen Position:", 1);
            hComboPos = makeCombo(200, 98, 390, 200, ID_COMBO_POS, kPosOptions, ARRAYSIZE(kPosOptions), 1);

            makeLabel(32, 140, 160, 20, L"Default Dashboard:", 1);
            hComboDefaultDash = makeCombo(200, 136, 390, 180, ID_COMBO_DEFAULT_DASH, kDefaultDashOptions, ARRAYSIZE(kDefaultDashOptions), 1);

            hChkStartup = makeCheck(32, 184, 560, 24, ID_CHK_STARTUP, L"🚀 Launch Dynamic Island automatically on Windows Startup", 1);
            hChkAutohideFs = makeCheck(32, 216, 560, 24, ID_CHK_AUTOHIDE_FS, L"🖥️ Auto-hide when Fullscreen Apps or Games are running", 1);
            hChkRememberLastPage = makeCheck(32, 248, 560, 24, ID_CHK_REMEMBER_LAST_PAGE, L"📌 Remember last viewed dashboard tab across collapse", 1);
            hChkNotifFsSuppress = makeCheck(32, 280, 560, 24, ID_CHK_NOTIF_FS_SUPPRESS, L"🔕 Suppress notification banners during Fullscreen apps / games", 1);
            hChkNotifEnabled = makeCheck(32, 312, 560, 24, ID_CHK_NOTIF_ENABLED, L"🔔 Enable system peripheral alerts (AirPods, Headphones, Battery, Volume)", 1);

            // ── PAGE 2: Features & Widgets ──────────────────────────────────────
            makeLabel(32, 60, 560, 20, L"Music & Media Features (Dynamic Ledge Parity):", 2, true);
            hChkModMedia = makeCheck(48, 86, 540, 24, ID_CHK_MOD_MEDIA, L"🎵 Now Playing Media (Album Art, Song Marquee & Transport Controls)", 2);
            hChkModLyrics = makeCheck(48, 114, 540, 24, ID_CHK_MOD_LYRICS, L"🎤 Live Synced Lyrics (LRCLIB & Karaoke display)", 2);

            makeLabel(32, 150, 560, 20, L"Desktop Dashboard Widgets (Unified 410 × 210 Frame):", 2, true);
            hChkModQuickControls = makeCheck(48, 176, 260, 24, ID_CHK_MOD_QUICK_CONTROLS, L"🎛️ Desktop Quick Controls", 2);
            hChkModHardware = makeCheck(320, 176, 280, 24, ID_CHK_MOD_HARDWARE, L"📊 System Hardware Monitor", 2);
            hChkModWeather = makeCheck(48, 206, 260, 24, ID_CHK_MOD_WEATHER, L"☀️ Live Weather & Forecast", 2);
            hChkModCalendar = makeCheck(320, 206, 280, 24, ID_CHK_MOD_CALENDAR, L"📅 Calendar & Upcoming Schedule", 2);
            hChkModTray = makeCheck(48, 236, 260, 24, ID_CHK_MOD_TRAY, L"📁 Drag & Drop File Tray Shelf", 2);
            hChkModFocusTimer = makeCheck(320, 236, 280, 24, ID_CHK_MOD_FOCUS_TIMER, L"⏱️ Focus & Pomodoro Timer", 2);
            hChkModLookup = makeCheck(48, 266, 540, 24, ID_CHK_MOD_LOOKUP, L"🔍 Quick Lookup & App Launcher (Ctrl+Alt+Space)", 2);

            makeBtn(48, 308, 220, 28, ID_BTN_CLEAR_CLIPBOARD, L"🧹 Clear Clipboard History", 2);
            makeBtn(280, 308, 220, 28, ID_BTN_BROWSE_CLIPBOARD, L"📋 Browse Clipboard...", 2);

            // ── Dialog Bottom Action Bar ────────────────────────────────────────
            HWND btnReset = CreateWindowExW(0, L"BUTTON", L"↺ Reset Defaults", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                            20, 412, 130, 32, hwnd, reinterpret_cast<HMENU>(ID_BTN_RESET_DEFAULTS), nullptr, nullptr);
            SendMessageW(btnReset, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
            SetWindowTheme(btnReset, L"DarkMode_Explorer", nullptr);

            hLblStatus = CreateWindowExW(0, L"STATIC", L"Changes apply instantly to Island.", WS_CHILD | WS_VISIBLE | SS_LEFT,
                                         162, 420, 260, 22, hwnd, reinterpret_cast<HMENU>(ID_LBL_STATUS), nullptr, nullptr);
            SendMessageW(hLblStatus, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND btnApply = CreateWindowExW(0, L"BUTTON", L"Save & Apply", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                            432, 412, 110, 32, hwnd, reinterpret_cast<HMENU>(ID_BTN_APPLY), nullptr, nullptr);
            SendMessageW(btnApply, WM_SETFONT, reinterpret_cast<WPARAM>(hBoldFont), TRUE);
            SetWindowTheme(btnApply, L"DarkMode_Explorer", nullptr);

            HWND btnCancel = CreateWindowExW(0, L"BUTTON", L"Close", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                             552, 412, 72, 32, hwnd, reinterpret_cast<HMENU>(ID_BTN_CANCEL), nullptr, nullptr);
            SendMessageW(btnCancel, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
            SetWindowTheme(btnCancel, L"DarkMode_Explorer", nullptr);

            LoadValuesIntoUi();
            ShowTab(0);
            return 0;
        }

        case WM_NOTIFY: {
            LPNMHDR pnmh = reinterpret_cast<LPNMHDR>(lParam);
            if (pnmh->idFrom == ID_TAB && pnmh->code == TCN_SELCHANGE) {
                int cur = TabCtrl_GetCurSel(hTab);
                ShowTab(cur);
            }
            return 0;
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == ID_BTN_APPLY) {
                SaveValuesFromUi(hwnd);
                if (hLblStatus) SetWindowTextW(hLblStatus, L"✓ Saved & applied successfully!");
            } else if (id == ID_BTN_RESET_DEFAULTS) {
                if (MessageBoxW(hwnd, L"Restore all recommended factory defaults?", L"Reset Defaults", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    ResetToRecommendedDefaults(hwnd);
                    if (hLblStatus) SetWindowTextW(hLblStatus, L"✓ Recommended defaults restored!");
                }
            } else if (id == ID_BTN_CANCEL) {
                DestroyWindow(hwnd);
            } else if (id == ID_BTN_CLEAR_CLIPBOARD) {
                island_globals::g_clipboardManager.ClearAll();
                MessageBoxW(hwnd, L"Clipboard history cleared.", L"Clipboard Manager", MB_OK | MB_ICONINFORMATION);
            } else if (id == ID_BTN_BROWSE_CLIPBOARD) {
                dynamic_clipboard_dlg::ShowClipboardDialog(hwnd);
            }
            return 0;
        }

        case WM_CTLCOLORDLG:
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN: {
            HDC hdc = reinterpret_cast<HDC>(wParam);
            SetTextColor(hdc, RGB(235, 235, 240));
            SetBkColor(hdc, RGB(28, 28, 32));
            static HBRUSH s_darkBrush = CreateSolidBrush(RGB(28, 28, 32));
            return reinterpret_cast<INT_PTR>(s_darkBrush);
        }

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            g_hSettingsWnd = nullptr;
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

inline void ShowSettingsWindow(HWND hParent) {
    if (g_hSettingsWnd) {
        ShowWindow(g_hSettingsWnd, SW_RESTORE);
        SetForegroundWindow(g_hSettingsWnd);
        return;
    }

    HINSTANCE hInst = GetModuleHandleW(nullptr);
    static HBRUSH s_wndDarkBrush = CreateSolidBrush(RGB(28, 28, 32));
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = SettingsWndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    wc.hbrBackground = s_wndDarkBrush;
    wc.lpszClassName = L"DynamicIsland.SettingsDialog";
    RegisterClassExW(&wc);

    const int clientW = 644;
    const int clientH = 460;
    RECT wndRect = { 0, 0, clientW, clientH };
    DWORD dwStyle = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
    DWORD dwExStyle = WS_EX_DLGMODALFRAME;
    AdjustWindowRectEx(&wndRect, dwStyle, FALSE, dwExStyle);
    const int width = wndRect.right - wndRect.left;
    const int height = wndRect.bottom - wndRect.top;

    HMONITOR hMon = MonitorFromWindow(hParent ? hParent : GetDesktopWindow(), MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = {}; mi.cbSize = sizeof(mi);
    GetMonitorInfoW(hMon, &mi);
    const int posX = mi.rcWork.left + (mi.rcWork.right - mi.rcWork.left - width) / 2;
    const int posY = mi.rcWork.top + (mi.rcWork.bottom - mi.rcWork.top - height) / 2;

    HWND hwnd = CreateWindowExW(
        dwExStyle,
        wc.lpszClassName,
        L"Dynamic Island — Settings",
        dwStyle | WS_VISIBLE,
        posX, posY, width, height,
        nullptr, nullptr, hInst, nullptr
    );

    if (hwnd) {
        g_hSettingsWnd = hwnd;
        BOOL darkMode = TRUE;
        DwmSetWindowAttribute(hwnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &darkMode, sizeof(darkMode));
        SetForegroundWindow(hwnd);
    }
}

// ── ONBOARDING DIALOG ────────────────────────────────────────────────────────
inline void ShowOnboardingDialog(HWND hParent) {
    // For streamlined first run, immediately open Settings so user has 1 single source of truth
    ShowSettingsWindow(hParent);
}

}  // namespace settings_ui
