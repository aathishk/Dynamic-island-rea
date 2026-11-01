// clipboard_manager.h - Privacy-first, opt-in local text clipboard manager for Dynamic Edge v1.7.0
#pragma once
#include <windows.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <mutex>
#include <algorithm>
#include <fstream>
#include <sstream>

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
        SaveToFileLocked();
        return true;
    }

    static std::wstring GetHistoryFilePath() {
        wchar_t path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path))) {
            return std::wstring(path) + L"\\DynamicIsland\\clipboard_history.txt";
        }
        return L"clipboard_history.txt";
    }

    // Copies an existing entry back to the Windows system clipboard safely
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

        // Allocate and populate global memory BEFORE opening/emptying system clipboard
        size_t bytes = (textToCopy.size() + 1) * sizeof(wchar_t);
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
        if (!hMem) {
            return false;
        }

        void* pMem = GlobalLock(hMem);
        if (!pMem) {
            GlobalFree(hMem);
            return false;
        }
        memcpy(pMem, textToCopy.c_str(), bytes);
        GlobalUnlock(hMem);

        if (!OpenClipboard(hwndOwner)) {
            GlobalFree(hMem);
            return false;
        }

        EmptyClipboard();
        if (!SetClipboardData(CF_UNICODETEXT, hMem)) {
            GlobalFree(hMem);
            CloseClipboard();
            return false;
        }
        CloseClipboard();
        return true;
    }

    void TogglePin(uint64_t id) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& e : entries_) {
            if (e.id == id) {
                e.pinned = !e.pinned;
                SaveToFileLocked();
                break;
            }
        }
    }

    bool DeleteEntry(uint64_t id) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto it = entries_.begin(); it != entries_.end(); ++it) {
            if (it->id == id) {
                entries_.erase(it);
                SaveToFileLocked();
                return true;
            }
        }
        return false;
    }

    void ClearAll() {
        std::lock_guard<std::mutex> lock(mutex_);
        entries_.clear();
        lastCapturedText_.clear();
        DeleteFileW(GetHistoryFilePath().c_str());
    }

    bool SaveToFile(const std::wstring& targetPath = L"") const {
        std::lock_guard<std::mutex> lock(mutex_);
        return SaveToFileLocked(targetPath);
    }

private:
    static std::wstring EscapeText(const std::wstring& s) {
        std::wstring out;
        out.reserve(s.size() + 16);
        for (wchar_t c : s) {
            if (c == L'\\') {
                out += L"\\\\";
            } else if (c == L'\r') {
                out += L"\\r";
            } else if (c == L'\n') {
                out += L"\\n";
            } else {
                out += c;
            }
        }
        return out;
    }

    static std::wstring UnescapeText(const std::wstring& s) {
        std::wstring out;
        out.reserve(s.size());
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == L'\\' && i + 1 < s.size()) {
                wchar_t next = s[i + 1];
                if (next == L'r') {
                    out += L'\r';
                    ++i;
                } else if (next == L'n') {
                    out += L'\n';
                    ++i;
                } else if (next == L'\\') {
                    out += L'\\';
                    ++i;
                } else {
                    out += s[i];
                }
            } else {
                out += s[i];
            }
        }
        return out;
    }

    bool SaveToFileLocked(const std::wstring& targetPath = L"") const {
        std::wstring path = targetPath.empty() ? GetHistoryFilePath() : targetPath;
        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return false;

        for (const auto& e : entries_) {
            std::wstring line = (e.pinned ? L"PINNED|" : L"ENTRY|") + EscapeText(e.text) + L"\r\n";
            DWORD written = 0;
            WriteFile(hFile, line.c_str(), static_cast<DWORD>(line.size() * sizeof(wchar_t)), &written, nullptr);
        }
        CloseHandle(hFile);
        return true;
    }

public:

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
        entries_.clear();

        while (std::getline(ss, line)) {
            if (line.empty()) continue;
            bool pinned = false;
            std::wstring content = line;
            if (line.rfind(L"PINNED|", 0) == 0) {
                pinned = true;
                content = line.substr(7);
            } else if (line.rfind(L"ENTRY|", 0) == 0) {
                content = line.substr(6);
            }
            if (content.empty()) continue;

            std::wstring fullText = UnescapeText(content);
            if (fullText.size() > maxTextLength_) {
                fullText.resize(maxTextLength_);
            }

            ClipboardEntry e;
            e.id = nextId_++;
            e.text = fullText;
            e.preview = fullText.substr(0, std::min<size_t>(fullText.size(), 60));
            std::replace(e.preview.begin(), e.preview.end(), L'\r', L' ');
            std::replace(e.preview.begin(), e.preview.end(), L'\n', L' ');
            e.pinned = pinned;
            e.timestampTick = GetTickCount64();
            entries_.push_back(e);
            if (entries_.size() >= maxItems_) break;
        }
        return true;
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
