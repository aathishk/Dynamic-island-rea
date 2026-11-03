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

inline const wchar_t* GestureActionToIdString(GestureAction action) {
    switch (action) {
    case GestureAction::ExpandCollapse: return L"expand";
    case GestureAction::ToggleFocusMode: return L"focus";
    case GestureAction::OpenQuickControls: return L"controls";
    case GestureAction::OpenProfileStudio: return L"profile";
    case GestureAction::StartPauseTimer: return L"timer";
    case GestureAction::ToggleVisibility: return L"toggle_vis";
    case GestureAction::OpenSettings: return L"settings";
    case GestureAction::NextMediaTrack: return L"next_track";
    case GestureAction::PreviousMediaTrack: return L"prev_track";
    case GestureAction::CycleTabs: return L"tabs";
    case GestureAction::None: return L"none";
    default: break;
    }
    return L"none";
}

inline GestureAction GestureActionFromIdString(const std::wstring& id) {
    if (_wcsicmp(id.c_str(), L"expand") == 0) return GestureAction::ExpandCollapse;
    if (_wcsicmp(id.c_str(), L"focus") == 0) return GestureAction::ToggleFocusMode;
    if (_wcsicmp(id.c_str(), L"controls") == 0) return GestureAction::OpenQuickControls;
    if (_wcsicmp(id.c_str(), L"profile") == 0) return GestureAction::OpenProfileStudio;
    if (_wcsicmp(id.c_str(), L"timer") == 0) return GestureAction::StartPauseTimer;
    if (_wcsicmp(id.c_str(), L"toggle_vis") == 0) return GestureAction::ToggleVisibility;
    if (_wcsicmp(id.c_str(), L"settings") == 0) return GestureAction::OpenSettings;
    if (_wcsicmp(id.c_str(), L"next_track") == 0) return GestureAction::NextMediaTrack;
    if (_wcsicmp(id.c_str(), L"prev_track") == 0) return GestureAction::PreviousMediaTrack;
    if (_wcsicmp(id.c_str(), L"tabs") == 0) return GestureAction::CycleTabs;
    if (_wcsicmp(id.c_str(), L"none") == 0) return GestureAction::None;
    return GestureAction::None;
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
    GestureAction wheelUpAction_ = GestureAction::None;
    GestureAction wheelDownAction_ = GestureAction::None;

public:
    void SetAction(GestureType gesture, GestureAction action) {
        switch (gesture) {
        case GestureType::SingleClick: singleClickAction_ = action; break;
        case GestureType::DoubleClick: doubleClickAction_ = action; break;
        case GestureType::MiddleClick: middleClickAction_ = action; break;
        case GestureType::RightClick: rightClickAction_ = action; break;
        case GestureType::WheelScrollUp: wheelUpAction_ = action; break;
        case GestureType::WheelScrollDown: wheelDownAction_ = action; break;
        default: break;
        }
    }

    GestureAction GetAction(GestureType gesture) const {
        switch (gesture) {
        case GestureType::SingleClick: return singleClickAction_;
        case GestureType::DoubleClick: return doubleClickAction_;
        case GestureType::MiddleClick: return middleClickAction_;
        case GestureType::RightClick: return rightClickAction_;
        case GestureType::WheelScrollUp: return wheelUpAction_;
        case GestureType::WheelScrollDown: return wheelDownAction_;
        default: return GestureAction::None;
        }
    }

    void ResetToDefaults() {
        singleClickAction_ = GestureAction::ExpandCollapse;
        doubleClickAction_ = GestureAction::OpenQuickControls;
        middleClickAction_ = GestureAction::CycleTabs;
        rightClickAction_ = GestureAction::OpenSettings;
        wheelUpAction_ = GestureAction::None;
        wheelDownAction_ = GestureAction::None;
    }
};

struct GestureExecutionContext {
    int quickControlsTabIndex = -1;
    bool isFocusTimerActive = false;
    bool isPinnedExpanded = false;
    void* userData = nullptr;
    void (*onOpenQuickControls)(int tabIndex, void* user) = nullptr;
    void (*onToggleFocusMode)(bool currentlyActive, void* user) = nullptr;
    void (*onOpenSettings)(void* user) = nullptr;
    void (*onStartPauseTimer)(void* user) = nullptr;
    void (*onCycleTabs)(void* user) = nullptr;
    void (*onMediaCommand)(int cmd, void* user) = nullptr; // 0 = prev, 2 = next
    void (*onToggleExpand)(bool newState, void* user) = nullptr;
    void (*onToggleVisibility)(void* user) = nullptr;
};

enum class GestureDispatchResult {
    Handled,
    IgnoredDisabled,
    NoOp
};

inline GestureDispatchResult ExecuteGestureAction(GestureAction action, const GestureExecutionContext& ctx) {
    switch (action) {
    case GestureAction::OpenQuickControls:
        if (ctx.quickControlsTabIndex >= 0) {
            if (ctx.onOpenQuickControls) ctx.onOpenQuickControls(ctx.quickControlsTabIndex, ctx.userData);
            return GestureDispatchResult::Handled;
        }
        return GestureDispatchResult::IgnoredDisabled;

    case GestureAction::ToggleFocusMode:
        if (ctx.onToggleFocusMode) ctx.onToggleFocusMode(ctx.isFocusTimerActive, ctx.userData);
        return GestureDispatchResult::Handled;

    case GestureAction::OpenSettings:
    case GestureAction::OpenProfileStudio:
        if (ctx.onOpenSettings) ctx.onOpenSettings(ctx.userData);
        return GestureDispatchResult::Handled;

    case GestureAction::StartPauseTimer:
        if (ctx.onStartPauseTimer) ctx.onStartPauseTimer(ctx.userData);
        return GestureDispatchResult::Handled;

    case GestureAction::CycleTabs:
        if (ctx.onCycleTabs) ctx.onCycleTabs(ctx.userData);
        return GestureDispatchResult::Handled;

    case GestureAction::NextMediaTrack:
        if (ctx.onMediaCommand) ctx.onMediaCommand(2, ctx.userData);
        return GestureDispatchResult::Handled;

    case GestureAction::PreviousMediaTrack:
        if (ctx.onMediaCommand) ctx.onMediaCommand(0, ctx.userData);
        return GestureDispatchResult::Handled;

    case GestureAction::ExpandCollapse:
        if (ctx.onToggleExpand) ctx.onToggleExpand(!ctx.isPinnedExpanded, ctx.userData);
        return GestureDispatchResult::Handled;

    case GestureAction::ToggleVisibility:
        if (ctx.onToggleVisibility) ctx.onToggleVisibility(ctx.userData);
        return GestureDispatchResult::Handled;

    case GestureAction::None:
    default:
        // Explicit safe no-op: unknown or unmapped actions must never execute side-effects
        return GestureDispatchResult::NoOp;
    }
}

} // namespace island_gestures
