// notification_history.h - Bounded, privacy-first notification history for Dynamic Edge v1.7.0
#pragma once
#include <windows.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <mutex>
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>
#include "notification_policy.h"

namespace island_notification {

inline NotificationCategory SafeCategoryFromInt(int val) {
    if (val >= 0 && val <= static_cast<int>(NotificationCategory::GeneralStatus)) {
        return static_cast<NotificationCategory>(val);
    }
    return NotificationCategory::GeneralStatus;
}

inline NotificationPriority SafePriorityFromInt(int val) {
    if (val >= 0 && val <= static_cast<int>(NotificationPriority::Critical)) {
        return static_cast<NotificationPriority>(val);
    }
    return NotificationPriority::Normal;
}

inline std::wstring EscapeText(const std::wstring& s) {
    std::wstring out;
    out.reserve(s.size() + 8);
    for (wchar_t ch : s) {
        if (ch == L'\\') {
            out += L"\\\\";
        } else if (ch == L'|') {
            out += L"\\p";
        } else if (ch == L'\r') {
            out += L"\\r";
        } else if (ch == L'\n') {
            out += L"\\n";
        } else {
            out += ch;
        }
    }
    return out;
}

inline std::wstring UnescapeText(const std::wstring& s) {
    std::wstring out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == L'\\' && i + 1 < s.size()) {
            wchar_t next = s[i + 1];
            if (next == L'\\') {
                out += L'\\';
                ++i;
            } else if (next == L'p') {
                out += L'|';
                ++i;
            } else if (next == L'r') {
                out += L'\r';
                ++i;
            } else if (next == L'n') {
                out += L'\n';
                ++i;
            } else {
                out += next;
                ++i;
            }
        } else {
            out += s[i];
        }
    }
    return out;
}

class NotificationHistory {
private:
    std::vector<NotificationItem> history_;
    mutable std::mutex mutex_;
    bool enabled_ = false;       // Opt-in privacy default
    bool paused_ = false;        // Temporary pause
    size_t maxEntries_ = 100;
    DWORD maxRetentionDays_ = 7;
    uint64_t nextId_ = 1;

    static std::wstring GetHistoryFilePath() {
        wchar_t path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path))) {
            return std::wstring(path) + L"\\DynamicIsland\\notification_history.txt";
        }
        return L"notification_history.txt";
    }

    bool SaveToFileLocked(const std::wstring& targetPath = L"") const {
        std::wstring path = targetPath.empty() ? GetHistoryFilePath() : targetPath;
        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return false;

        for (const auto& it : history_) {
            std::wstring line = std::to_wstring(it.id) + L"|" +
                                std::to_wstring(it.timestampWallClock) + L"|" +
                                EscapeText(it.sourceApp) + L"|" +
                                EscapeText(it.title) + L"|" +
                                EscapeText(it.message) + L"|" +
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
            nextId_ = 1;
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

        NotificationItem it = item;
        if (it.id == 0) {
            it.id = nextId_++;
        } else {
            nextId_ = std::max(nextId_, it.id + 1);
        }
        if (it.timestampWallClock == 0) {
            it.timestampWallClock = static_cast<uint64_t>(time(nullptr));
        }
        if (it.timestampTick == 0) {
            it.timestampTick = GetTickCount64();
        }

        history_.push_back(it);
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

        uint64_t nowWall = static_cast<uint64_t>(time(nullptr));
        uint64_t maxAgeSec = static_cast<uint64_t>(maxRetentionDays_) * 86400ULL;

        while (std::getline(ss, line)) {
            if (!line.empty() && line.back() == L'\r') {
                line.pop_back();
            }
            if (line.empty()) continue;

            // Parse: id|timestampWallClock|source|title|msg|cat|pri|dismissed
            std::vector<std::wstring> tokens;
            size_t start = 0, end = 0;
            while ((end = line.find(L'|', start)) != std::wstring::npos) {
                tokens.push_back(line.substr(start, end - start));
                start = end + 1;
            }
            tokens.push_back(line.substr(start));

            if (tokens.size() >= 8) {
                // New format: id|timestampWallClock|sourceApp|title|message|category|priority|dismissed
                NotificationItem it;
                try { it.id = std::stoull(tokens[0]); } catch (...) { it.id = nextId_++; }
                try { it.timestampWallClock = std::stoull(tokens[1]); } catch (...) { it.timestampWallClock = nowWall; }
                it.sourceApp = UnescapeText(tokens[2]);
                it.title = UnescapeText(tokens[3]);
                it.message = UnescapeText(tokens[4]);
                try {
                    it.category = SafeCategoryFromInt(std::stoi(tokens[5]));
                } catch (...) {
                    it.category = NotificationCategory::GeneralStatus;
                }
                try {
                    it.priority = SafePriorityFromInt(std::stoi(tokens[6]));
                } catch (...) {
                    it.priority = NotificationPriority::Normal;
                }
                it.dismissed = (tokens[7] == L"1");
                it.timestampTick = GetTickCount64();

                // Validate and sanitize timestamp
                if (it.timestampWallClock > nowWall + 86400ULL) {
                    // Implausibly far in the future (>24h): reject corrupted record
                    continue;
                } else if (it.timestampWallClock > nowWall) {
                    // Minor clock drift / timezone shift: normalize to current wall-clock
                    it.timestampWallClock = nowWall;
                } else if (it.timestampWallClock == 0) {
                    it.timestampWallClock = nowWall;
                }

                // Wall-clock retention check: filter out expired entries across reboots
                if ((nowWall - it.timestampWallClock) > maxAgeSec) {
                    continue;
                }

                nextId_ = std::max(nextId_, it.id + 1);
                history_.push_back(it);
            } else if (tokens.size() >= 4) {
                // Legacy format: timestampTick|sourceApp|title|message[|cat|pri|dismissed]
                NotificationItem it;
                it.id = nextId_++;
                it.timestampWallClock = nowWall;
                try { it.timestampTick = std::stoull(tokens[0]); } catch (...) { it.timestampTick = GetTickCount64(); }
                it.sourceApp = tokens[1];
                it.title = tokens[2];
                it.message = tokens[3];
                if (tokens.size() > 4) {
                    try { it.category = SafeCategoryFromInt(std::stoi(tokens[4])); } catch (...) { it.category = NotificationCategory::GeneralStatus; }
                }
                if (tokens.size() > 5) {
                    try { it.priority = SafePriorityFromInt(std::stoi(tokens[5])); } catch (...) { it.priority = NotificationPriority::Normal; }
                }
                if (tokens.size() > 6) {
                    it.dismissed = (tokens[6] == L"1");
                }
                history_.push_back(it);
            }

            if (history_.size() >= maxEntries_) break;
        }
        return true;
    }

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
        nextId_ = 1;
        DeleteFileW(GetHistoryFilePath().c_str());
    }

    size_t Count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return history_.size();
    }
};

} // namespace island_notification
