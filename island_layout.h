// island_layout.h - Adaptive island layout, priority arbitration, and state sizing for Dynamic Edge v1.7.0
#pragma once
#include <windows.h>
#include <algorithm>
#include <cmath>

namespace island_layout {

enum class IslandPresentationState {
    Collapsed,
    CompactMedia,
    CompactTimer,
    NotificationCard,
    ExpandedDashboard,
    QuickControlGrid,
};

struct IslandContentDimensions {
    float width = 180.0f;
    float height = 36.0f;
    float cornerRadius = 18.0f;
};

// Priority arbitration determining which state currently commands the island layout
inline IslandPresentationState ArbitratePresentationState(
    bool isQuickControlsOpen,
    bool hasActiveNotificationCard,
    bool isHoveredOrPinned,
    bool isTimerRunning,
    bool isMediaPlaying) {

    if (isQuickControlsOpen) {
        return IslandPresentationState::QuickControlGrid;
    }
    if (hasActiveNotificationCard) {
        return IslandPresentationState::NotificationCard;
    }
    if (isHoveredOrPinned) {
        return IslandPresentationState::ExpandedDashboard;
    }
    if (isTimerRunning) {
        return IslandPresentationState::CompactTimer;
    }
    if (isMediaPlaying) {
        return IslandPresentationState::CompactMedia;
    }
    return IslandPresentationState::Collapsed;
}

// Calculates adaptive dimensions based on presentation state and user size scale
inline IslandContentDimensions CalculateAdaptiveDimensions(
    IslandPresentationState state,
    float userScale,
    float dpiScale,
    int tabType = 0) {

    float scale = std::clamp(userScale, 0.50f, 2.0f) * std::max(0.5f, dpiScale);
    IslandContentDimensions dims;

    switch (state) {
    case IslandPresentationState::Collapsed:
        dims.width = 160.0f * scale;
        dims.height = 36.0f * scale;
        dims.cornerRadius = dims.height * 0.5f;
        break;

    case IslandPresentationState::CompactMedia:
        dims.width = 220.0f * scale;
        dims.height = 38.0f * scale;
        dims.cornerRadius = dims.height * 0.5f;
        break;

    case IslandPresentationState::CompactTimer:
        dims.width = 200.0f * scale;
        dims.height = 36.0f * scale;
        dims.cornerRadius = dims.height * 0.5f;
        break;

    case IslandPresentationState::NotificationCard:
        dims.width = 320.0f * scale;
        dims.height = 72.0f * scale;
        dims.cornerRadius = 24.0f * scale;
        break;

    case IslandPresentationState::QuickControlGrid:
        dims.width = 380.0f * scale;
        dims.height = 240.0f * scale;
        dims.cornerRadius = 28.0f * scale;
        break;

    case IslandPresentationState::ExpandedDashboard:
    default:
        // Expanded tabs (Media = 380x184, Hardware = 380x140, File Tray = 380x190)
        dims.width = 380.0f * scale;
        dims.height = (tabType == 33 /* File Tray */) ? (190.0f * scale) : (184.0f * scale);
        dims.cornerRadius = 28.0f * scale;
        break;
    }

    return dims;
}

// ── Single Authoritative Navigation Model (Directives Section 4 & 5) ─────────
enum class DashboardPageId {
    Overview = 0,       // Integrated glanceable overview (Time, Date, System, Battery, compact media, privacy)
    Media,              // Dedicated media player (cover art, scrubber, transport, source switch)
    Calendar,           // Month calendar
    Weather,            // Weather details
    HardwareMonitor,    // CPU, RAM, GPU, C: STORAGE, Network telemetry
    FileTray,           // Drag & drop shelf
    Lyrics,             // Full synced lyrics view
    QuickControls,      // Master volume slider & action tiles
    GameOverlay,        // Gaming HUD / performance metrics
    Count
};

struct DashboardPageDescriptor {
    DashboardPageId id;
    const wchar_t* name;
    bool isDefaultEligible;
};

enum class GameOverlayMode {
    DashboardOnly = 0,    // Page in expanded dashboard only (default when enabled)
    CollapsedHud = 1,     // Replaces collapsed strip with HUD
    GamingContextOnly = 2 // Replaces collapsed strip ONLY when gaming context is active
};

// ── Authoritative Media Presentation State Machine (Directive Section 4) ─────
enum class MediaPresentationState {
    Disabled,       // Media module disabled in settings
    NoMedia,        // No SMTC media session or empty title
    Blocked,        // Source process matches media blocklist
    Playing,        // Actively playing audio/video
    PausedGrace,    // Paused within configured timeout grace period
    PausedExpired   // Paused beyond configured timeout
};

inline MediaPresentationState EvaluateMediaPresentationState(
    bool mediaModuleEnabled,
    bool mediaAvailable,
    bool isPlaying,
    bool isBlocked,
    double pausedAt,
    double now,
    int timeoutSeconds) {

    if (!mediaModuleEnabled) {
        return MediaPresentationState::Disabled;
    }
    if (!mediaAvailable) {
        return MediaPresentationState::NoMedia;
    }
    if (isBlocked) {
        return MediaPresentationState::Blocked;
    }
    if (isPlaying) {
        return MediaPresentationState::Playing;
    }

    // Paused / stopped state:
    // If timeoutSeconds <= 0, timeout is disabled (infinite grace period)
    if (timeoutSeconds <= 0) {
        return MediaPresentationState::PausedGrace;
    }

    if (pausedAt > 0.0 && (now - pausedAt) > static_cast<double>(timeoutSeconds)) {
        return MediaPresentationState::PausedExpired;
    }

    return MediaPresentationState::PausedGrace;
}

inline bool IsMediaActiveOrInGrace(MediaPresentationState state) {
    return state == MediaPresentationState::Playing || state == MediaPresentationState::PausedGrace;
}

// Determines if Game Overlay is visible in the collapsed strip or expanded dashboard
inline bool IsGameOverlayVisible(
    bool enabled,
    GameOverlayMode mode,
    bool isExpanded,
    DashboardPageId selectedPage,
    bool inGamingContext,
    bool isPinned,
    bool isFullscreen) {

    if (isFullscreen) return false;
    if (isPinned) return true;
    if (!enabled) return false;

    if (isExpanded) {
        // When expanded, Game Overlay is visible only if selected page is GameOverlay
        return (selectedPage == DashboardPageId::GameOverlay);
    } else {
        // When collapsed:
        switch (mode) {
        case GameOverlayMode::CollapsedHud:
            return true;
        case GameOverlayMode::GamingContextOnly:
            return inGamingContext;
        case GameOverlayMode::DashboardOnly:
        default:
            return false;
        }
    }
}

inline std::vector<DashboardPageDescriptor> BuildActiveDashboardPages(
    bool hasOverview,
    MediaPresentationState mediaState,
    bool calendarEnabled,
    bool weatherEnabled,
    bool hardwareEnabled,
    bool fileTrayEnabled,
    bool lyricsModuleEnabled,
    bool lyricsAvailable,
    bool lyricsPinnedFirst,
    bool quickControlsEnabled,
    bool gameOverlayEnabled
) {
    std::vector<DashboardPageDescriptor> pages;

    // 1. Overview is always available as the glanceable primary dashboard
    if (hasOverview) {
        pages.push_back({DashboardPageId::Overview, L"Overview", true});
    }

    // 2. Lyrics (if pinned first and media is active or within grace period)
    const bool mediaValid = IsMediaActiveOrInGrace(mediaState);
    const bool canShowLyrics = lyricsModuleEnabled && mediaValid && lyricsAvailable;
    if (canShowLyrics && lyricsPinnedFirst) {
        pages.push_back({DashboardPageId::Lyrics, L"Lyrics", true});
    }

    // 3. Media (if media session is valid and active or in paused grace)
    if (mediaValid) {
        pages.push_back({DashboardPageId::Media, L"Media", true});
    }

    // 4. Calendar (if enabled)
    if (calendarEnabled) {
        pages.push_back({DashboardPageId::Calendar, L"Calendar", true});
    }

    // 5. Weather (if enabled)
    if (weatherEnabled) {
        pages.push_back({DashboardPageId::Weather, L"Weather", true});
    }

    // 6. Hardware Monitor (if enabled)
    if (hardwareEnabled) {
        pages.push_back({DashboardPageId::HardwareMonitor, L"Hardware", true});
    }

    // 7. File Tray (if enabled)
    if (fileTrayEnabled) {
        pages.push_back({DashboardPageId::FileTray, L"File Tray", true});
    }

    // 8. Lyrics (if not pinned first and media is active or in paused grace)
    if (canShowLyrics && !lyricsPinnedFirst) {
        pages.push_back({DashboardPageId::Lyrics, L"Lyrics", true});
    }

    // 9. Quick Controls (if enabled)
    if (quickControlsEnabled) {
        pages.push_back({DashboardPageId::QuickControls, L"Controls", true});
    }

    // 10. Game Overlay (if enabled)
    if (gameOverlayEnabled) {
        pages.push_back({DashboardPageId::GameOverlay, L"Game Overlay", true});
    }

    // Fallback: If all optional modules were disabled and hasOverview was false, ensure at least Overview exists
    if (pages.empty()) {
        pages.push_back({DashboardPageId::Overview, L"Overview", true});
    }

    return pages;
}

// Backward-compatible overload for callers passing boolean mediaAvailable / mediaBlocked
inline std::vector<DashboardPageDescriptor> BuildActiveDashboardPages(
    bool hasOverview,
    bool mediaModuleEnabled,
    bool mediaAvailable,
    bool mediaBlocked,
    bool calendarEnabled,
    bool weatherEnabled,
    bool hardwareEnabled,
    bool fileTrayEnabled,
    bool lyricsModuleEnabled,
    bool lyricsAvailable,
    bool lyricsPinnedFirst,
    bool quickControlsEnabled,
    bool gameOverlayEnabled
) {
    MediaPresentationState state = MediaPresentationState::Disabled;
    if (!mediaModuleEnabled) state = MediaPresentationState::Disabled;
    else if (!mediaAvailable) state = MediaPresentationState::NoMedia;
    else if (mediaBlocked) state = MediaPresentationState::Blocked;
    else state = MediaPresentationState::Playing;

    return BuildActiveDashboardPages(
        hasOverview,
        state,
        calendarEnabled,
        weatherEnabled,
        hardwareEnabled,
        fileTrayEnabled,
        lyricsModuleEnabled,
        lyricsAvailable,
        lyricsPinnedFirst,
        quickControlsEnabled,
        gameOverlayEnabled
    );
}

inline int FindPageIndex(const std::vector<DashboardPageDescriptor>& pages, DashboardPageId id) {
    for (size_t i = 0; i < pages.size(); ++i) {
        if (pages[i].id == id) return static_cast<int>(i);
    }
    return -1;
}

inline int NormalizePageIndex(int rawIndex, int pageCount) {
    if (pageCount <= 1) return 0;
    return ((rawIndex % pageCount) + pageCount) % pageCount;
}

} // namespace island_layout
