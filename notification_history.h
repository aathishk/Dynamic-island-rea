// notification_history.h - Bounded, privacy-first notification history for Dynamic Edge v1.7.0
#pragma once
#include <windows.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <mutex>
#include <fstream>
#include "notification_policy.h"

namespace island_notification {

class NotificationHistory {
private:
    std::vector<NotificationItem> history_;
    mutable std::mutex mutex_;
    bool enabled_ = false;       // Opt-in privacy default
    bool paused_ = false;        // Temporary pause
    size_t maxEntries_ = 100;
    DWORD maxRetentionDays_ = 7;

    static std::wstring GetHistoryFilePath() {
        wchar_t path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path))) {
            return std::wstring(path) + L"\\DynamicIsland\\notification_history.txt";
        }
        return L"notification_history.txt";
    }

public:
    void SetMaxRetentionDays(DWORD days) {
        std::lock_guard<std::mutex> lock(mutex_);
        maxRetentionDays_ = days;
    }

    DWORD GetMaxRetentionDays() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return maxRetentionDays_;
    }

    void SetEnabled(bool enable) {
        std::lock_guard<std::mutex> lock(mutex_);
        enabled_ = enable;
        if (!enabled_) {
            history_.clear();
            DeleteFileW(GetHistoryFilePath().c_str());
        }
    }

    bool IsEnabled() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return enabled_;
    }

    void SetPaused(bool pause) {
        std::lock_guard<std::mutex> lock(mutex_);
        paused_ = pause;
    }

    bool IsPaused() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return paused_;
    }

    void SetMaxEntries(size_t maxEntries) {
        std::lock_guard<std::mutex> lock(mutex_);
        maxEntries_ = std::clamp<size_t>(maxEntries, 10, 500);
        while (history_.size() > maxEntries_) {
            history_.erase(history_.begin());
        }
    }

    void Record(const NotificationItem& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!enabled_ || paused_) return;

        history_.push_back(item);
        while (history_.size() > maxEntries_) {
            history_.erase(history_.begin());
        }
    }

    std::vector<NotificationItem> GetRecent(size_t count = 20) const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<NotificationItem> out;
        if (history_.empty()) return out;

        size_t n = std::min(count, history_.size());
        out.reserve(n);
        // Return most recent first
        for (auto it = history_.rbegin(); it != history_.rend() && out.size() < n; ++it) {
            if (!it->dismissed) {
                out.push_back(*it);
            }
        }
        return out;
    }

    bool Dismiss(uint64_t id) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& item : history_) {
            if (item.id == id) {
                item.dismissed = true;
                return true;
            }
        }
        return false;
    }

    void ClearAll() {
        std::lock_guard<std::mutex> lock(mutex_);
        history_.clear();
        DeleteFileW(GetHistoryFilePath().c_str());
    }

    size_t Count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return history_.size();
    }
};

} // namespace island_notification
