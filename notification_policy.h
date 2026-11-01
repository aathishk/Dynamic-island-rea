// notification_policy.h - Central notification categorization, quiet hours, and arbitration for Dynamic Edge v1.7.0
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>
#include <deque>
#include <memory>

namespace island_notification {

enum class NotificationCategory {
    SystemAlert,
    Battery,
    Bluetooth,
    Media,
    Clipboard,
    Application,
    Productivity,
    GeneralStatus,
};

enum class NotificationPriority {
    Quiet = 0,
    Normal = 1,
    Critical = 2,
};

struct NotificationItem {
    uint64_t id = 0;
    NotificationCategory category = NotificationCategory::GeneralStatus;
    NotificationPriority priority = NotificationPriority::Normal;
    std::wstring title;
    std::wstring message;
    std::wstring sourceApp;
    ULONGLONG timestampTick = 0;
    SYSTEMTIME timestampSystem = {};
    DWORD durationMs = 3500;
    bool dismissed = false;
    std::shared_ptr<const void> iconRef;
};

inline NotificationCategory ClassifyNotification(const std::wstring& app, const std::wstring& title) {
    std::wstring lower = app + L" " + title;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);
    if (lower.find(L"battery") != std::wstring::npos) return NotificationCategory::Battery;
    if (lower.find(L"bluetooth") != std::wstring::npos) return NotificationCategory::Bluetooth;
    if (lower.find(L"media") != std::wstring::npos || lower.find(L"spotify") != std::wstring::npos) return NotificationCategory::Media;
    if (lower.find(L"clipboard") != std::wstring::npos) return NotificationCategory::Clipboard;
    return NotificationCategory::Application;
}

inline NotificationPriority DeterminePriority(NotificationCategory category) {
    if (category == NotificationCategory::Battery || category == NotificationCategory::SystemAlert) {
        return NotificationPriority::Critical;
    }
    return NotificationPriority::Normal;
}

// Checks whether a given hour falls inside quiet hours, supporting midnight wrap-around
inline bool IsInQuietHours(int currentHour, int startHour, int endHour) {
    if (startHour == endHour) {
        return false; // Equal start and end means disabled
    }
    if (startHour < endHour) {
        // e.g. 13:00 to 17:00
        return (currentHour >= startHour && currentHour < endHour);
    } else {
        // Wrap-around midnight: e.g. 22:00 to 07:00
        return (currentHour >= startHour || currentHour < endHour);
    }
}

struct NotificationPolicySettings {
    bool enabled = true;
    bool quietHoursEnabled = false;
    int quietHoursStart = 22; // 22:00
    int quietHoursEnd = 7;    // 07:00
    bool allowCriticalInQuietHours = true;
    bool suppressInFullscreen = true;
    bool suppressInPresentation = true;
    bool duplicateDebounceEnabled = true;
    DWORD duplicateDebounceMs = 3000;
};

// Evaluates whether a notification should be presented on the island immediately,
// or relegated to quiet/history only.
inline bool ShouldPresentNotification(
    const NotificationItem& item,
    const NotificationPolicySettings& settings,
    bool isFullscreenOrGame,
    bool isPresentation,
    int currentHour) {

    if (!settings.enabled) {
        return false;
    }

    // Critical notifications always present unless specifically blocked
    if (item.priority == NotificationPriority::Critical) {
        if (settings.quietHoursEnabled && IsInQuietHours(currentHour, settings.quietHoursStart, settings.quietHoursEnd)) {
            return settings.allowCriticalInQuietHours;
        }
        return true;
    }

    // Quiet priority never interrupts the active screen
    if (item.priority == NotificationPriority::Quiet) {
        return false;
    }

    // Normal priority checks:
    // 1. Fullscreen / Gaming suppression
    if (settings.suppressInFullscreen && isFullscreenOrGame) {
        return false;
    }

    // 2. Presentation suppression
    if (settings.suppressInPresentation && isPresentation) {
        return false;
    }

    // 3. Quiet hours check
    if (settings.quietHoursEnabled && IsInQuietHours(currentHour, settings.quietHoursStart, settings.quietHoursEnd)) {
        return false;
    }

    return true;
}

// Arbitration Queue: maintains priority order, deduplication, and bounds
class NotificationQueue {
private:
    std::deque<NotificationItem> queue_;
    std::wstring lastTitle_;
    std::wstring lastMessage_;
    ULONGLONG lastEventTick_ = 0;
    uint64_t nextId_ = 1;
    size_t maxQueueCapacity_ = 20;

public:
    bool Enqueue(const NotificationItem& incoming) {
        NotificationPolicySettings defSettings;
        return Enqueue(incoming, defSettings);
    }

    bool Enqueue(const NotificationItem& incoming, const NotificationPolicySettings& settings) {
        ULONGLONG now = GetTickCount64();

        // Duplicate de-bounce check
        if (settings.duplicateDebounceEnabled) {
            if (incoming.title == lastTitle_ && incoming.message == lastMessage_) {
                if (now - lastEventTick_ < settings.duplicateDebounceMs) {
                    return false; // Suppress duplicate
                }
            }
        }

        lastTitle_ = incoming.title;
        lastMessage_ = incoming.message;
        lastEventTick_ = now;

        NotificationItem item = incoming;
        item.id = nextId_++;
        item.timestampTick = now;
        GetLocalTime(&item.timestampSystem);

        // Priority insertion: Critical items go to front
        if (item.priority == NotificationPriority::Critical) {
            queue_.push_front(item);
        } else {
            queue_.push_back(item);
        }

        while (queue_.size() > maxQueueCapacity_) {
            queue_.pop_back(); // Drop oldest non-critical item
        }
        return true;
    }

    bool PopNext(NotificationItem& outItem) {
        // Expire items older than 30 seconds
        ULONGLONG now = GetTickCount64();
        while (!queue_.empty()) {
            if (now - queue_.front().timestampTick > 30000ULL) {
                queue_.pop_front();
            } else {
                break;
            }
        }

        if (queue_.empty()) return false;
        outItem = queue_.front();
        queue_.pop_front();
        return true;
    }

    size_t Size() const { return queue_.size(); }
    void Clear() { queue_.clear(); }
};

} // namespace island_notification
