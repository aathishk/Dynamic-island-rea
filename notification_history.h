// notification_history.h - Bounded, privacy-first notification history for Dynamic Edge v1.7.0
#pragma once
#include <windows.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <mutex>
#include <fstream>
#include <sstream>
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
        SaveToFileLocked();
    }

    bool SaveToFile(const std::wstring& targetPath = L"") const {
        std::lock_guard<std::mutex> lock(mutex_);
        return SaveToFileLocked(targetPath);
    }

    bool LoadFromFile(const std::wstring& targetPath = L"") {
        std::wstring path = targetPath.empty() ? GetHistoryFilePath() : targetPath;
        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return false;

        DWORD fileSize = GetFileSize(hFile, nullptr);
        if (fileSize == 0 || fileSize == INVALID_FILE_SIZE) {
            CloseHandle(hFile);
            return false;
        }

        std::vector<wchar_t> buf(fileSize / sizeof(wchar_t) + 1, 0);
        DWORD readBytes = 0;
        ReadFile(hFile, buf.data(), fileSize, &readBytes, nullptr);
        CloseHandle(hFile);

        std::wstringstream ss(buf.data());
        std::wstring line;
        std::lock_guard<std::mutex> lock(mutex_);
        history_.clear();

        ULONGLONG now = GetTickCount64();
        ULONGLONG maxAgeMs = static_cast<ULONGLONG>(maxRetentionDays_) * 86400ULL * 1000ULL;

        while (std::getline(ss, line)) {
            if (line.empty()) continue;
            // Parse: timestamp|source|title|msg|cat|pri|dismissed
            std::vector<std::wstring> tokens;
            size_t start = 0, end = 0;
            while ((end = line.find(L'|', start)) != std::wstring::npos) {
                tokens.push_back(line.substr(start, end - start));
                start = end + 1;
            }
            tokens.push_back(line.substr(start));
            if (tokens.size() < 4) continue;

            ULONGLONG tick = 0;
            try { tick = std::stoull(tokens[0]); } catch (...) { tick = now; }

            // Enforce max retention days: filter out items older than maxRetentionDays_
            if (now >= tick && (now - tick) > maxAgeMs) {
                continue;
            }

            NotificationItem it;
            it.timestampTick = tick;
            it.sourceApp = tokens[1];
            it.title = tokens[2];
            it.message = tokens[3];
            if (tokens.size() > 6) {
                it.dismissed = (tokens[6] == L"1");
            }
            history_.push_back(it);
            if (history_.size() >= maxEntries_) break;
        }
        return true;
    }

private:
    bool SaveToFileLocked(const std::wstring& targetPath = L"") const {
        std::wstring path = targetPath.empty() ? GetHistoryFilePath() : targetPath;
        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return false;

        for (const auto& it : history_) {
            std::wstring line = std::to_wstring(it.timestampTick) + L"|" +
                                it.sourceApp + L"|" +
                                it.title + L"|" +
                                it.message + L"|" +
                                std::to_wstring(static_cast<int>(it.category)) + L"|" +
                                std::to_wstring(static_cast<int>(it.priority)) + L"|" +
                                (it.dismissed ? L"1" : L"0") + L"\r\n";
            DWORD written = 0;
            WriteFile(hFile, line.c_str(), static_cast<DWORD>(line.size() * sizeof(wchar_t)), &written, nullptr);
        }
        CloseHandle(hFile);
        return true;
    }

public:

    std::vector<NotificationItem> GetAllEntries() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return history_;
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
                SaveToFileLocked();
                return true;
            }
        }
        return false;
    }

    bool DismissEntry(uint64_t id) {
        return Dismiss(id);
    }

    bool DeleteEntry(size_t index) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (index < history_.size()) {
            history_.erase(history_.begin() + index);
            SaveToFileLocked();
            return true;
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
