// settings_window.h - Native Windows 11 Dark-Themed Settings & Profile Studio GUI for Dynamic Edge v1.7.0
#pragma once
#include <windows.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <uxtheme.h>
#include <string>
#include <vector>
#include <atomic>
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

inline const SettingOption kContextOptions[] = {
    {L"Normal (Default)", L"normal"},
    {L"Focus (Productivity)", L"focus"},
    {L"Gaming (Silent / Performance)", L"gaming"},
    {L"Presentation (Clean / Suppressed)", L"presentation"},
};

inline const SettingOption kPowerOptions[] = {
    {L"Balanced (Standard - Adaptive)", L"balanced"},
    {L"Battery Saver (Throttle & No Optical Glass)", L"battery_saver"},
    {L"Visual Quality (Full Direct2D Effects)", L"visual_quality"},
};
inline const SettingOption kClickGestureOptions[] = {
    {L"Expand / Collapse", L"expand"},
    {L"Open Quick Controls", L"controls"},
    {L"Toggle Focus Mode", L"focus"},
    {L"Open Profile Studio", L"profile"},
    {L"Open Settings", L"settings"},
    {L"Start / Pause Timer", L"timer"},
    {L"Cycle Island Tabs", L"tabs"},
    {L"Do Nothing", L"none"},
};

inline const SettingOption kMiddleGestureOptions[] = {
    {L"Cycle Island Tabs", L"tabs"},
    {L"Start / Pause Timer", L"timer"},
    {L"Toggle Focus Mode", L"focus"},
    {L"Open Quick Controls", L"controls"},
    {L"Open Settings", L"settings"},
    {L"Expand / Collapse", L"expand"},
    {L"Next Media Track", L"next_track"},
    {L"Previous Media Track", L"prev_track"},
    {L"Do Nothing", L"none"},
};

inline const SettingOption kWheelGestureOptions[] = {
    {L"Do Nothing", L"none"},
    {L"Next Media Track", L"next_track"},
    {L"Previous Media Track", L"prev_track"},
    {L"Cycle Island Tabs", L"tabs"},
    {L"Start / Pause Timer", L"timer"},
    {L"Toggle Focus Mode", L"focus"},
    {L"Expand / Collapse", L"expand"},
    {L"Open Quick Controls", L"controls"},
    {L"Open Settings", L"settings"},
};


// Control IDs
enum CtrlId : int {
    ID_TAB = 2000,

    // Page 0: Overview & Profiles
    ID_COMBO_ACTIVE_PROFILE,
    ID_BTN_APPLY_PROFILE,
    ID_BTN_SAVE_PROFILE,
    ID_BTN_DUPLICATE_PROFILE,
    ID_BTN_DELETE_PROFILE,
    ID_BTN_RENAME_PROFILE,
    ID_BTN_EXPORT_PROFILE,
    ID_BTN_IMPORT_PROFILE,
    ID_BTN_RESET_PROFILE,
    ID_COMBO_ACTIVE_CONTEXT,
    ID_CHK_CONTEXT_LOCK,
    ID_BTN_TOGGLE_ISLAND,
    ID_LBL_OVERVIEW_STATUS,

    // Page 1: Personalisation
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

    // Page 2: Behaviour & Gestures
    ID_CHK_HOVER,
    ID_CHK_AUTOHIDE_FS,
    ID_CHK_AUTO_CONTEXT,
    ID_COMBO_GESTURE_SINGLE,
    ID_COMBO_GESTURE_DOUBLE,
    ID_COMBO_GESTURE_MIDDLE,
    ID_COMBO_GESTURE_WHEEL_UP,
    ID_COMBO_GESTURE_WHEEL_DOWN,

    // Page 3: Notifications & History
    ID_CHK_NOTIF_ENABLED,
    ID_CHK_QUIET_HOURS,
    ID_COMBO_QUIET_START,
    ID_COMBO_QUIET_END,
    ID_CHK_NOTIF_CRITICAL,
    ID_CHK_NOTIF_FS_SUPPRESS,
    ID_CHK_NOTIF_BLE_FILTER,
    ID_CHK_NOTIF_HISTORY,
    ID_BTN_CLEAR_HISTORY,
    ID_BTN_BROWSE_NOTIF_HISTORY,

    // Page 4: Performance & Diagnostics
    ID_COMBO_POWER_POLICY,
    ID_CHK_AUTO_BATTERY,
    ID_LBL_DIAG_METRICS,
    ID_BTN_REFRESH_DIAG,

    // Page 5: Productivity & Modules
    ID_CHK_MOD_MEDIA,
    ID_CHK_MOD_LYRICS,
    ID_CHK_MOD_CALENDAR,
    ID_CHK_MOD_WEATHER,
    ID_CHK_MOD_HARDWARE,
    ID_CHK_MOD_TRAY,
    ID_CHK_MOD_LOOKUP,
    ID_CHK_MOD_FOCUS_TIMER,
    ID_CHK_MOD_CLIPBOARD,
    ID_BTN_CLEAR_CLIPBOARD,
    ID_BTN_BROWSE_CLIPBOARD,
    ID_CHK_STARTUP,
    ID_CHK_LOG,
    ID_BTN_ONBOARDING,
    ID_BTN_RESET_DEFAULTS,

    // Dialog bottom buttons
    ID_BTN_APPLY,
    ID_BTN_OK,
    ID_BTN_CANCEL,
    ID_BTN_OPEN_INI
};

// Controls tracking
inline HWND hTab = nullptr;
inline std::vector<HWND> g_pageControls[6];

// Page 0 controls
namespace dynamic_input_dlg {
    inline wchar_t s_result[256] = {};
    inline bool s_confirmed = false;
    inline HWND s_hEdit = nullptr;

