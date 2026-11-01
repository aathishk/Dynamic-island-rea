// gesture_manager.h - Configurable island gestures and safe quick actions for Dynamic Edge v1.7.0
#pragma once
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <algorithm>

namespace island_gestures {

enum class GestureType {
    SingleClick,
    DoubleClick,
    MiddleClick,
    RightClick,
    WheelScrollUp,
    WheelScrollDown,
};

enum class GestureAction {
    ExpandCollapse,
    ToggleFocusMode,
    OpenQuickControls,
    OpenProfileStudio,
    StartPauseTimer,
    ToggleVisibility,
    OpenSettings,
    NextMediaTrack,
    PreviousMediaTrack,
    CycleTabs,
    None,
};

struct GestureBinding {
    GestureType gesture;
    GestureAction action;
};

inline const wchar_t* GestureActionToString(GestureAction action) {
    switch (action) {
    case GestureAction::ExpandCollapse: return L"Expand / Collapse";
    case GestureAction::ToggleFocusMode: return L"Toggle Focus Mode";
    case GestureAction::OpenQuickControls: return L"Open Quick Controls";
    case GestureAction::OpenProfileStudio: return L"Open Profile Studio";
    case GestureAction::StartPauseTimer: return L"Start / Pause Focus Timer";
    case GestureAction::ToggleVisibility: return L"Toggle Island Visibility";
    case GestureAction::OpenSettings: return L"Open Settings";
    case GestureAction::NextMediaTrack: return L"Next Media Track";
    case GestureAction::PreviousMediaTrack: return L"Previous Media Track";
    case GestureAction::CycleTabs: return L"Cycle Island Tabs";
    case GestureAction::None: return L"Do Nothing";
    }
    return L"None";
}

struct CustomQuickAction {
    std::wstring label;
    std::wstring targetPathOrUri; // Executable path, folder path, or ms-settings URI
    std::wstring arguments;
    bool isUri = false;
};

// Validates and safely executes a user-defined quick action without cmd/powershell
inline bool ExecuteSafeQuickAction(const CustomQuickAction& action, HWND parentHwnd = nullptr) {
    if (action.targetPathOrUri.empty()) return false;

    // Disallow dangerous shell interpreters to avoid arbitrary script injection
    std::wstring lower = action.targetPathOrUri;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);
    if (lower.find(L"cmd.exe") != std::wstring::npos ||
        lower.find(L"powershell.exe") != std::wstring::npos ||
        lower.find(L"pwsh.exe") != std::wstring::npos ||
        lower.find(L"wscript.exe") != std::wstring::npos ||
        lower.find(L"cscript.exe") != std::wstring::npos) {
        return false;
    }

    HINSTANCE hInst = ShellExecuteW(
        parentHwnd,
        L"open",
        action.targetPathOrUri.c_str(),
        action.arguments.empty() ? nullptr : action.arguments.c_str(),
        nullptr,
        SW_SHOWNORMAL
    );
    return (reinterpret_cast<INT_PTR>(hInst) > 32);
}

class GestureManager {
private:
    GestureAction singleClickAction_ = GestureAction::ExpandCollapse;
    GestureAction doubleClickAction_ = GestureAction::OpenQuickControls;
    GestureAction middleClickAction_ = GestureAction::CycleTabs;
    GestureAction rightClickAction_ = GestureAction::OpenSettings;

public:
    void SetAction(GestureType gesture, GestureAction action) {
        switch (gesture) {
        case GestureType::SingleClick: singleClickAction_ = action; break;
        case GestureType::DoubleClick: doubleClickAction_ = action; break;
        case GestureType::MiddleClick: middleClickAction_ = action; break;
        case GestureType::RightClick: rightClickAction_ = action; break;
        default: break;
        }
    }

    GestureAction GetAction(GestureType gesture) const {
        switch (gesture) {
        case GestureType::SingleClick: return singleClickAction_;
        case GestureType::DoubleClick: return doubleClickAction_;
        case GestureType::MiddleClick: return middleClickAction_;
        case GestureType::RightClick: return rightClickAction_;
        default: return GestureAction::None;
        }
    }

    void ResetToDefaults() {
        singleClickAction_ = GestureAction::ExpandCollapse;
        doubleClickAction_ = GestureAction::OpenQuickControls;
        middleClickAction_ = GestureAction::CycleTabs;
        rightClickAction_ = GestureAction::OpenSettings;
    }
};

} // namespace island_gestures
