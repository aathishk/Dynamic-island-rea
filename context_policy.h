// context_policy.h - Context evaluation, application rules, and fullscreen/presentation detection for Dynamic Edge v1.7.0
#pragma once
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cwctype>

namespace island_context {

enum class ContextMode {
    Normal,
    Focus,
    Gaming,
    Presentation,
    Custom,
};

inline const wchar_t* ContextModeToString(ContextMode mode) {
    switch (mode) {
    case ContextMode::Normal: return L"Normal";
    case ContextMode::Focus: return L"Focus";
    case ContextMode::Gaming: return L"Gaming";
    case ContextMode::Presentation: return L"Presentation";
    case ContextMode::Custom: return L"Custom";
    }
    return L"Normal";
}

inline ContextMode StringToContextMode(const std::wstring& str) {
    if (_wcsicmp(str.c_str(), L"focus") == 0) return ContextMode::Focus;
    if (_wcsicmp(str.c_str(), L"gaming") == 0) return ContextMode::Gaming;
    if (_wcsicmp(str.c_str(), L"presentation") == 0) return ContextMode::Presentation;
    if (_wcsicmp(str.c_str(), L"custom") == 0) return ContextMode::Custom;
    return ContextMode::Normal;
}

struct AppRule {
    std::wstring processName; // e.g. "Code.exe", "Spotify.exe", "POWERPNT.EXE"
    ContextMode targetContext = ContextMode::Normal;
    std::wstring targetProfile; // Optional profile to apply
    bool enabled = true;
    int priority = 10;          // Higher number = higher precedence
};

// Evaluates application rules against a foreground process name
inline bool MatchAppRule(
    const std::vector<AppRule>& rules,
    const std::wstring& foregroundExeName,
    ContextMode& outContext,
    std::wstring& outProfile) {

    if (foregroundExeName.empty()) return false;

    int highestPriority = -1;
    bool found = false;

    for (const auto& rule : rules) {
        if (!rule.enabled) continue;
        if (_wcsicmp(rule.processName.c_str(), foregroundExeName.c_str()) == 0) {
            if (rule.priority > highestPriority) {
                highestPriority = rule.priority;
                outContext = rule.targetContext;
                outProfile = rule.targetProfile;
                found = true;
            }
        }
    }
    return found;
}

// Queries Windows shell for user notification state (Fullscreen / Presentation)
inline ContextMode DetectShellNotificationState() {
    QUERY_USER_NOTIFICATION_STATE quns;
    if (SUCCEEDED(SHQueryUserNotificationState(&quns))) {
        switch (quns) {
        case QUNS_PRESENTATION_MODE:
            return ContextMode::Presentation;
        case QUNS_RUNNING_D3D_FULL_SCREEN:
            return ContextMode::Gaming;
        case QUNS_BUSY:
            return ContextMode::Gaming;
        case QUNS_ACCEPTS_NOTIFICATIONS:
        default:
            return ContextMode::Normal;
        }
    }
    return ContextMode::Normal;
}

// Context Engine State Tracker
class ContextEngine {
private:
    ContextMode activeMode_ = ContextMode::Normal;
    ContextMode manualLockedMode_ = ContextMode::Normal;
    bool isManualLocked_ = false;
    ULONGLONG lockExpiryTick_ = 0; // 0 = indefinite

    std::wstring activeProfile_ = L"Obsidian";
    std::vector<AppRule> appRules_;
    bool autoContextDetectionEnabled_ = true;

public:
    ContextEngine() {
        // Built-in default application hints
        appRules_.push_back({L"Code.exe", ContextMode::Focus, L"Studio", true, 10});
        appRules_.push_back({L"devenv.exe", ContextMode::Focus, L"Studio", true, 10});
        appRules_.push_back({L"POWERPNT.EXE", ContextMode::Presentation, L"Minimal", true, 20});
        appRules_.push_back({L"Spotify.exe", ContextMode::Normal, L"Aurora", true, 5});
    }

    void SetAutoDetectionEnabled(bool enable) { autoContextDetectionEnabled_ = enable; }
    bool IsAutoDetectionEnabled() const { return autoContextDetectionEnabled_; }

    void SetRules(const std::vector<AppRule>& rules) { appRules_ = rules; }
    const std::vector<AppRule>& GetRules() const { return appRules_; }

    void SetManualLock(ContextMode mode, DWORD durationMinutes = 0) {
        manualLockedMode_ = mode;
        isManualLocked_ = true;
        activeMode_ = mode;
        if (durationMinutes > 0) {
            lockExpiryTick_ = GetTickCount64() + (static_cast<ULONGLONG>(durationMinutes) * 60ULL * 1000ULL);
        } else {
            lockExpiryTick_ = 0;
        }
    }

    void ClearManualLock() {
        isManualLocked_ = false;
        lockExpiryTick_ = 0;
        activeMode_ = ContextMode::Normal;
    }

    bool IsManualLocked() const {
        if (!isManualLocked_) return false;
        if (lockExpiryTick_ > 0 && GetTickCount64() >= lockExpiryTick_) {
            return false;
        }
        return true;
    }

    ContextMode Evaluate(const std::wstring& foregroundExe, std::wstring& outSuggestedProfile) {
        // 1. Manual Lock takes highest precedence
        if (IsManualLocked()) {
            return manualLockedMode_;
        }

        // 2. If auto detection is disabled, remain in manual or normal mode
        if (!autoContextDetectionEnabled_) {
            return activeMode_;
        }

        // 3. Application Rule match
        ContextMode ruleContext;
        std::wstring ruleProfile;
        if (MatchAppRule(appRules_, foregroundExe, ruleContext, ruleProfile)) {
            activeMode_ = ruleContext;
            if (!ruleProfile.empty()) {
                outSuggestedProfile = ruleProfile;
            }
            return activeMode_;
        }

        // 4. Windows shell presentation / fullscreen detection
        ContextMode shellMode = DetectShellNotificationState();
        if (shellMode != ContextMode::Normal) {
            activeMode_ = shellMode;
            return activeMode_;
        }

        activeMode_ = ContextMode::Normal;
        return activeMode_;
    }

    ContextMode GetActiveMode() const { return activeMode_; }
};

} // namespace island_context