    inline LRESULT CALLBACK InputWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_CREATE: {
            CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
            const wchar_t* pPrompt = reinterpret_cast<const wchar_t*>(cs->lpCreateParams);
            HFONT hFont = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            HWND lbl = CreateWindowExW(0, L"STATIC", pPrompt ? pPrompt : L"Enter value:", WS_CHILD | WS_VISIBLE,
                                       20, 16, 320, 24, hwnd, nullptr, nullptr, nullptr);
            SendMessageW(lbl, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            s_hEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", s_result, WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                                      20, 44, 320, 26, hwnd, reinterpret_cast<HMENU>(101), nullptr, nullptr);
            SendMessageW(s_hEdit, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
            SendMessageW(s_hEdit, EM_SETSEL, 0, -1);
            SetFocus(s_hEdit);

            HWND btnOk = CreateWindowExW(0, L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                                        160, 84, 85, 28, hwnd, reinterpret_cast<HMENU>(IDOK), nullptr, nullptr);
            SendMessageW(btnOk, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND btnCancel = CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                                            255, 84, 85, 28, hwnd, reinterpret_cast<HMENU>(IDCANCEL), nullptr, nullptr);
            SendMessageW(btnCancel, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
            return 0;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == IDOK) {
                if (s_hEdit) {
                    GetWindowTextW(s_hEdit, s_result, ARRAYSIZE(s_result));
                    s_confirmed = true;
                }
                DestroyWindow(hwnd);
                return 0;
            } else if (id == IDCANCEL) {
                s_confirmed = false;
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        }
        case WM_CLOSE:
            s_confirmed = false;
            DestroyWindow(hwnd);
            return 0;
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

inline bool PromptUserString(HWND hwndParent, const wchar_t* title, const wchar_t* prompt, const wchar_t* defaultText, wchar_t* outBuf, size_t maxChars) {
    if (!outBuf || maxChars == 0) return false;
    outBuf[0] = L'\0';

    static bool s_classRegistered = false;
    wcscpy_s(dynamic_input_dlg::s_result, defaultText ? defaultText : L"");
    dynamic_input_dlg::s_confirmed = false;

    HINSTANCE hInst = GetModuleHandleW(nullptr);
    if (!s_classRegistered) {
        WNDCLASSEXW wc = { sizeof(wc) };
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = dynamic_input_dlg::InputWndProc;
        wc.hInstance = hInst;
        wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszClassName = L"DynamicEdgeInputDialog";
        RegisterClassExW(&wc);
        s_classRegistered = true;
    }

    RECT rcParent = {};
    if (hwndParent) GetWindowRect(hwndParent, &rcParent);
    int px = rcParent.left + (rcParent.right - rcParent.left - 380) / 2;
    int py = rcParent.top + (rcParent.bottom - rcParent.top - 160) / 2;

    HWND hDlg = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        L"DynamicEdgeInputDialog",
        title ? title : L"Input",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        px, py, 380, 160,
        hwndParent, nullptr, hInst, const_cast<wchar_t*>(prompt));

    if (!hDlg) return false;

    if (hwndParent) EnableWindow(hwndParent, FALSE);

    MSG msg;
    while (IsWindow(hDlg) && GetMessageW(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
            dynamic_input_dlg::s_confirmed = false;
            DestroyWindow(hDlg);
            break;
        }
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN) {
            if (dynamic_input_dlg::s_hEdit) {
                GetWindowTextW(dynamic_input_dlg::s_hEdit, dynamic_input_dlg::s_result, ARRAYSIZE(dynamic_input_dlg::s_result));
                dynamic_input_dlg::s_confirmed = true;
            }
            DestroyWindow(hDlg);
            break;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (hwndParent) {
        EnableWindow(hwndParent, TRUE);
        SetActiveWindow(hwndParent);
    }

    if (dynamic_input_dlg::s_confirmed) {
        wcsncpy_s(outBuf, maxChars, dynamic_input_dlg::s_result, maxChars - 1);
        return true;
    }
    return false;
}

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
                                      CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
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
                    if (island_globals::g_clipboardManager.CopyToSystemClipboard(s_entries[cur].id, hwnd)) {
                        MessageBoxW(hwnd, L"Text copied to Windows system clipboard!", L"Clipboard", MB_OK | MB_ICONINFORMATION);
                    }
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
            } else if (id == 306) { // Clear All
                if (MessageBoxW(hwnd, L"Clear all clipboard history?", L"Confirm Clear", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    island_globals::g_clipboardManager.ClearAll();
                    RefreshList();
                }
            } else if (id == IDCANCEL) {
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

inline void ShowClipboardHistoryDialog(HWND hwndParent) {
    static bool s_classRegistered = false;
    HINSTANCE hInst = GetModuleHandleW(nullptr);
    if (!s_classRegistered) {
        WNDCLASSEXW wc = { sizeof(wc) };
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = dynamic_clipboard_dlg::ClipboardWndProc;
        wc.hInstance = hInst;
        wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszClassName = L"DynamicEdgeClipboardViewer";
        RegisterClassExW(&wc);
        s_classRegistered = true;
    }

    RECT rcParent = {};
    if (hwndParent) GetWindowRect(hwndParent, &rcParent);
    int px = rcParent.left + (rcParent.right - rcParent.left - 570) / 2;
    int py = rcParent.top + (rcParent.bottom - rcParent.top - 440) / 2;

    HWND hDlg = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        L"DynamicEdgeClipboardViewer",
        L"Dynamic Edge — Clipboard History",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        px, py, 570, 440,
        hwndParent, nullptr, hInst, nullptr);

    if (!hDlg) return;
    if (hwndParent) EnableWindow(hwndParent, FALSE);

    MSG msg;
    while (IsWindow(hDlg) && GetMessageW(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
            DestroyWindow(hDlg);
            break;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    if (hwndParent) {
        EnableWindow(hwndParent, TRUE);
        SetFocus(hwndParent);
    }
}

namespace dynamic_notif_dlg {
    inline HWND s_hList = nullptr;
    inline HWND s_hPreview = nullptr;
    inline std::vector<island_notification::NotificationItem> s_items;

    inline void RefreshList() {
        if (!s_hList || !s_hPreview) return;
        SendMessageW(s_hList, LB_RESETCONTENT, 0, 0);
        s_items = island_globals::g_notificationHistory.GetAllEntries();
        for (const auto& it : s_items) {
            std::wstring display = (it.dismissed ? L"[DISMISSED] " : L"[ACTIVE]    ") +
                                   it.sourceApp + L" - " + it.title;
            SendMessageW(s_hList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(display.c_str()));
        }
        if (!s_items.empty()) {
            SendMessageW(s_hList, LB_SETCURSEL, 0, 0);
            const auto& first = s_items[0];
            std::wstring details = L"Source: " + first.sourceApp + L"\r\n" +
                                   L"Title: " + first.title + L"\r\n" +
                                   L"Status: " + (first.dismissed ? L"Dismissed" : L"Active") + L"\r\n" +
                                   L"Priority: " + (first.priority == island_notification::NotificationPriority::Critical ? L"Critical" : L"Normal") + L"\r\n\r\n" +
                                   L"Message:\r\n" + first.message;
            SetWindowTextW(s_hPreview, details.c_str());
        } else {
            SetWindowTextW(s_hPreview, L"No notification history recorded.");
        }
    }

    inline LRESULT CALLBACK NotifWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            HWND lblList = CreateWindowExW(0, L"STATIC", L"Recorded Notifications:", WS_CHILD | WS_VISIBLE,
                                           16, 12, 300, 20, hwnd, nullptr, nullptr, nullptr);
            SendMessageW(lblList, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            s_hList = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
                                      WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | LBS_NOTIFY,
                                      16, 36, 540, 160, hwnd, reinterpret_cast<HMENU>(401), nullptr, nullptr);
            SendMessageW(s_hList, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND lblPrev = CreateWindowExW(0, L"STATIC", L"Notification Details:", WS_CHILD | WS_VISIBLE,
                                           16, 206, 300, 20, hwnd, nullptr, nullptr, nullptr);
            SendMessageW(lblPrev, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            s_hPreview = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                         WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                                         16, 228, 540, 130, hwnd, reinterpret_cast<HMENU>(402), nullptr, nullptr);
            SendMessageW(s_hPreview, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND btnDismiss = CreateWindowExW(0, L"BUTTON", L"❌ Dismiss", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                                              16, 372, 110, 28, hwnd, reinterpret_cast<HMENU>(403), nullptr, nullptr);
            SendMessageW(btnDismiss, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND btnDel = CreateWindowExW(0, L"BUTTON", L"🗑️ Delete Entry", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                                          136, 372, 120, 28, hwnd, reinterpret_cast<HMENU>(404), nullptr, nullptr);
            SendMessageW(btnDel, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND btnClear = CreateWindowExW(0, L"BUTTON", L"🧹 Clear All", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                                            266, 372, 110, 28, hwnd, reinterpret_cast<HMENU>(405), nullptr, nullptr);
            SendMessageW(btnClear, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND btnClose = CreateWindowExW(0, L"BUTTON", L"Close", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                                            446, 372, 110, 28, hwnd, reinterpret_cast<HMENU>(IDCANCEL), nullptr, nullptr);
            SendMessageW(btnClose, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            RefreshList();
            return 0;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);
            if (id == 401 && code == LBN_SELCHANGE) {
                int cur = static_cast<int>(SendMessageW(s_hList, LB_GETCURSEL, 0, 0));
                if (cur >= 0 && static_cast<size_t>(cur) < s_items.size()) {
                    const auto& item = s_items[cur];
                    std::wstring details = L"Source: " + item.sourceApp + L"\r\n" +
                                           L"Title: " + item.title + L"\r\n" +
                                           L"Status: " + (item.dismissed ? L"Dismissed" : L"Active") + L"\r\n" +
                                           L"Priority: " + (item.priority == island_notification::NotificationPriority::Critical ? L"Critical" : L"Normal") + L"\r\n\r\n" +
                                           L"Message:\r\n" + item.message;
                    SetWindowTextW(s_hPreview, details.c_str());
                }
            } else if (id == 403) { // Dismiss
                int cur = static_cast<int>(SendMessageW(s_hList, LB_GETCURSEL, 0, 0));
                if (cur >= 0 && static_cast<size_t>(cur) < s_items.size()) {
                    island_globals::g_notificationHistory.DismissEntry(s_items[cur].id);
                    RefreshList();
                    SendMessageW(s_hList, LB_SETCURSEL, cur, 0);
                }
            } else if (id == 404) { // Delete
                int cur = static_cast<int>(SendMessageW(s_hList, LB_GETCURSEL, 0, 0));
                if (cur >= 0 && static_cast<size_t>(cur) < s_items.size()) {
                    island_globals::g_notificationHistory.DeleteEntry(static_cast<size_t>(cur));
                    RefreshList();
                }
            } else if (id == 405) { // Clear All
                if (MessageBoxW(hwnd, L"Clear all notification history?", L"Confirm Clear", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    island_globals::g_notificationHistory.ClearAll();
                    RefreshList();
                }
            } else if (id == IDCANCEL) {
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

inline void ShowNotificationHistoryDialog(HWND hwndParent) {
    static bool s_classRegistered = false;
    HINSTANCE hInst = GetModuleHandleW(nullptr);
    if (!s_classRegistered) {
        WNDCLASSEXW wc = { sizeof(wc) };
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = dynamic_notif_dlg::NotifWndProc;
        wc.hInstance = hInst;
        wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszClassName = L"DynamicEdgeNotifHistoryViewer";
        RegisterClassExW(&wc);
        s_classRegistered = true;
    }

    RECT rcParent = {};
    if (hwndParent) GetWindowRect(hwndParent, &rcParent);
    int px = rcParent.left + (rcParent.right - rcParent.left - 590) / 2;
    int py = rcParent.top + (rcParent.bottom - rcParent.top - 450) / 2;

    HWND hDlg = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        L"DynamicEdgeNotifHistoryViewer",
        L"Dynamic Edge — Notification History",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        px, py, 590, 450,
        hwndParent, nullptr, hInst, nullptr);

    if (!hDlg) return;
    if (hwndParent) EnableWindow(hwndParent, FALSE);

    MSG msg;
    while (IsWindow(hDlg) && GetMessageW(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
            DestroyWindow(hDlg);
            break;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    if (hwndParent) {
        EnableWindow(hwndParent, TRUE);
        SetFocus(hwndParent);
    }
}

inline HWND hComboActiveProfile = nullptr;
inline HWND hComboActiveContext = nullptr;
inline HWND hChkContextLock = nullptr;
inline HWND hLblOverviewStatus = nullptr;

// Page 1 controls
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

// Page 2 controls
inline HWND hChkHover = nullptr;
inline HWND hChkAutohideFs = nullptr;
inline HWND hChkAutoContext = nullptr;
inline HWND hComboGestureSingle = nullptr;
inline HWND hComboGestureDouble = nullptr;
inline HWND hComboGestureMiddle = nullptr;
inline HWND hComboGestureWheelUp = nullptr;
inline HWND hComboGestureWheelDown = nullptr;

// Page 3 controls
inline HWND hChkNotifEnabled = nullptr;
inline HWND hChkQuietHours = nullptr;
inline HWND hComboQuietStart = nullptr;
inline HWND hComboQuietEnd = nullptr;
inline HWND hChkNotifCritical = nullptr;
inline HWND hChkNotifFsSuppress = nullptr;
inline HWND hChkNotifBleFilter = nullptr;
inline HWND hChkNotifHistory = nullptr;

// Page 4 controls
inline HWND hComboPowerPolicy = nullptr;
inline HWND hChkAutoBattery = nullptr;
inline HWND hLblDiagMetrics = nullptr;

// Page 5 controls
inline HWND hChkModMedia = nullptr;
inline HWND hChkModLyrics = nullptr;
inline HWND hChkModCalendar = nullptr;
inline HWND hChkModWeather = nullptr;
inline HWND hChkModHardware = nullptr;
inline HWND hChkModTray = nullptr;
inline HWND hChkModLookup = nullptr;
inline HWND hChkModFocusTimer = nullptr;
inline HWND hChkModClipboard = nullptr;
inline HWND hChkStartup = nullptr;
inline HWND hChkLog = nullptr;

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

// Forward declarations
void LoadValuesIntoUi();
void SaveValuesFromUi(HWND hwnd);
void UpdateDiagnosticsDisplay();
void ShowOnboardingDialog(HWND hParent);

inline void PopulateProfilesCombo(HWND cb) {
    SendMessageW(cb, CB_RESETCONTENT, 0, 0);
    auto profiles = island_profile::ProfileManager::EnumerateAllProfiles();
    for (const auto& p : profiles) {
        std::wstring item = p.name;
        if (p.isBuiltIn) {
            item += L" (Built-in)";
        }
        SendMessageW(cb, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item.c_str()));
    }

    std::wstring active = island_profile::ProfileManager::GetActiveProfileName(GetIniFilePath());
    for (size_t i = 0; i < profiles.size(); ++i) {
        if (_wcsicmp(profiles[i].name.c_str(), active.c_str()) == 0) {
            SendMessageW(cb, CB_SETCURSEL, i, 0);
            return;
        }
    }
    SendMessageW(cb, CB_SETCURSEL, 0, 0);
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

    PopulateProfilesCombo(hComboActiveProfile);

    // Page 0: Context
    selectCombo(hComboActiveContext, kContextOptions, ARRAYSIZE(kContextOptions), ReadIniString(L"Settings", L"Context.ContextMode", L"normal"));
    SendMessageW(hChkContextLock, BM_SETCHECK, ReadIniInt(L"Settings", L"Context.IsManualLocked", 0) ? BST_CHECKED : BST_UNCHECKED, 0);

    // Page 1: Personalisation
    selectCombo(hComboPos, kPosOptions, ARRAYSIZE(kPosOptions), ReadIniString(L"Settings", L"Appearance.Position", L"top-center"));
    selectCombo(hComboShape, kShapeOptions, ARRAYSIZE(kShapeOptions), ReadIniString(L"Settings", L"Appearance.ShapeStyle", L"notch"));
    selectCombo(hComboTheme, kThemeOptions, ARRAYSIZE(kThemeOptions), ReadIniString(L"Settings", L"Themes.ThemePreset", L"transparent"));
    selectCombo(hComboAnim, kAnimOptions, ARRAYSIZE(kAnimOptions), ReadIniString(L"Settings", L"Animations.AnimationStyle", L"applespring"));
    selectCombo(hComboScale, kScaleOptions, ARRAYSIZE(kScaleOptions), ReadIniString(L"Settings", L"Appearance.SizeScale", L"1.0"));
    selectCombo(hComboOpacity, kOpacityOptions, ARRAYSIZE(kOpacityOptions), ReadIniString(L"Settings", L"Themes.PillOpacity", L"16"));

    SendMessageW(hChkRealGlass, BM_SETCHECK, ReadIniInt(L"Settings", L"Themes.RealLiquidGlass", 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkBloom, BM_SETCHECK, ReadIniInt(L"Settings", L"Themes.MaterialDepth", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkShadow, BM_SETCHECK, ReadIniInt(L"Settings", L"Themes.DropShadow", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkBorder, BM_SETCHECK, ReadIniInt(L"Settings", L"Themes.ContourBorderEnabled", 1) ? BST_CHECKED : BST_UNCHECKED, 0);

    // Page 2: Behaviour & Gestures
    SendMessageW(hChkHover, BM_SETCHECK, ReadIniInt(L"Settings", L"Behavior.ExpandOnHover", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkAutohideFs, BM_SETCHECK, ReadIniInt(L"Settings", L"Behavior.AutoHideFullscreen", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkAutoContext, BM_SETCHECK, ReadIniInt(L"Settings", L"Context.AutoContextSwitching", 1) ? BST_CHECKED : BST_UNCHECKED, 0);

    selectCombo(hComboGestureSingle, kClickGestureOptions, ARRAYSIZE(kClickGestureOptions), ReadIniString(L"Settings", L"Gestures.SingleClick", L"expand"));
    selectCombo(hComboGestureDouble, kClickGestureOptions, ARRAYSIZE(kClickGestureOptions), ReadIniString(L"Settings", L"Gestures.DoubleClick", L"controls"));
    selectCombo(hComboGestureMiddle, kMiddleGestureOptions, ARRAYSIZE(kMiddleGestureOptions), ReadIniString(L"Settings", L"Gestures.MiddleClick", L"tabs"));
    selectCombo(hComboGestureWheelUp, kWheelGestureOptions, ARRAYSIZE(kWheelGestureOptions), ReadIniString(L"Settings", L"Gestures.WheelScrollUp", L"none"));
    selectCombo(hComboGestureWheelDown, kWheelGestureOptions, ARRAYSIZE(kWheelGestureOptions), ReadIniString(L"Settings", L"Gestures.WheelScrollDown", L"none"));

    // Page 3: Notifications
    SendMessageW(hChkNotifEnabled, BM_SETCHECK, ReadIniInt(L"Settings", L"Notifications.Enabled", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkQuietHours, BM_SETCHECK, ReadIniInt(L"Settings", L"Notifications.QuietHoursEnabled", 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hComboQuietStart, CB_SETCURSEL, ReadIniInt(L"Settings", L"Notifications.QuietHoursStart", 22), 0);
    SendMessageW(hComboQuietEnd, CB_SETCURSEL, ReadIniInt(L"Settings", L"Notifications.QuietHoursEnd", 7), 0);
    SendMessageW(hChkNotifCritical, BM_SETCHECK, ReadIniInt(L"Settings", L"Notifications.AllowCriticalInQuietHours", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkNotifFsSuppress, BM_SETCHECK, ReadIniInt(L"Settings", L"Notifications.SuppressInFullscreen", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkNotifBleFilter, BM_SETCHECK, ReadIniInt(L"Settings", L"Notifications.SuppressBleSensorDuplicates", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkNotifHistory, BM_SETCHECK, ReadIniInt(L"Settings", L"Notifications.HistoryEnabled", 0) ? BST_CHECKED : BST_UNCHECKED, 0);

    // Page 4: Performance
    selectCombo(hComboPowerPolicy, kPowerOptions, ARRAYSIZE(kPowerOptions), ReadIniString(L"Settings", L"Performance.PerformanceMode", L"balanced"));
    SendMessageW(hChkAutoBattery, BM_SETCHECK, ReadIniInt(L"Settings", L"Performance.AutoBatterySaver", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    UpdateDiagnosticsDisplay();

    // Page 5: Productivity & Modules
    SendMessageW(hChkModMedia, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.Media", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModLyrics, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.Lyrics", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModCalendar, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.Calendar", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModWeather, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.Weather", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModHardware, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.HardwareMonitorModule", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModTray, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.FileTrayModule", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModLookup, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.QuickLookup", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModFocusTimer, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.FocusTimer", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkModClipboard, BM_SETCHECK, ReadIniInt(L"Settings", L"Modules.Clipboard", 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkStartup, BM_SETCHECK, standalone_app::IsStartupEnabled() ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(hChkLog, BM_SETCHECK, ReadIniInt(L"Settings", L"Debug.Log", 0) ? BST_CHECKED : BST_UNCHECKED, 0);
}

inline void SaveValuesFromUi(HWND hwnd) {
    auto getComboVal = [](HWND cb, const SettingOption* opts, size_t count) -> const wchar_t* {
        int idx = static_cast<int>(SendMessageW(cb, CB_GETCURSEL, 0, 0));
        if (idx >= 0 && static_cast<size_t>(idx) < count) return opts[idx].val;
        return opts[0].val;
    };

    // Page 0: Context
    WriteIniString(L"Settings", L"Context.ContextMode", getComboVal(hComboActiveContext, kContextOptions, ARRAYSIZE(kContextOptions)));
    WriteIniInt(L"Settings", L"Context.IsManualLocked", SendMessageW(hChkContextLock, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);

    // Page 1: Personalisation
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

    // Page 2: Behaviour & Gestures
    const wchar_t* sClickVal = getComboVal(hComboGestureSingle, kClickGestureOptions, ARRAYSIZE(kClickGestureOptions));
    const wchar_t* dClickVal = getComboVal(hComboGestureDouble, kClickGestureOptions, ARRAYSIZE(kClickGestureOptions));
    const wchar_t* mClickVal = getComboVal(hComboGestureMiddle, kMiddleGestureOptions, ARRAYSIZE(kMiddleGestureOptions));
    const wchar_t* wUpVal = getComboVal(hComboGestureWheelUp, kWheelGestureOptions, ARRAYSIZE(kWheelGestureOptions));
    const wchar_t* wDownVal = getComboVal(hComboGestureWheelDown, kWheelGestureOptions, ARRAYSIZE(kWheelGestureOptions));

    WriteIniString(L"Settings", L"Gestures.SingleClick", sClickVal);
    WriteIniString(L"Settings", L"Gestures.DoubleClick", dClickVal);
    WriteIniString(L"Settings", L"Gestures.MiddleClick", mClickVal);
    WriteIniString(L"Settings", L"Gestures.WheelScrollUp", wUpVal);
    WriteIniString(L"Settings", L"Gestures.WheelScrollDown", wDownVal);

    island_globals::g_gestureManager.SetAction(island_gestures::GestureType::SingleClick,
        island_gestures::GestureActionFromIdString(sClickVal));
    island_globals::g_gestureManager.SetAction(island_gestures::GestureType::DoubleClick,
        island_gestures::GestureActionFromIdString(dClickVal));
    island_globals::g_gestureManager.SetAction(island_gestures::GestureType::MiddleClick,
        island_gestures::GestureActionFromIdString(mClickVal));
    island_globals::g_gestureManager.SetAction(island_gestures::GestureType::WheelScrollUp,
        island_gestures::GestureActionFromIdString(wUpVal));
    island_globals::g_gestureManager.SetAction(island_gestures::GestureType::WheelScrollDown,
        island_gestures::GestureActionFromIdString(wDownVal));

    WriteIniInt(L"Settings", L"Behavior.ExpandOnHover", SendMessageW(hChkHover, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Behavior.AutoHideFullscreen", SendMessageW(hChkAutohideFs, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Context.AutoContextSwitching", SendMessageW(hChkAutoContext, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);

    const wchar_t* curCtxMode = getComboVal(hComboActiveContext, kContextOptions, ARRAYSIZE(kContextOptions));
    const bool isCtxLocked = (SendMessageW(hChkContextLock, BM_GETCHECK, 0, 0) == BST_CHECKED);
    island_globals::g_contextEngine.SetAutoDetectionEnabled(SendMessageW(hChkAutoContext, BM_GETCHECK, 0, 0) == BST_CHECKED);
    if (isCtxLocked) {
        island_globals::g_contextEngine.SetManualLock(island_context::StringToContextMode(curCtxMode));
    } else {
        island_globals::g_contextEngine.ClearManualLock();
    }

    // Page 3: Notifications
    WriteIniInt(L"Settings", L"Notifications.Enabled", SendMessageW(hChkNotifEnabled, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Notifications.QuietHoursEnabled", SendMessageW(hChkQuietHours, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Notifications.QuietHoursStart", static_cast<int>(SendMessageW(hComboQuietStart, CB_GETCURSEL, 0, 0)));
    WriteIniInt(L"Settings", L"Notifications.QuietHoursEnd", static_cast<int>(SendMessageW(hComboQuietEnd, CB_GETCURSEL, 0, 0)));
    WriteIniInt(L"Settings", L"Notifications.AllowCriticalInQuietHours", SendMessageW(hChkNotifCritical, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Notifications.SuppressInFullscreen", SendMessageW(hChkNotifFsSuppress, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Notifications.SuppressBleSensorDuplicates", SendMessageW(hChkNotifBleFilter, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Notifications.HistoryEnabled", SendMessageW(hChkNotifHistory, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);

    // Page 4: Performance
    WriteIniString(L"Settings", L"Performance.PerformanceMode", getComboVal(hComboPowerPolicy, kPowerOptions, ARRAYSIZE(kPowerOptions)));
    WriteIniInt(L"Settings", L"Performance.AutoBatterySaver", SendMessageW(hChkAutoBattery, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);

    // Page 5: Productivity & Modules
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
    WriteIniInt(L"Settings", L"Modules.FocusTimer", SendMessageW(hChkModFocusTimer, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
    WriteIniInt(L"Settings", L"Modules.Clipboard", SendMessageW(hChkModClipboard, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);

    // Flush Windows INI cache
    WritePrivateProfileStringW(nullptr, nullptr, nullptr, GetIniFilePath().c_str());

    // Instantly notify live Dynamic Island overlay
    LoadSettings();
    g_layoutDirty = true;
    TriggerNudge();
    if (g_hwnd) {
        PostMessageW(g_hwnd, WM_APP_NEW_EVENT, 0, 0);
    }
}

inline void UpdateDiagnosticsDisplay() {
    if (!hLblDiagMetrics) return;
    static island_performance::DiagnosticsTracker s_tracker;
    island_performance::PowerStateInfo power = island_performance::ReadSystemPowerStatus();
    island_performance::EffectivePowerMode eff = island_performance::DetermineEffectivePowerMode(
        ReadIniString(L"Settings", L"Performance.PerformanceMode", L"balanced"),
        ReadIniInt(L"Settings", L"Performance.AutoBatterySaver", 1) != 0,
        power
    );
    island_performance::ProcessDiagnostics diag = s_tracker.Sample(eff);

    std::wstring workers;
    if (g_workerStatus.media) workers += L"Media ";
    if (g_workerStatus.audio) workers += L"Audio ";
    if (g_workerStatus.bluetooth) workers += L"Bluetooth ";
    if (g_workerStatus.weather) workers += L"Weather ";
    if (g_workerStatus.lyrics) workers += L"Lyrics ";
    if (g_workerStatus.notification) workers += L"Notifications ";
    if (g_workerStatus.keyboard) workers += L"Keyboard ";
    if (g_workerStatus.mouse) workers += L"Mouse ";
    if (workers.empty()) workers = L"None (Idle)";

    wchar_t text[512];
    double memMb = static_cast<double>(diag.workingSetBytes) / (1024.0 * 1024.0);
    swprintf_s(text,
        L"⚡ Process CPU: %.1f%%\n"
        L"🧠 Working Set Memory: %.1f MB (Peak: %.1f MB)\n"
        L"🔋 Power Status: %s (%d%%)\n"
        L"⚙️ Effective Policy: %s (Battery Saver: %s)\n"
        L"⏱️ Application Uptime: %lu seconds\n"
        L"🧩 Auxiliary Workers (Live): %s",
        diag.processCpuPercent,
        memMb,
        static_cast<double>(diag.peakWorkingSetBytes) / (1024.0 * 1024.0),
        power.isAcConnected ? L"AC Power Connected" : L"On Battery",
        power.batteryPercent,
        (eff == island_performance::EffectivePowerMode::BatterySaver) ? L"Battery Saver" :
        (eff == island_performance::EffectivePowerMode::VisualQuality) ? L"Visual Quality" : L"Balanced",
        power.isBatterySaverActive ? L"Active" : L"Inactive",
        diag.uptimeSeconds,
        workers.c_str()
    );
    SetWindowTextW(hLblDiagMetrics, text);
}

inline void ShowTab(int tabIndex) {
    for (int p = 0; p < 6; ++p) {
        const int cmdShow = (p == tabIndex) ? SW_SHOW : SW_HIDE;
        for (HWND h : g_pageControls[p]) {
            ShowWindow(h, cmdShow);
        }
    }
    if (tabIndex == 4) {
        UpdateDiagnosticsDisplay();
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
                                   12, 12, 630, 430, hwnd, reinterpret_cast<HMENU>(ID_TAB),
                                   reinterpret_cast<LPCREATESTRUCT>(lParam)->hInstance, nullptr);
            SendMessageW(hTab, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
            SetWindowTheme(hTab, L"DarkMode_Explorer", nullptr);

            TCITEMW ti = {};
            ti.mask = TCIF_TEXT;
            ti.pszText = const_cast<LPWSTR>(L"🏠 Overview");
            TabCtrl_InsertItem(hTab, 0, &ti);
            ti.pszText = const_cast<LPWSTR>(L"🎨 Personalisation");
            TabCtrl_InsertItem(hTab, 1, &ti);
            ti.pszText = const_cast<LPWSTR>(L"⚡ Behaviour");
            TabCtrl_InsertItem(hTab, 2, &ti);
            ti.pszText = const_cast<LPWSTR>(L"🔔 Notifications");
            TabCtrl_InsertItem(hTab, 3, &ti);
            ti.pszText = const_cast<LPWSTR>(L"🔋 Performance");
            TabCtrl_InsertItem(hTab, 4, &ti);
            ti.pszText = const_cast<LPWSTR>(L"🧩 Productivity");
            TabCtrl_InsertItem(hTab, 5, &ti);

            auto makeLabel = [&](int x, int y, int w, int h, const wchar_t* text, int page) -> HWND {
                HWND lbl = CreateWindowExW(0, L"STATIC", text, WS_CHILD | SS_LEFT, x, y, w, h, hwnd, nullptr, nullptr, nullptr);
                SendMessageW(lbl, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
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

            // ── PAGE 0: Overview & Profiles ─────────────────────────────────────
            makeLabel(32, 55, 200, 20, L"Active Personalisation Profile:", 0);
            hComboActiveProfile = CreateWindowExW(0, WC_COMBOBOXW, L"", WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL, 240, 52, 240, 240, hwnd, reinterpret_cast<HMENU>(ID_COMBO_ACTIVE_PROFILE), nullptr, nullptr);
            SendMessageW(hComboActiveProfile, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
            SetWindowTheme(hComboActiveProfile, L"DarkMode_Explorer", nullptr);
            g_pageControls[0].push_back(hComboActiveProfile);

            makeBtn(490, 50, 120, 28, ID_BTN_APPLY_PROFILE, L"Apply Profile", 0);
            makeBtn(240, 88, 95, 26, ID_BTN_SAVE_PROFILE, L"💾 Save As...", 0);
            makeBtn(340, 88, 95, 26, ID_BTN_RENAME_PROFILE, L"✏️ Rename", 0);
            makeBtn(440, 88, 95, 26, ID_BTN_DUPLICATE_PROFILE, L"📋 Duplicate", 0);
            makeBtn(540, 88, 80, 26, ID_BTN_DELETE_PROFILE, L"🗑️ Delete", 0);
            makeBtn(240, 118, 95, 26, ID_BTN_EXPORT_PROFILE, L"📤 Export...", 0);
            makeBtn(340, 118, 95, 26, ID_BTN_IMPORT_PROFILE, L"📥 Import...", 0);
            makeBtn(440, 118, 140, 26, ID_BTN_RESET_PROFILE, L"↺ Reset Defaults", 0);

            makeLabel(32, 155, 200, 20, L"Active Context Mode:", 0);
            hComboActiveContext = makeCombo(240, 152, 240, 200, ID_COMBO_ACTIVE_CONTEXT, kContextOptions, ARRAYSIZE(kContextOptions), 0);
            hChkContextLock = makeCheck(32, 185, 400, 24, ID_CHK_CONTEXT_LOCK, L"🔒 Lock Context (prevent automatic app detection)", 0);

            makeBtn(32, 220, 200, 32, ID_BTN_TOGGLE_ISLAND, L"👁️ Show / Hide Island", 0);

            hLblOverviewStatus = makeLabel(32, 275, 570, 120,
                L"Dynamic Edge v1.7.0 — Adaptive Desktop Companion\n"
                L"• Six built-in profiles: Obsidian, Frost, Midnight, Aurora, Minimal, Studio\n"
                L"• Adaptive intelligence: Fullscreen suppression, battery saver, and quiet hours\n"
                L"• Native Win32 + Direct2D: Zero cloud dependencies, 100% offline local privacy", 0);

            // ── PAGE 1: Personalisation ─────────────────────────────────────────
            makeLabel(32, 55, 180, 20, L"Screen Position:", 1);
            hComboPos = makeCombo(210, 52, 380, 240, ID_COMBO_POS, kPosOptions, ARRAYSIZE(kPosOptions), 1);

            makeLabel(32, 95, 180, 20, L"Island Shape Style:", 1);
            hComboShape = makeCombo(210, 92, 380, 240, ID_COMBO_SHAPE, kShapeOptions, ARRAYSIZE(kShapeOptions), 1);

            makeLabel(32, 135, 180, 20, L"Theme Preset:", 1);
            hComboTheme = makeCombo(210, 132, 380, 240, ID_COMBO_THEME, kThemeOptions, ARRAYSIZE(kThemeOptions), 1);

            makeLabel(32, 175, 180, 20, L"Size Scale:", 1);
            hComboScale = makeCombo(210, 172, 380, 240, ID_COMBO_SCALE, kScaleOptions, ARRAYSIZE(kScaleOptions), 1);

            makeLabel(32, 215, 180, 20, L"Pill Opacity:", 1);
            hComboOpacity = makeCombo(210, 212, 380, 240, ID_COMBO_OPACITY, kOpacityOptions, ARRAYSIZE(kOpacityOptions), 1);

            makeLabel(32, 255, 180, 20, L"Animation Physics:", 1);
            hComboAnim = makeCombo(210, 252, 380, 240, ID_COMBO_ANIM, kAnimOptions, ARRAYSIZE(kAnimOptions), 1);

            hChkRealGlass = makeCheck(32, 295, 540, 24, ID_CHK_REAL_GLASS, L"💎 Real Liquid Glass (Live Optical Backdrop Blur & Lens Refraction)", 1);
            hChkBloom = makeCheck(32, 325, 540, 24, ID_CHK_BLOOM, L"🎵 Dynamic Album Art Bloom (Glass Tint washes with Music)", 1);
            hChkShadow = makeCheck(32, 355, 540, 24, ID_CHK_SHADOW, L"🌑 Soft Curved Drop Shadow", 1);
            hChkBorder = makeCheck(32, 385, 540, 24, ID_CHK_BORDER, L"✨ Specular Hairline Contour Rim", 1);

            // ── PAGE 2: Behaviour & Gestures ────────────────────────────────────
            hChkHover = makeCheck(32, 45, 540, 24, ID_CHK_HOVER, L"🖱️ Expand on Mouse Hover (vs Click to Expand)", 2);
            hChkAutohideFs = makeCheck(32, 75, 540, 24, ID_CHK_AUTOHIDE_FS, L"🖥️ Auto-hide when Fullscreen Apps/Games are active", 2);
            hChkAutoContext = makeCheck(32, 105, 540, 24, ID_CHK_AUTO_CONTEXT, L"🧠 Enable Application-Aware Context Rules (Code $\\rightarrow$ Focus, etc.)", 2);

            makeLabel(32, 145, 190, 20, L"Single Click Gesture:", 2);
            hComboGestureSingle = makeCombo(230, 142, 350, 180, ID_COMBO_GESTURE_SINGLE, kClickGestureOptions, ARRAYSIZE(kClickGestureOptions), 2);

            makeLabel(32, 180, 190, 20, L"Double Click Gesture:", 2);
            hComboGestureDouble = makeCombo(230, 177, 350, 180, ID_COMBO_GESTURE_DOUBLE, kClickGestureOptions, ARRAYSIZE(kClickGestureOptions), 2);

            makeLabel(32, 215, 190, 20, L"Middle Click Gesture:", 2);
            hComboGestureMiddle = makeCombo(230, 212, 350, 180, ID_COMBO_GESTURE_MIDDLE, kMiddleGestureOptions, ARRAYSIZE(kMiddleGestureOptions), 2);

            makeLabel(32, 250, 190, 20, L"Mouse Wheel Up:", 2);
            hComboGestureWheelUp = makeCombo(230, 247, 350, 180, ID_COMBO_GESTURE_WHEEL_UP, kWheelGestureOptions, ARRAYSIZE(kWheelGestureOptions), 2);

            makeLabel(32, 285, 190, 20, L"Mouse Wheel Down:", 2);
            hComboGestureWheelDown = makeCombo(230, 282, 350, 180, ID_COMBO_GESTURE_WHEEL_DOWN, kWheelGestureOptions, ARRAYSIZE(kWheelGestureOptions), 2);

            // ── PAGE 3: Notifications & History ─────────────────────────────────
            hChkNotifEnabled = makeCheck(32, 55, 540, 24, ID_CHK_NOTIF_ENABLED, L"🔔 Enable Central Notification Engine", 3);
            hChkQuietHours = makeCheck(32, 90, 260, 24, ID_CHK_QUIET_HOURS, L"🌙 Quiet Hours Schedule:", 3);

            makeLabel(300, 92, 40, 20, L"From:", 3);
            hComboQuietStart = CreateWindowExW(0, WC_COMBOBOXW, L"", WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL, 345, 88, 80, 200, hwnd, reinterpret_cast<HMENU>(ID_COMBO_QUIET_START), nullptr, nullptr);
            SendMessageW(hComboQuietStart, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
            SetWindowTheme(hComboQuietStart, L"DarkMode_Explorer", nullptr);
            for (int h = 0; h < 24; ++h) {
                wchar_t b[16]; swprintf_s(b, L"%02d:00", h);
                SendMessageW(hComboQuietStart, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(b));
            }
            g_pageControls[3].push_back(hComboQuietStart);

            makeLabel(440, 92, 30, 20, L"To:", 3);
            hComboQuietEnd = CreateWindowExW(0, WC_COMBOBOXW, L"", WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL, 475, 88, 80, 200, hwnd, reinterpret_cast<HMENU>(ID_COMBO_QUIET_END), nullptr, nullptr);
            SendMessageW(hComboQuietEnd, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
            SetWindowTheme(hComboQuietEnd, L"DarkMode_Explorer", nullptr);
            for (int h = 0; h < 24; ++h) {
                wchar_t b[16]; swprintf_s(b, L"%02d:00", h);
                SendMessageW(hComboQuietEnd, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(b));
            }
            g_pageControls[3].push_back(hComboQuietEnd);

            hChkNotifCritical = makeCheck(32, 130, 540, 24, ID_CHK_NOTIF_CRITICAL, L"⚡ Allow Critical Battery & System Alerts during Quiet Hours", 3);
            hChkNotifFsSuppress = makeCheck(32, 165, 540, 24, ID_CHK_NOTIF_FS_SUPPRESS, L"🚫 Suppress Non-Critical Cards while in Fullscreen or Gaming", 3);
            hChkNotifBleFilter = makeCheck(32, 200, 540, 24, ID_CHK_NOTIF_BLE_FILTER, L"🔍 Suppress Duplicate BLE Peripheral Connection Spams", 3);
            hChkNotifHistory = makeCheck(32, 240, 540, 24, ID_CHK_NOTIF_HISTORY, L"📜 Enable Notification History (Bounded Local Storage, Opt-In)", 3);

            makeBtn(32, 280, 220, 30, ID_BTN_CLEAR_HISTORY, L"🗑️ Clear Notification History", 3);
            makeBtn(270, 280, 220, 30, ID_BTN_BROWSE_NOTIF_HISTORY, L"📜 Browse Notifications...", 3);

            // ── PAGE 4: Performance & Diagnostics ───────────────────────────────
            makeLabel(32, 55, 180, 20, L"Power & Rendering Policy:", 4);
            hComboPowerPolicy = makeCombo(220, 52, 370, 200, ID_COMBO_POWER_POLICY, kPowerOptions, ARRAYSIZE(kPowerOptions), 4);

            hChkAutoBattery = makeCheck(32, 95, 540, 24, ID_CHK_AUTO_BATTERY, L"🔋 Automatically drop to Battery Saver when on DC Battery power", 4);

            HWND grpDiag = CreateWindowExW(0, L"BUTTON", L"Live Measurable Diagnostics", WS_CHILD | BS_GROUPBOX, 25, 135, 580, 210, hwnd, nullptr, nullptr, nullptr);
            SendMessageW(grpDiag, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
            SetWindowTheme(grpDiag, L"DarkMode_Explorer", nullptr);
            g_pageControls[4].push_back(grpDiag);

            hLblDiagMetrics = CreateWindowExW(0, L"STATIC", L"Collecting diagnostics...", WS_CHILD | SS_LEFT, 45, 165, 540, 160, hwnd, reinterpret_cast<HMENU>(ID_LBL_DIAG_METRICS), nullptr, nullptr);
            SendMessageW(hLblDiagMetrics, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
            g_pageControls[4].push_back(hLblDiagMetrics);

            makeBtn(32, 360, 160, 28, ID_BTN_REFRESH_DIAG, L"🔄 Refresh Metrics", 4);

            // ── PAGE 5: Productivity & Modules ──────────────────────────────────
            hChkModFocusTimer = makeCheck(32, 50, 260, 24, ID_CHK_MOD_FOCUS_TIMER, L"⏱️ Focus / Pomodoro Timer", 5);
            hChkModClipboard = makeCheck(320, 50, 280, 24, ID_CHK_MOD_CLIPBOARD, L"📋 Text Clipboard Panel (Opt-In)", 5);
            hChkModMedia = makeCheck(32, 85, 260, 24, ID_CHK_MOD_MEDIA, L"🎵 Media Player & Waveform", 5);
            hChkModLyrics = makeCheck(320, 85, 280, 24, ID_CHK_MOD_LYRICS, L"🎤 Live Synced Lyrics (LRCLIB)", 5);
            hChkModCalendar = makeCheck(32, 120, 260, 24, ID_CHK_MOD_CALENDAR, L"📅 Calendar & Date View", 5);
            hChkModWeather = makeCheck(320, 120, 280, 24, ID_CHK_MOD_WEATHER, L"☀️ Weather Info (wttr.in)", 5);
            hChkModHardware = makeCheck(32, 155, 260, 24, ID_CHK_MOD_HARDWARE, L"📊 Hardware Monitor (CPU/RAM)", 5);
            hChkModTray = makeCheck(320, 155, 280, 24, ID_CHK_MOD_TRAY, L"📁 File Tray Shelf (Drag & Drop)", 5);
            hChkModLookup = makeCheck(32, 190, 540, 24, ID_CHK_MOD_LOOKUP, L"🔍 Quick Lookup & App Launcher (Ctrl+Alt+Space)", 5);

            hChkStartup = makeCheck(32, 235, 300, 24, ID_CHK_STARTUP, L"🚀 Launch automatically on Windows Startup", 5);
            hChkLog = makeCheck(320, 235, 280, 24, ID_CHK_LOG, L"📝 Enable Debug Log (%APPDATA%)", 5);

            makeBtn(32, 280, 240, 30, ID_BTN_ONBOARDING, L"✨ Re-run First-Run Onboarding...", 5);
            makeBtn(320, 280, 220, 30, ID_BTN_CLEAR_CLIPBOARD, L"🗑️ Clear Clipboard History", 5);
            makeBtn(320, 320, 220, 30, ID_BTN_BROWSE_CLIPBOARD, L"📋 Browse Clipboard...", 5);
            makeBtn(32, 330, 240, 30, ID_BTN_RESET_DEFAULTS, L"⚠️ Restore All Factory Defaults", 5);

            // ── Dialog Bottom Buttons ───────────────────────────────────────────
            HWND btnOpenIni = CreateWindowExW(0, L"BUTTON", L"Open INI", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                              12, 452, 90, 28, hwnd, reinterpret_cast<HMENU>(ID_BTN_OPEN_INI), nullptr, nullptr);
            SendMessageW(btnOpenIni, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND btnApply = CreateWindowExW(0, L"BUTTON", L"Apply", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                            360, 452, 80, 28, hwnd, reinterpret_cast<HMENU>(ID_BTN_APPLY), nullptr, nullptr);
            SendMessageW(btnApply, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND btnOk = CreateWindowExW(0, L"BUTTON", L"Save & OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                         450, 452, 95, 28, hwnd, reinterpret_cast<HMENU>(ID_BTN_OK), nullptr, nullptr);
            SendMessageW(btnOk, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND btnCancel = CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                             555, 452, 65, 28, hwnd, reinterpret_cast<HMENU>(ID_BTN_CANCEL), nullptr, nullptr);
            SendMessageW(btnCancel, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            SetWindowTheme(btnOpenIni, L"DarkMode_Explorer", nullptr);
            SetWindowTheme(btnApply, L"DarkMode_Explorer", nullptr);
            SetWindowTheme(btnOk, L"DarkMode_Explorer", nullptr);
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
            } else if (id == ID_BTN_OK) {
                SaveValuesFromUi(hwnd);
                DestroyWindow(hwnd);
            } else if (id == ID_BTN_CANCEL) {
                DestroyWindow(hwnd);
            } else if (id == ID_BTN_OPEN_INI) {
                standalone_app::EnsureSettingsFile();
                ShellExecuteW(nullptr, L"open", L"notepad.exe", (L"\"" + GetIniFilePath() + L"\"").c_str(), nullptr, SW_SHOWNORMAL);
            } else if (id == ID_BTN_APPLY_PROFILE) {
                int cur = static_cast<int>(SendMessageW(hComboActiveProfile, CB_GETCURSEL, 0, 0));
                auto profiles = island_profile::ProfileManager::EnumerateAllProfiles();
                if (cur >= 0 && static_cast<size_t>(cur) < profiles.size()) {
                    island_profile::ProfileManager::ApplyProfileToSettingsIni(profiles[cur], GetIniFilePath());
                    LoadValuesIntoUi();
                    SaveValuesFromUi(hwnd);
                    MessageBoxW(hwnd, (L"Profile '" + profiles[cur].name + L"' applied successfully!").c_str(), L"Profile Studio", MB_OK | MB_ICONINFORMATION);
                }
            } else if (id == ID_BTN_DUPLICATE_PROFILE) {
                int cur = static_cast<int>(SendMessageW(hComboActiveProfile, CB_GETCURSEL, 0, 0));
                auto profiles = island_profile::ProfileManager::EnumerateAllProfiles();
                if (cur >= 0 && static_cast<size_t>(cur) < profiles.size()) {
                    std::wstring newName = profiles[cur].name + L"_Copy";
                    if (island_profile::ProfileManager::DuplicateProfile(profiles[cur].name, newName)) {
                        PopulateProfilesCombo(hComboActiveProfile);
                        MessageBoxW(hwnd, (L"Profile duplicated as '" + newName + L"'").c_str(), L"Profile Studio", MB_OK | MB_ICONINFORMATION);
                    }
                }
            } else if (id == ID_BTN_DELETE_PROFILE) {
                int cur = static_cast<int>(SendMessageW(hComboActiveProfile, CB_GETCURSEL, 0, 0));
                auto profiles = island_profile::ProfileManager::EnumerateAllProfiles();
                if (cur >= 0 && static_cast<size_t>(cur) < profiles.size()) {
                    if (profiles[cur].isBuiltIn) {
                        MessageBoxW(hwnd, L"Built-in profiles cannot be deleted.", L"Profile Studio", MB_OK | MB_ICONWARNING);
                    } else if (MessageBoxW(hwnd, (L"Are you sure you want to delete profile '" + profiles[cur].name + L"'?").c_str(), L"Confirm Delete", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                        island_profile::ProfileManager::DeleteProfile(profiles[cur].name);
                        PopulateProfilesCombo(hComboActiveProfile);
                    }
                }
            } else if (id == ID_BTN_RENAME_PROFILE) {
                int cur = static_cast<int>(SendMessageW(hComboActiveProfile, CB_GETCURSEL, 0, 0));
                auto profiles = island_profile::ProfileManager::EnumerateAllProfiles();
                if (cur >= 0 && static_cast<size_t>(cur) < profiles.size()) {
                    if (profiles[cur].isBuiltIn) {
                        MessageBoxW(hwnd, L"Built-in profiles cannot be renamed.", L"Profile Studio", MB_OK | MB_ICONWARNING);
                    } else {
                        wchar_t newName[64] = {};
                        if (PromptUserString(hwnd, L"Rename Profile", L"Enter new profile name:", profiles[cur].name.c_str(), newName, ARRAYSIZE(newName))) {
                            if (newName[0] != L'\0') {
                                if (island_profile::ProfileManager::RenameProfile(profiles[cur].name, newName)) {
                                    PopulateProfilesCombo(hComboActiveProfile);
                                    MessageBoxW(hwnd, (L"Profile renamed to '" + std::wstring(newName) + L"' successfully!").c_str(), L"Profile Studio", MB_OK | MB_ICONINFORMATION);
                                } else {
                                    MessageBoxW(hwnd, L"Failed to rename profile.", L"Profile Studio", MB_OK | MB_ICONERROR);
                                }
                            }
                        }
                    }
                }
            } else if (id == ID_BTN_EXPORT_PROFILE) {
                int cur = static_cast<int>(SendMessageW(hComboActiveProfile, CB_GETCURSEL, 0, 0));
                auto profiles = island_profile::ProfileManager::EnumerateAllProfiles();
                if (cur >= 0 && static_cast<size_t>(cur) < profiles.size()) {
                    wchar_t path[MAX_PATH] = {};
                    OPENFILENAMEW ofn = { sizeof(ofn) };
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFilter = L"Profile INI (*.ini)\0*.ini\0All Files (*.*)\0*.*\0";
                    ofn.lpstrFile = path;
                    ofn.nMaxFile = MAX_PATH;
                    ofn.Flags = OFN_OVERWRITEPROMPT;
                    wcscpy_s(path, (profiles[cur].name + L".ini").c_str());
                    if (GetSaveFileNameW(&ofn)) {
                        if (island_profile::ProfileManager::ExportProfile(profiles[cur].name, path)) {
                            MessageBoxW(hwnd, L"Profile exported successfully!", L"Profile Studio", MB_OK | MB_ICONINFORMATION);
                        } else {
                            MessageBoxW(hwnd, L"Failed to export profile file.", L"Profile Studio", MB_OK | MB_ICONERROR);
                        }
                    }
                }
            } else if (id == ID_BTN_IMPORT_PROFILE) {
                wchar_t path[MAX_PATH] = {};
                OPENFILENAMEW ofn = { sizeof(ofn) };
                ofn.hwndOwner = hwnd;
                ofn.lpstrFilter = L"Profile INI (*.ini)\0*.ini\0All Files (*.*)\0*.*\0";
                ofn.lpstrFile = path;
                ofn.nMaxFile = MAX_PATH;
                ofn.Flags = OFN_FILEMUSTEXIST;
                if (GetOpenFileNameW(&ofn)) {
                    island_profile::ProfileData imported;
                    if (island_profile::ProfileManager::ImportProfile(path, imported)) {
                        PopulateProfilesCombo(hComboActiveProfile);
                        MessageBoxW(hwnd, (L"Profile '" + imported.name + L"' imported successfully!").c_str(), L"Profile Studio", MB_OK | MB_ICONINFORMATION);
                    }
                }
            } else if (id == ID_BTN_SAVE_PROFILE) {
                int cur = static_cast<int>(SendMessageW(hComboActiveProfile, CB_GETCURSEL, 0, 0));
                auto profiles = island_profile::ProfileManager::EnumerateAllProfiles();
                std::wstring defName = (cur >= 0 && static_cast<size_t>(cur) < profiles.size()) ? (profiles[cur].name + L"_Custom") : L"MyCustomProfile";
                wchar_t customName[64] = {};
                if (PromptUserString(hwnd, L"Save Profile As", L"Enter profile name:", defName.c_str(), customName, ARRAYSIZE(customName)) && customName[0] != L'\0') {
                    island_profile::ProfileData current;
                    current.name = island_profile::SanitizeProfileName(customName);
                    current.description = L"Custom User Profile";
                    current.isBuiltIn = false;

                    // Full state capture across all settings
                    current.themePreset = ReadIniString(L"Settings", L"Themes.ThemePreset", L"transparent");
                    current.position = ReadIniString(L"Settings", L"Appearance.Position", L"top-center");
                    current.shapeStyle = ReadIniString(L"Settings", L"Appearance.ShapeStyle", L"notch");
                    std::wstring scStr = ReadIniString(L"Settings", L"Appearance.SizeScale", L"1.0");
                    try { current.sizeScale = std::stof(scStr); } catch (...) { current.sizeScale = 1.0f; }
                    current.pillOpacity = ReadIniInt(L"Settings", L"Themes.PillOpacity", 16);
                    current.monitorIndex = ReadIniInt(L"Settings", L"Appearance.MonitorIndex", 0);

                    current.animationStyle = ReadIniString(L"Settings", L"Animations.AnimationStyle", L"applespring");
                    current.animationSpeed = ReadIniString(L"Settings", L"Animations.AnimationSpeed", L"normal");
                    current.reducedMotion = (ReadIniInt(L"Settings", L"Animations.ReducedMotion", 0) != 0);

                    current.realLiquidGlass = (ReadIniInt(L"Settings", L"Themes.RealLiquidGlass", 0) != 0);
                    current.dynamicAlbumBloom = (ReadIniInt(L"Settings", L"Themes.DynamicAlbumBloom", 1) != 0);
                    current.tintIntensity = ReadIniInt(L"Settings", L"Themes.TintIntensity", 0);

                    current.performanceMode = ReadIniString(L"Settings", L"Performance.PerformanceMode", L"balanced");
                    current.autoBatterySaver = (ReadIniInt(L"Settings", L"Performance.AutoBatterySaver", 1) != 0);
                    current.contextMode = ReadIniString(L"Settings", L"Context.ContextMode", L"normal");
                    current.autoContextSwitching = (ReadIniInt(L"Settings", L"Context.AutoContextSwitching", 1) != 0);
                    current.autoHideFullscreen = (ReadIniInt(L"Settings", L"Behavior.AutoHideFullscreen", 1) != 0);

                    current.quietHoursEnabled = (ReadIniInt(L"Settings", L"Notifications.QuietHoursEnabled", 0) != 0);
                    current.quietHoursStart = ReadIniInt(L"Settings", L"Notifications.QuietHoursStart", 22);
                    current.quietHoursEnd = ReadIniInt(L"Settings", L"Notifications.QuietHoursEnd", 7);
                    current.suppressBleSensorDuplicates = (ReadIniInt(L"Settings", L"Notifications.SuppressBleSensorDuplicates", 1) != 0);
                    current.notificationDurationMs = 3500;

                    current.moduleMedia = (ReadIniInt(L"Settings", L"Modules.Media", 1) != 0);
                    current.moduleVolume = (ReadIniInt(L"Settings", L"Modules.Volume", 1) != 0);
                    current.moduleBrightness = (ReadIniInt(L"Settings", L"Modules.Brightness", 1) != 0);
                    current.moduleBluetooth = (ReadIniInt(L"Settings", L"Modules.Bluetooth", 1) != 0);
                    current.moduleWeather = (ReadIniInt(L"Settings", L"Modules.Weather", 1) != 0);
                    current.moduleCalendar = (ReadIniInt(L"Settings", L"Modules.Calendar", 1) != 0);
                    current.moduleHardware = (ReadIniInt(L"Settings", L"Modules.HardwareMonitorModule", ReadIniInt(L"Settings", L"Modules.Hardware", 1)) != 0);
                    current.moduleFileTray = (ReadIniInt(L"Settings", L"Modules.FileTrayModule", ReadIniInt(L"Settings", L"Modules.FileTray", 1)) != 0);
                    current.moduleFocusTimer = (ReadIniInt(L"Settings", L"Modules.FocusTimer", 1) != 0);
                    current.moduleClipboard = (ReadIniInt(L"Settings", L"Modules.Clipboard", 0) != 0);
                    current.moduleQuickLookup = (ReadIniInt(L"Settings", L"Modules.QuickLookup", 1) != 0);
                    current.moduleLyrics = (ReadIniInt(L"Settings", L"Modules.Lyrics", 1) != 0);

                    std::wstring targetPath = island_profile::ProfileManager::GetProfileFilePath(current.name);
                    if (island_profile::ProfileManager::SaveProfileToFile(current, targetPath)) {
                        PopulateProfilesCombo(hComboActiveProfile);
                        MessageBoxW(hwnd, (L"Profile saved successfully as '" + current.name + L"'!").c_str(), L"Profile Studio", MB_OK | MB_ICONINFORMATION);
                    } else {
                        MessageBoxW(hwnd, L"Failed to save profile to disk.", L"Profile Studio", MB_OK | MB_ICONERROR);
                    }
                }
            } else if (id == ID_BTN_RESET_PROFILE) {
                island_profile::ProfileData obs = island_profile::CreateBuiltInProfile(island_profile::BuiltInProfileType::Obsidian);
                island_profile::ProfileManager::ApplyProfileToSettingsIni(obs, GetIniFilePath());
                LoadValuesIntoUi();
                SaveValuesFromUi(hwnd);
                MessageBoxW(hwnd, L"Active profile reset to Obsidian factory defaults.", L"Profile Studio", MB_OK | MB_ICONINFORMATION);
            } else if (id == ID_BTN_REFRESH_DIAG) {
                UpdateDiagnosticsDisplay();
            } else if (id == ID_BTN_BROWSE_NOTIF_HISTORY) {
                ShowNotificationHistoryDialog(hwnd);
            } else if (id == ID_BTN_BROWSE_CLIPBOARD) {
                ShowClipboardHistoryDialog(hwnd);
            } else if (id == ID_BTN_CLEAR_HISTORY) {
                island_globals::g_notificationHistory.ClearAll();
                MessageBoxW(hwnd, L"Shared notification history cleared.", L"Notifications", MB_OK | MB_ICONINFORMATION);
            } else if (id == ID_BTN_CLEAR_CLIPBOARD) {
                island_globals::g_clipboardManager.ClearAll();
                MessageBoxW(hwnd, L"Shared clipboard history cleared.", L"Clipboard", MB_OK | MB_ICONINFORMATION);
            } else if (id == ID_BTN_ONBOARDING) {
                ShowOnboardingDialog(hwnd);
            } else if (id == ID_BTN_TOGGLE_ISLAND) {
                if (g_hwnd) {
                    PostMessageW(g_hwnd, WM_HOTKEY, 1 /* ID_HIDE_SHOW_HOTKEY */, 0);
                }
            } else if (id == ID_BTN_RESET_DEFAULTS) {
                if (MessageBoxW(hwnd, L"Are you sure you want to restore all settings to default factory settings?", L"Dynamic Edge", MB_YESNO | MB_ICONWARNING) == IDYES) {
                    DeleteFileW(GetIniFilePath().c_str());
                    standalone_app::EnsureSettingsFile();
                    LoadValuesIntoUi();
                    SaveValuesFromUi(hwnd);
                }
            }
            return 0;
        }

        case WM_CTLCOLORDLG:
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLOREDIT: {
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
            for (int p = 0; p < 6; ++p) {
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

    const int width = 670;
    const int height = 535;
    HMONITOR hMon = MonitorFromWindow(hParent ? hParent : GetDesktopWindow(), MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = {}; mi.cbSize = sizeof(mi);
    GetMonitorInfoW(hMon, &mi);
    const int posX = mi.rcWork.left + (mi.rcWork.right - mi.rcWork.left - width) / 2;
    const int posY = mi.rcWork.top + (mi.rcWork.bottom - mi.rcWork.top - height) / 2;

    HWND hwnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        wc.lpszClassName,
        L"Dynamic Edge — Settings & Profile Studio",
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

// ── FIRST-RUN ONBOARDING DIALOG ──────────────────────────────────────────────
inline LRESULT CALLBACK OnboardingWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hTitleFont = CreateFontW(-18, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Display");
            HFONT hFont = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Variable Text");

            HWND lblTitle = CreateWindowExW(0, L"STATIC", L"Welcome to Dynamic Edge v1.7.0", WS_CHILD | WS_VISIBLE | SS_LEFT, 30, 25, 450, 30, hwnd, nullptr, nullptr, nullptr);
            SendMessageW(lblTitle, WM_SETFONT, reinterpret_cast<WPARAM>(hTitleFont), TRUE);

            HWND lblSubtitle = CreateWindowExW(0, L"STATIC", L"Your adaptive desktop companion for Windows 11. Choose your initial setup in under 30 seconds:", WS_CHILD | WS_VISIBLE | SS_LEFT, 30, 58, 520, 20, hwnd, nullptr, nullptr, nullptr);
            SendMessageW(lblSubtitle, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            // Step 1: Appearance Profile
            HWND lblStep1 = CreateWindowExW(0, L"STATIC", L"1. Initial Personalisation Profile:", WS_CHILD | WS_VISIBLE | SS_LEFT, 30, 95, 250, 20, hwnd, nullptr, nullptr, nullptr);
            SendMessageW(lblStep1, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND cbProfile = CreateWindowExW(0, WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 280, 92, 260, 200, hwnd, reinterpret_cast<HMENU>(3001), nullptr, nullptr);
            SendMessageW(cbProfile, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
            SetWindowTheme(cbProfile, L"DarkMode_Explorer", nullptr);
            SendMessageW(cbProfile, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Obsidian (Crystal Black - Best Battery)"));
            SendMessageW(cbProfile, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Frost (Frosted Optical Glass)"));
            SendMessageW(cbProfile, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Midnight (Deep Navy & Electric Blue)"));
            SendMessageW(cbProfile, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Aurora (Dynamic Color Palette)"));
            SendMessageW(cbProfile, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Minimal (Compact & Battery Saver)"));
            SendMessageW(cbProfile, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Studio (Productivity & Hardware)"));
            SendMessageW(cbProfile, CB_SETCURSEL, 0, 0);

            // Step 2: Screen Position
            HWND lblStep2 = CreateWindowExW(0, L"STATIC", L"2. Screen Edge Placement:", WS_CHILD | WS_VISIBLE | SS_LEFT, 30, 140, 250, 20, hwnd, nullptr, nullptr, nullptr);
            SendMessageW(lblStep2, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND cbPos = CreateWindowExW(0, WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 280, 137, 260, 200, hwnd, reinterpret_cast<HMENU>(3002), nullptr, nullptr);
            SendMessageW(cbPos, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
            SetWindowTheme(cbPos, L"DarkMode_Explorer", nullptr);
            SendMessageW(cbPos, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Top Center (Recommended)"));
            SendMessageW(cbPos, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Top Left"));
            SendMessageW(cbPos, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Top Right"));
            SendMessageW(cbPos, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Bottom Center"));
            SendMessageW(cbPos, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Adaptive Edge Blade (Auto)"));
            SendMessageW(cbPos, CB_SETCURSEL, 0, 0);

            // Step 3: Essentials & Privacy
            HWND lblStep3 = CreateWindowExW(0, L"STATIC", L"3. Active Modules & Privacy:", WS_CHILD | WS_VISIBLE | SS_LEFT, 30, 185, 250, 20, hwnd, nullptr, nullptr, nullptr);
            SendMessageW(lblStep3, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            HWND chkMedia = CreateWindowExW(0, L"BUTTON", L"🎵 Media Player & Waveform", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 50, 215, 230, 22, hwnd, reinterpret_cast<HMENU>(3003), nullptr, nullptr);
            HWND chkBT = CreateWindowExW(0, L"BUTTON", L"🎧 Bluetooth & Controller Alerts", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 300, 215, 250, 22, hwnd, reinterpret_cast<HMENU>(3004), nullptr, nullptr);
            HWND chkTimer = CreateWindowExW(0, L"BUTTON", L"⏱️ Focus / Pomodoro Timer", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 50, 245, 230, 22, hwnd, reinterpret_cast<HMENU>(3005), nullptr, nullptr);
            HWND chkStartup = CreateWindowExW(0, L"BUTTON", L"🚀 Launch on Windows Startup", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 300, 245, 250, 22, hwnd, reinterpret_cast<HMENU>(3006), nullptr, nullptr);

            SendMessageW(chkMedia, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
            SendMessageW(chkBT, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
            SendMessageW(chkTimer, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
            SendMessageW(chkStartup, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
            SetWindowTheme(chkMedia, L"DarkMode_Explorer", nullptr);
            SetWindowTheme(chkBT, L"DarkMode_Explorer", nullptr);
            SetWindowTheme(chkTimer, L"DarkMode_Explorer", nullptr);
            SetWindowTheme(chkStartup, L"DarkMode_Explorer", nullptr);
            SendMessageW(chkMedia, BM_SETCHECK, BST_CHECKED, 0);
            SendMessageW(chkBT, BM_SETCHECK, BST_CHECKED, 0);
            SendMessageW(chkTimer, BM_SETCHECK, BST_CHECKED, 0);
            SendMessageW(chkStartup, BM_SETCHECK, BST_CHECKED, 0);

            HWND lblPrivacy = CreateWindowExW(0, L"STATIC",
                L"🔒 Privacy First: Dynamic Edge operates 100% locally. No analytics, no cloud accounts,\n"
                L"and clipboard history is opt-in and disabled by default until explicitly enabled.",
                WS_CHILD | WS_VISIBLE | SS_LEFT, 30, 285, 520, 36, hwnd, nullptr, nullptr, nullptr);
            SendMessageW(lblPrivacy, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

            // Action buttons
            HWND btnDone = CreateWindowExW(0, L"BUTTON", L"Complete Setup & Launch", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                           340, 345, 200, 34, hwnd, reinterpret_cast<HMENU>(3010), nullptr, nullptr);
            SendMessageW(btnDone, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
            SetWindowTheme(btnDone, L"DarkMode_Explorer", nullptr);

            HWND btnSkip = CreateWindowExW(0, L"BUTTON", L"Skip to Defaults", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                           200, 345, 130, 34, hwnd, reinterpret_cast<HMENU>(3011), nullptr, nullptr);
            SendMessageW(btnSkip, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
            SetWindowTheme(btnSkip, L"DarkMode_Explorer", nullptr);
            return 0;
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == 3010 || id == 3011) {
                // Apply chosen or default profile
                island_profile::ProfileData chosen = island_profile::CreateBuiltInProfile(island_profile::BuiltInProfileType::Obsidian);
                if (id == 3010) {
                    HWND cb = GetDlgItem(hwnd, 3001);
                    int sel = static_cast<int>(SendMessageW(cb, CB_GETCURSEL, 0, 0));
                    switch (sel) {
                    case 0: chosen = island_profile::CreateBuiltInProfile(island_profile::BuiltInProfileType::Obsidian); break;
                    case 1: chosen = island_profile::CreateBuiltInProfile(island_profile::BuiltInProfileType::Frost); break;
                    case 2: chosen = island_profile::CreateBuiltInProfile(island_profile::BuiltInProfileType::Midnight); break;
                    case 3: chosen = island_profile::CreateBuiltInProfile(island_profile::BuiltInProfileType::Aurora); break;
                    case 4: chosen = island_profile::CreateBuiltInProfile(island_profile::BuiltInProfileType::Minimal); break;
                    case 5: chosen = island_profile::CreateBuiltInProfile(island_profile::BuiltInProfileType::Studio); break;
                    }

                    HWND cbP = GetDlgItem(hwnd, 3002);
                    int pSel = static_cast<int>(SendMessageW(cbP, CB_GETCURSEL, 0, 0));
                    if (pSel == 1) chosen.position = L"top-left";
                    else if (pSel == 2) chosen.position = L"top-right";
                    else if (pSel == 3) chosen.position = L"bottom-center";
                    else if (pSel == 4) chosen.position = L"auto";
                    else chosen.position = L"top-center";

                    bool startup = (SendDlgItemMessageW(hwnd, 3006, BM_GETCHECK, 0, 0) == BST_CHECKED);
                    standalone_app::SetStartupEnabled(startup);
                    chosen.moduleMedia = (SendDlgItemMessageW(hwnd, 3003, BM_GETCHECK, 0, 0) == BST_CHECKED);
                    chosen.moduleBluetooth = (SendDlgItemMessageW(hwnd, 3004, BM_GETCHECK, 0, 0) == BST_CHECKED);
                    chosen.moduleFocusTimer = (SendDlgItemMessageW(hwnd, 3005, BM_GETCHECK, 0, 0) == BST_CHECKED);
                }

                island_profile::ProfileManager::ApplyProfileToSettingsIni(chosen, GetIniFilePath());
                WritePrivateProfileStringW(L"Settings", L"App.FirstRunCompleted", L"1", GetIniFilePath().c_str());
                WritePrivateProfileStringW(nullptr, nullptr, nullptr, GetIniFilePath().c_str());

                DestroyWindow(hwnd);
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
            g_hOnboardingWnd = nullptr;
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

inline void ShowOnboardingDialog(HWND hParent) {
    if (g_hOnboardingWnd) {
        ShowWindow(g_hOnboardingWnd, SW_RESTORE);
        SetForegroundWindow(g_hOnboardingWnd);
        return;
    }

    HINSTANCE hInst = GetModuleHandleW(nullptr);
    static HBRUSH s_wndDarkBrush = CreateSolidBrush(RGB(28, 28, 32));
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = OnboardingWndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    wc.hbrBackground = s_wndDarkBrush;
    wc.lpszClassName = L"DynamicIsland.OnboardingDialog";
    RegisterClassExW(&wc);

    const int width = 580;
    const int height = 430;
    HMONITOR hMon = MonitorFromWindow(hParent ? hParent : GetDesktopWindow(), MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = {}; mi.cbSize = sizeof(mi);
    GetMonitorInfoW(hMon, &mi);
    const int posX = mi.rcWork.left + (mi.rcWork.right - mi.rcWork.left - width) / 2;
    const int posY = mi.rcWork.top + (mi.rcWork.bottom - mi.rcWork.top - height) / 2;

    HWND hwnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        wc.lpszClassName,
        L"Welcome to Dynamic Edge",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        posX, posY, width, height,
        nullptr, nullptr, hInst, nullptr
    );

    if (hwnd) {
        g_hOnboardingWnd = hwnd;
        BOOL darkMode = TRUE;
        DwmSetWindowAttribute(hwnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &darkMode, sizeof(darkMode));
        SetForegroundWindow(hwnd);
    }
}

}  // namespace settings_ui
