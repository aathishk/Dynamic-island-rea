// clipboard_manager.h - Privacy-first, opt-in local text clipboard manager for Dynamic Edge v1.7.0
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <mutex>
#include <algorithm>

namespace island_clipboard {

struct ClipboardEntry {
    uint64_t id = 0;
    std::wstring text;
    std::wstring preview;
    bool pinned = false;
    ULONGLONG timestampTick = 0;
};

class ClipboardManager {
private:
    std::vector<ClipboardEntry> entries_;
    mutable std::mutex mutex_;
    bool enabled_ = false;  // Disabled by default (Strict Privacy)
    bool paused_ = false;
    size_t maxItems_ = 50;
    size_t maxTextLength_ = 4096; // 4 KB text limit per entry to prevent memory bloat
    uint64_t nextId_ = 1;
    std::wstring lastCapturedText_;

public:
    void SetEnabled(bool enable) {
        std::lock_guard<std::mutex> lock(mutex_);
        enabled_ = enable;
        if (!enabled_) {
            entries_.clear();
            lastCapturedText_.clear();
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

    void SetMaxItems(size_t maxItems) {
        std::lock_guard<std::mutex> lock(mutex_);
        maxItems_ = std::clamp<size_t>(maxItems, 5, 200);
        PruneUnpinned();
    }

    // Called when clipboard update event occurs (e.g. WM_CLIPBOARDUPDATE)
    bool CaptureFromSystemClipboard(HWND hwndOwner = nullptr) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!enabled_ || paused_) return false;
        }

        if (!OpenClipboard(hwndOwner)) {
            return false;
        }

        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        if (!hData) {
            CloseClipboard();
            return false;
        }

        const wchar_t* pText = static_cast<const wchar_t*>(GlobalLock(hData));
        if (!pText) {
            CloseClipboard();
            return false;
        }

        std::wstring text(pText);
        GlobalUnlock(hData);
        CloseClipboard();

        if (text.empty() || text.find_first_not_of(L" \t\r\n") == std::wstring::npos) {
            return false;
        }

        // Bounded length protection
        if (text.size() > maxTextLength_) {
            text.resize(maxTextLength_);
        }

        std::lock_guard<std::mutex> lock(mutex_);
        if (text == lastCapturedText_) {
            return false; // Skip duplicate
        }
        lastCapturedText_ = text;

        ClipboardEntry entry;
        entry.id = nextId_++;
        entry.text = text;
        entry.pinned = false;
        entry.timestampTick = GetTickCount64();

        // Create single-line clean preview
        entry.preview = text.substr(0, std::min<size_t>(text.size(), 60));
        std::replace(entry.preview.begin(), entry.preview.end(), L'\r', L' ');
        std::replace(entry.preview.begin(), entry.preview.end(), L'\n', L' ');

        entries_.insert(entries_.begin(), entry);
        PruneUnpinned();
        return true;
    }

    // Copies an existing entry back to the Windows system clipboard
    bool CopyToSystemClipboard(uint64_t id, HWND hwndOwner = nullptr) {
        std::wstring textToCopy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& e : entries_) {
                if (e.id == id) {
                    textToCopy = e.text;
                    break;
                }
            }
        }

        if (textToCopy.empty()) return false;
        if (!OpenClipboard(hwndOwner)) return false;

        EmptyClipboard();
        size_t bytes = (textToCopy.size() + 1) * sizeof(wchar_t);
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
        if (!hMem) {
            CloseClipboard();
            return false;
        }

        void* pMem = GlobalLock(hMem);
        if (pMem) {
            memcpy(pMem, textToCopy.c_str(), bytes);
            GlobalUnlock(hMem);
            SetClipboardData(CF_UNICODETEXT, hMem);
        }
        CloseClipboard();
        return true;
    }

    void TogglePin(uint64_t id) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& e : entries_) {
            if (e.id == id) {
                e.pinned = !e.pinned;
                break;
            }
        }
    }

    bool DeleteEntry(uint64_t id) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto it = entries_.begin(); it != entries_.end(); ++it) {
            if (it->id == id) {
                entries_.erase(it);
                return true;
            }
        }
        return false;
    }

    void ClearAll() {
        std::lock_guard<std::mutex> lock(mutex_);
        entries_.clear();
        lastCapturedText_.clear();
    }

    std::vector<ClipboardEntry> GetEntries() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return entries_;
    }

    size_t Count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return entries_.size();
    }

private:
    void PruneUnpinned() {
        if (entries_.size() <= maxItems_) return;
        // Remove oldest unpinned items from the back
        for (int i = static_cast<int>(entries_.size()) - 1; i >= 0 && entries_.size() > maxItems_; --i) {
            if (!entries_[i].pinned) {
                entries_.erase(entries_.begin() + i);
            }
        }
    }
};

} // namespace island_clipboard
