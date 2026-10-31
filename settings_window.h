// settings_window.h - Native Windows 11 Dark-Themed Settings GUI for Dynamic Island
#pragma once
#include <windows.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <string>
#include <vector>
#include <atomic>

namespace standalone_app {
    bool IsStartupEnabled();
    void SetStartupEnabled(bool);
    void EnsureSettingsFile();
}

namespace settings_ui {

inline HWND g_hSettingsWnd = nullptr;

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
    {L"iPhone Pill Capsule", L"pill"},
    {L"Windows 11 Flyout", L"w11"},
    {L"macOS Notch", L"notch"}
};

inline const SettingOption kThemeOptions[] = {
    {L"Transparent Liquid Glass", L"transparent"},
    {L"Obsidian (OLED Black)", L"obsidian"},
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

inline const SettingOption kOpacityOptions[] = {
    {L"100% (Opaque / Solid)", L"100"},
    {L"85% (Translucent)", L"85"},
    {L"70% (Glass Accent)", L"70"},
    {L"55% (See-Through)", L"55"},
    {L"16% (Crystal Clear Transparent)", L"16"}
};

// Control IDs
enum CtrlId : int {
    ID_TAB = 2000,
    ID_COMBO_POS,
    ID_COMBO_SHAPE,
    ID_COMBO_THEME,
    ID_COMBO_ANIM,
    ID_COMBO_SCALE,
    ID_COMBO_OPACITY,
    ID_CHK_REAL_GLASS,
    ID_CHK_BLOOM,
    ID_CHK_SHADOW,
    ID_CHK_BORDER,
    ID_CHK_HOVER,
    ID_CHK_AUTOHIDE_FS,
    ID_CHK_STARTUP,
    ID_CHK_LOG,
    ID_CHK_MOD_MEDIA,
    ID_CHK_MOD_LYRICS,
    ID_CHK_MOD_CALENDAR,
    ID_CHK_MOD_WEATHER,
    ID_CHK_MOD_HARDWARE,
    ID_CHK_MOD_TRAY,
    ID_CHK_MOD_LOOKUP,
    ID_BTN_APPLY,
    ID_BTN_OK,
    ID_BTN_CANCEL,
    ID_BTN_OPEN_INI
};

inline HWND hTab = nullptr;
inline HWND hComboPos = nullptr;
inline HWND hComboShape = nullptr;
inline HWND hComboTheme = nullptr;
inline HWND hComboAnim = nullptr;
inline HWND hComboScale = nullptr;
inline HWND hComboOpacity = nullptr;
inline HWND hChkRealGlass = nullptr;
inline HWND hChkBloom = nullptr;
inline HWND hChkShadow = nullptr;
inline HWND hChkBorder = nullptr;
inline HWND hChkHover = nullptr;
inline HWND hChkAutohideFs = nullptr;
inline HWND hChkStartup = nullptr;
inline HWND hChkLog = nullptr;
inline HWND hChkModMedia = nullptr;
inline HWND hChkModLyrics = nullptr;
inline HWND hChkModCalendar = nullptr;
inline HWND hChkModWeather = nullptr;
inline HWND hChkModHardware = nullptr;
inline HWND hChkModTray = nullptr;
inline HWND hChkModLookup = nullptr;
inline std::vector<HWND> g_pageControls[4];

inline std::wstring ReadIniString(const wchar_t* section, const wchar_t* key, const wchar_t* def) {
    wchar_t buf[256] = {};
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
    wchar_t buf[32];
    swprintf_s(buf, L"%d", val);
    WritePrivateProfileStringW(section, key, buf, GetIniFilePath().c_str());
}

inline void LoadValuesIntoUi() {
    auto selectCombo = [](HWND cb, const SettingOption* opts, size_t count, const std::wstring& current) {
        for (size_t i = 0; i < count; ++i) {
            if (_wcsicmp(current.c_str(), opts[i].val) == 0) {
                SendMessageW(cb, CB_SETCURSEL, i, 0);
                return;
            }
        }
        SendMessageW(cb, CB_SETCURSEL, 0, 0);
    };

    selectCombo(hComboPos, kPosOptions, ARRAYSIZE(kPosOptions), ReadIniString(L"Settings", L"Appearance.Position", L"top-center"));
    selectCombo(hComboShape, kShapeOptions, ARRAYSIZE(kShapeOptions), ReadIniString(L"Settings", L"Appearance.ShapeStyle", L"pill"));
    selectCombo(hComboTheme, kThemeOptions, ARRAYSIZE(kThemeOptions), ReadIniString(L"Settings", L"Themes.ThemePreset", L"transparent"));
    selectCombo(hComboAnim, kAnimOptions, ARRAYSIZE(kAnimOptions), ReadIniString(L"Settings", L"Animations.AnimationStyle", L"applespring"));
    selectCombo(hComboScale, kScaleOptions, ARRAYSIZE(kScaleOptions), ReadIniString(L"Settings", L"Appearance.SizeScale", L"1.0"));
    selectCombo(hComboOpacity, kOpacityOptions, ARRAYSIZE(kOpacityOptions), ReadIniString(L"Settings", L"Themes.PillOpacity", L"100"));

    SendMessageW(hChkRealGlass, BM_SETCHECK, ReadIniInt(L"Settings", L"Themes.RealLiquidGlass", 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkBloom, BM_SETCHECK, ReadIniInt(L"Settings", L"Themes.MaterialDepth", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkShadow, BM_SETCHECK, ReadIniInt(L"Settings", L"Themes.DropShadow", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkBorder, BM_SETCHECK, ReadIniInt(L"Settings", L"Themes.ContourBorderEnabled", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkHover, BM_SETCHECK, ReadIniInt(L"Settings", L"Behavior.ExpandOnHover", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkAutohideFs, BM_SETCHECK, ReadIniInt(L"Settings", L"Behavior.AutoHideFullscreen", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkStartup, BM_SETCHECK, standalone_app::IsStartupEnabled() ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkLog, BM_SETCHECK, ReadIniInt(L"Settings", L"Debug.Log", 0) ? BST_CHECKED : BST_UNCHECKED, 0);

    SendMessageW(hChkModMedia, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.Media", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModLyrics, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.Lyrics", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModCalendar, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.Calendar", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModWeather, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.Weather", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModHardware, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.HardwareMonitorModule", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModTray, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.FileTrayModule", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModLookup, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.QuickLookup", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
}

inline void SaveValuesFromUi(HWND hwnd) {
    auto getComboVal = [](HWND cb, const SettingOption* opts, size_t count) -> const wchar_t* {
        int idx = static_cast<int>(SendMessageW(cb, CB_GETCURSEL, 0, 0));
        if (idx >= 0 && static_cast<size_t>(idx) < count) return opts[idx].val;
        return opts[0].val;
    };

    WriteIniString(L"Settings", L"Appearance.Position", getComboVal(hComboPos, kPosOptions, ARRAYSIZE(kPosOptions)));
    WriteIniString(L"Settings", L"Appearance.ShapeStyle", getComboVal(hComboShape, kShapeOptions, ARRAYSIZE(kShapeOptions)));
    WriteIniString(L"Settings", L"Themes.ThemePreset", getComboVal(hComboTheme, kThemeOptions, ARRAYSIZE(kThemeOptions)));
    WriteIniString(L"Settings", L"Animations.AnimationStyle", getComboVal(hComboAnim, kAnimOptions, ARRAYSIZE(kAnimOptions)));
    WriteIniString(L"Settings", L"Appearance.SizeScale", getComboVal(hComboScale, kScaleOptions, ARRAYSIZE(kScaleOptions)));
    WriteIniString(L"Settings", L"Themes.PillOpacity", getComboVal(hComboOpacity, kOpacityOptions, ARRAYSIZE(kOpacityOptions)));

    WriteIniInt(L"Settings", L"Themes.RealLiquidGlass", SendMessageW(hChkRealGlass, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Themes.MaterialDepth", SendMessageW(hChkBloom, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Themes.DropShadow", SendMessageW(hChkShadow, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Themes.ContourBorderEnabled", SendMessageW(hChkBorder, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Behavior.ExpandOnHover", SendMessageW(hChkHover, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Behavior.AutoHideFullscreen", SendMessageW(hChkAutohideFs, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);

    const bool wantStartup = (SendMessageW(hChkStartup, BM_GETCHECK, 0, 0) == BST_CHECKED);
    if (wantStartup != standalone_app::IsStartupEnabled()) {
        standalone_app::SetStartupEnabled(wantStartup);
    }
    WriteIniInt(L"Settings", L"Debug.Log", SendMessageW(hChkLog, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);

    WriteIniInt(L"Settings", L"Modules.Media", SendMessageW(hChkModMedia, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Modules.Lyrics", SendMessageW(hChkModLyrics, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Modules.Calendar", SendMessageW(hChkModCalendar, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Modules.Weather", SendMessageW(hChkModWeather, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Modules.HardwareMonitorModule", SendMessageW(hChkModHardware, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Modules.FileTrayModule", SendMessageW(hChkModTray, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Modules.QuickLookup", SendMessageW(hChkModLookup, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);

    // Flush Windows INI cache to disk
    WritePrivateProfileStringW(nullptr, nullptr, nullptr, GetIniFilePath().c_str());

    // Instantly notify live Dynamic Island overlay window
    LoadSettings();
    g_layoutDirty = true;
    TriggerNudge();
    if (g_hwnd) {
        PostMessageW(g_hwnd, WM_APP_NEW_EVENT, 0, 0);
    }
}

inline void ShowTab(int tabIndex) {
    for (int p = 0; p < 4; ++p) {
        const int cmdShow = (p == tabIndex) ? SW_SHOW : SW_HIDE;
        for (HWND h : g_pageControls[p]) {
            ShowWindow(h, cmdShow);
        }
    }
}

inline LRESULT CALLBACK SettingsWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            INITCOMMONCONTROLSEX icex{sizeof(icex), ICC_TAB_CLASSES | ICC_STANDARD_CLASSES};
            InitCommonControlsEx(&icex);

            HFONT hFont = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");

            hTab = CreateWindowExW(0, WC_TABCONTROLW, L"",
                                   WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
                                   12, 12, 600, 420, hwnd, reinterpret_cast<HMENU>(ID_TAB),
                                   reinterpret_cast<LPCREATESTRUCT>(lParam)->hInstance, nullptr);
            SendMessageW(hTab, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            TCITEMW ti = {};
            ti.mask = TCIF_TEXT;
            ti.pszText = const_cast<LPWSTR>(L"🎨 Appearance & Theme");
            TabCtrl_InsertItem(hTab, 0, &ti);
            ti.pszText = const_cast<LPWSTR>(L"✨ Glass & Effects");
            TabCtrl_InsertItem(hTab, 1, &ti);
            ti.pszText = const_cast<LPWSTR>(L"⚡ Behavior & Animation");
            TabCtrl_InsertItem(hTab, 2, &ti);
            ti.pszText = const_cast<LPWSTR>(L"🧩 Active Modules");
            TabCtrl_InsertItem(hTab, 3, &ti);

            auto makeLabel = [&](int x, int y, int w, int h, const wchar_t* text, int page) -> HWND {
                HWND lbl = CreateWindowExW(0, L"STATIC", text, WS_CHILD | SS_LEFT, x, y, w, h, hwnd, nullptr, nullptr, nullptr);
                SendMessageW(lbl, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
                g_pageControls[page].push_back(lbl);
                return lbl;
            };

            auto makeCombo = [&](int x, int y, int w, int h, int id, const SettingOption* opts, size_t count, int page) -> HWND {
                HWND cb = CreateWindowExW(0, WC_COMBOBOXW, L"", WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL, x, y, w, h, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), nullptr, nullptr);
                SendMessageW(cb, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
                for (size_t i = 0; i < count; ++i) {
                    SendMessageW(cb, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(opts[i].label));
                }
                g_pageControls[page].push_back(cb);
                return cb;
            };

            auto makeCheck = [&](int x, int y, int w, int h, int id, const wchar_t* text, int page) -> HWND {
                HWND chk = CreateWindowExW(0, L"BUTTON", text, WS_CHILD | BS_AUTOCHECKBOX, x, y, w, h, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), nullptr, nullptr);
                SendMessageW(chk, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
                g_pageControls[page].push_back(chk);
                return chk;
            };

            // Page 0: Appearance & Theme
            makeLabel(32, 55, 180, 20, L"Screen Position:", 0);
            hComboPos = makeCombo(210, 52, 380, 240, ID_COMBO_POS, kPosOptions, ARRAYSIZE(kPosOptions), 0);

            makeLabel(32, 95, 180, 20, L"Island Shape Style:", 0);
            hComboShape = makeCombo(210, 92, 380, 240, ID_COMBO_SHAPE, kShapeOptions, ARRAYSIZE(kShapeOptions), 0);

            makeLabel(32, 135, 180, 20, L"Theme Preset:", 0);
            hComboTheme = makeCombo(210, 132, 380, 240, ID_COMBO_THEME, kThemeOptions, ARRAYSIZE(kThemeOptions), 0);

            makeLabel(32, 175, 180, 20, L"Size Scale:", 0);
            hComboScale = makeCombo(210, 172, 380, 240, ID_COMBO_SCALE, kScaleOptions, ARRAYSIZE(kScaleOptions), 0);

            makeLabel(32, 215, 180, 20, L"Pill Opacity:", 0);
            hComboOpacity = makeCombo(210, 212, 380, 240, ID_COMBO_OPACITY, kOpacityOptions, ARRAYSIZE(kOpacityOptions), 0);

            hChkStartup = makeCheck(32, 265, 300, 24, ID_CHK_STARTUP, L"🚀 Launch automatically on Windows Startup", 0);
            hChkLog = makeCheck(32, 295, 300, 24, ID_CHK_LOG, L"📝 Enable Debug Log (%APPDATA%\\DynamicIsland)", 0);

            // Page 1: Glass & Effects
            hChkRealGlass = makeCheck(32, 55, 540, 24, ID_CHK_REAL_GLASS, L"💎 Real Liquid Glass (Live Desktop Optical Blur & Lens Refraction)", 1);
            hChkBloom = makeCheck(32, 95, 540, 24, ID_CHK_BLOOM, L"🎵 Dynamic Album Art Bloom (Glass Tint washes with Spotify/Music)", 1);
            hChkShadow = makeCheck(32, 135, 540, 24, ID_CHK_SHADOW, L"🌑 Soft Curved Drop Shadow", 1);
            hChkBorder = makeCheck(32, 175, 540, 24, ID_CHK_BORDER, L"✨ Dual-Tone Liquid Glass Specular Hairline Rim", 1);

            // Page 2: Behavior & Animation
            makeLabel(32, 55, 180, 20, L"Animation Physics:", 2);
            hComboAnim = makeCombo(210, 52, 380, 240, ID_COMBO_ANIM, kAnimOptions, ARRAYSIZE(kAnimOptions), 2);

            hChkHover = makeCheck(32, 105, 540, 24, ID_CHK_HOVER, L"🖱️ Expand on Mouse Hover (vs Click to Expand)", 2);
            hChkAutohideFs = makeCheck(32, 145, 540, 24, ID_CHK_AUTOHIDE_FS, L"🖥️ Auto-hide when Fullscreen Apps/Games are active", 2);

            // Page 3: Active Modules
            hChkModMedia = makeCheck(32, 55, 260, 24, ID_CHK_MOD_MEDIA, L"🎵 Media Player & Waveform", 3);
            hChkModLyrics = makeCheck(320, 55, 260, 24, ID_CHK_MOD_LYRICS, L"🎤 Live Synced Lyrics", 3);
            hChkModCalendar = makeCheck(32, 95, 260, 24, ID_CHK_MOD_CALENDAR, L"📅 Calendar & Date View", 3);
            hChkModWeather = makeCheck(320, 95, 260, 24, ID_CHK_MOD_WEATHER, L"☀️ Weather Info (wttr.in)", 3);
            hChkModHardware = makeCheck(32, 135, 260, 24, ID_CHK_MOD_HARDWARE, L"📊 Hardware Monitor (CPU/RAM/GPU)", 3);
            hChkModTray = makeCheck(320, 135, 260, 24, ID_CHK_MOD_TRAY, L"📁 File Tray Shelf", 3);
            hChkModLookup = makeCheck(32, 175, 540, 24, ID_CHK_MOD_LOOKUP, L"🔍 Quick Lookup & App Launcher (Ctrl+Alt+Space)", 3);

            // Bottom action buttons
            HWND btnOpenIni = CreateWindowExW(0, L"BUTTON", L"Open INI", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                              12, 442, 90, 28, hwnd, reinterpret_cast<HMENU>(ID_BTN_OPEN_INI), nullptr, nullptr);
            SendMessageW(btnOpenIni, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND btnApply = CreateWindowExW(0, L"BUTTON", L"Apply", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                            350, 442, 80, 28, hwnd, reinterpret_cast<HMENU>(ID_BTN_APPLY), nullptr, nullptr);
            SendMessageW(btnApply, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND btnOk = CreateWindowExW(0, L"BUTTON", L"Save & OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                         440, 442, 95, 28, hwnd, reinterpret_cast<HMENU>(ID_BTN_OK), nullptr, nullptr);
            SendMessageW(btnOk, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND btnCancel = CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                             545, 442, 65, 28, hwnd, reinterpret_cast<HMENU>(ID_BTN_CANCEL), nullptr, nullptr);
            SendMessageW(btnCancel, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

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
            } else if (id == ID_BTN_OK) {
                SaveValuesFromUi(hwnd);
                DestroyWindow(hwnd);
            } else if (id == ID_BTN_CANCEL) {
                DestroyWindow(hwnd);
            } else if (id == ID_BTN_OPEN_INI) {
                standalone_app::EnsureSettingsFile();
                ShellExecuteW(nullptr, L"open", L"notepad.exe", (L"\"" + GetIniFilePath() + L"\"").c_str(), nullptr, SW_SHOWNORMAL);
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
            for (int p = 0; p < 4; ++p) {
                g_pageControls[p].clear();
            }
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

    const int width = 640;
    const int height = 525;
    HMONITOR hMon = MonitorFromWindow(hParent ? hParent : GetDesktopWindow(), MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = {sizeof(mi)};
    GetMonitorInfoW(hMon, &mi);
    const int posX = mi.rcWork.left + (mi.rcWork.right - mi.rcWork.left - width) / 2;
    const int posY = mi.rcWork.top + (mi.rcWork.bottom - mi.rcWork.top - height) / 2;

    HWND hwnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        wc.lpszClassName,
        L"Dynamic Island — Settings & Customization",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
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

}  // namespace settings_ui
