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

// ── Single Authoritative Navigation Model (Directives Section 4 & 5) ─────────
enum class DashboardPageId : int {
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

// Calculates adaptive dimensions based on presentation state, active page, and user size scale
inline IslandContentDimensions CalculateAdaptiveDimensions(
    IslandPresentationState state,
    float userScale,
    float dpiScale,
    DashboardPageId activePageId = DashboardPageId::Overview) {

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
        dims.width = 410.0f * scale;
        dims.height = 270.0f * scale;
        dims.cornerRadius = 28.0f * scale;
        break;

    case IslandPresentationState::ExpandedDashboard:
    default:
        // Page-specific adaptive dimensions:
        // Overview & Media: preserve existing compact size (380 x 184)
        // Hardware Monitor, Weather, Calendar, File Tray, Game Overlay: spacious ~410 x 210
        // Quick Controls: approximately 410 x 270
        dims.cornerRadius = 28.0f * scale;
        switch (activePageId) {
        case DashboardPageId::HardwareMonitor:
        case DashboardPageId::Weather:
        case DashboardPageId::Calendar:
        case DashboardPageId::FileTray:
        case DashboardPageId::GameOverlay:
            dims.width = 410.0f * scale;
            dims.height = 210.0f * scale;
            break;

        case DashboardPageId::QuickControls:
            dims.width = 410.0f * scale;
            dims.height = 270.0f * scale;
            break;

        case DashboardPageId::Overview:
        case DashboardPageId::Media:
        case DashboardPageId::Lyrics:
        default:
            dims.width = 380.0f * scale;
            dims.height = 184.0f * scale;
            break;
        }
        break;
    }

    return dims;
}

inline IslandContentDimensions CalculateAdaptiveDimensions(
    IslandPresentationState state,
    float userScale,
    float dpiScale,
    int tabType) {
    DashboardPageId pageId = DashboardPageId::Overview;
    if (tabType == 33 || tabType == static_cast<int>(DashboardPageId::FileTray)) {
        pageId = DashboardPageId::FileTray;
    } else if (tabType >= 0 && tabType < static_cast<int>(DashboardPageId::Count)) {
        pageId = static_cast<DashboardPageId>(tabType);
    }
    return CalculateAdaptiveDimensions(state, userScale, dpiScale, pageId);
}

} // namespace island_layout

#include "island_alpha.h"

namespace island_layout {

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

inline DashboardPageId ParseDashboardPageId(std::wstring_view name) {
    if (name == L"media") return DashboardPageId::Media;
    if (name == L"calendar") return DashboardPageId::Calendar;
    if (name == L"weather") return DashboardPageId::Weather;
    if (name == L"hardware") return DashboardPageId::HardwareMonitor;
    if (name == L"filetray") return DashboardPageId::FileTray;
    if (name == L"lyrics") return DashboardPageId::Lyrics;
    if (name == L"controls") return DashboardPageId::QuickControls;
    if (name == L"game_overlay") return DashboardPageId::GameOverlay;
    return DashboardPageId::Overview;
}

struct SurfaceMaterialProperties {
    float alpha;
    float r;
    float g;
    float b;
    bool isOpaqueBackdrop;
};

inline SurfaceMaterialProperties CalculateSurfaceMaterial(
    bool isExpanded,
    DashboardPageId activePageId,
    float settingsPillOpacity
) {
    SurfaceMaterialProperties p = {};
    p.alpha = island_alpha::CalculateSurfaceOpacity(isExpanded, activePageId, settingsPillOpacity);
    if (!isExpanded || activePageId == DashboardPageId::Media) {
        p.r = 0.04f; p.g = 0.04f; p.b = 0.06f;
        p.isOpaqueBackdrop = false;
    } else {
        p.r = 0.04f; p.g = 0.05f; p.b = 0.07f;
        p.isOpaqueBackdrop = true;
    }
    return p;
}

struct OverviewLayoutMetrics {
    float topY;
    float topH;
    float midY;
    float midH;
    float chipY;
    float chipH;
    float privacyY;
    float privacyH;
    float clockToChipSeparation;
    float totalContentH;
};

inline OverviewLayoutMetrics CalculateOverviewLayoutMetrics(
    float rectTop, float rectBottom, float scale,
    float measuredDateH, float measuredTimeH,
    bool dateFirst = true, bool privacyActive = false
) {
    OverviewLayoutMetrics m = {};
    m.topH = dateFirst ? measuredDateH : measuredTimeH;
    m.midH = dateFirst ? measuredTimeH : measuredDateH;
    const float rowGap = 4.0f * scale;
    const float clockToChipGap = 14.0f * scale;
    m.chipH = 26.0f * scale;
    m.privacyH = 20.0f * scale;
    const float privacyGap = 8.0f * scale;

    m.totalContentH = m.topH + rowGap + m.midH + clockToChipGap + m.chipH;
    if (privacyActive) {
        m.totalContentH += privacyGap + m.privacyH;
    }

    const float availH = rectBottom - rectTop;
    const float startY = rectTop + std::max(12.0f * scale, (availH - m.totalContentH) * 0.44f);

    m.topY = startY;
    m.midY = m.topY + m.topH + rowGap;
    m.chipY = m.midY + m.midH + clockToChipGap;
    m.privacyY = m.chipY + m.chipH + privacyGap;
    m.clockToChipSeparation = m.chipY - (m.midY + m.midH);
    return m;
}

inline std::vector<float> CalculateChipWidths(
    const std::vector<float>& naturalWidths,
    float maxRowW,
    float chipGap,
    float minChipW
) {
    std::vector<float> widths = naturalWidths;
    if (widths.empty()) return widths;

    float totalNaturalW = 0.0f;
    for (float w : widths) totalNaturalW += w;

    const float totalGaps = chipGap * static_cast<float>(widths.size() - 1);
    const float availBoxesW = std::max(0.0f, maxRowW - totalGaps);

    if (totalNaturalW + totalGaps > maxRowW && totalNaturalW > 0.0f) {
        const float allocatedMin = minChipW * static_cast<float>(widths.size());
        if (availBoxesW <= allocatedMin) {
            for (size_t i = 0; i < widths.size(); ++i) widths[i] = minChipW;
        } else {
            float remAvail = availBoxesW - allocatedMin;
            float remNatural = 0.0f;
            for (float w : widths) remNatural += std::max(0.0f, w - minChipW);

            for (size_t i = 0; i < widths.size(); ++i) {
                float extra = std::max(0.0f, widths[i] - minChipW);
                float share = (remNatural > 0.0f) ? (extra / remNatural) : (1.0f / static_cast<float>(widths.size()));
                widths[i] = minChipW + remAvail * share;
            }
        }
    }
    return widths;
}

// ── Quick Controls Grid & Volume Slider Layout & Hit-Testing ──────────────────
struct QuickControlTileRect {
    float left;
    float top;
    float right;
    float bottom;
};

struct QuickControlsLayout {
    float barLeft;
    float barTop;
    float barRight;
    float barBottom;
    float thumbX;
    float thumbY;
    float thumbRadius;
    QuickControlTileRect tiles[12];
    int tileCount;
};

inline QuickControlsLayout CalculateQuickControlsLayout(
    float contentWidth, float contentHeight, float scale, int volumePercent = 50
) {
    (void)contentHeight;
    QuickControlsLayout l = {};
    const float padX = 24.0f * scale;
    l.barLeft = padX;
    l.barRight = contentWidth - padX;
    l.barTop = 42.0f * scale;
    l.barBottom = l.barTop + 8.0f * scale;

    float pct = static_cast<float>(std::clamp(volumePercent, 0, 100)) / 100.0f;
    float fillW = (l.barRight - l.barLeft) * pct;
    l.thumbX = l.barLeft + fillW;
    l.thumbY = (l.barTop + l.barBottom) * 0.5f;
    l.thumbRadius = 7.0f * scale;

    const float gridY = l.barTop + 22.0f * scale;
    const float colGap = 9.0f * scale;
    const float rowGap = 9.0f * scale;
    const float tileW = (l.barRight - l.barLeft - colGap * 2.0f) / 3.0f;
    const float tileH = 32.0f * scale;

    l.tileCount = 12;
    for (int i = 0; i < 12; ++i) {
        int r = i / 3;
        int c = i % 3;
        float tx = l.barLeft + static_cast<float>(c) * (tileW + colGap);
        float ty = gridY + static_cast<float>(r) * (tileH + rowGap);
        l.tiles[i] = {tx, ty, tx + tileW, ty + tileH};
    }
    return l;
}

inline int HitTestQuickControlTile(const QuickControlsLayout& layout, float x, float y) {
    for (int i = 0; i < layout.tileCount; ++i) {
        if (x >= layout.tiles[i].left && x <= layout.tiles[i].right &&
            y >= layout.tiles[i].top && y <= layout.tiles[i].bottom) {
            return i;
        }
    }
    return -1;
}

inline bool HitTestVolumeSlider(const QuickControlsLayout& layout, float x, float y, float tolerance = 10.0f) {
    return (x >= layout.barLeft - tolerance && x <= layout.barRight + tolerance &&
            y >= layout.barTop - tolerance && y <= layout.barBottom + tolerance);
}

// ── Vertical Sliding Pagination Dots Layout & Hit-Testing ─────────────────────
struct PaginationDotLayout {
    float dotX;
    float startY;
    float spacing;
    float radius;
    int tabCount;
};

inline PaginationDotLayout CalculatePaginationDotsLayout(
    float contentWidth, float contentHeight, int tabCount, float scale = 1.0f
) {
    PaginationDotLayout p = {};
    p.tabCount = tabCount;
    p.spacing = 10.0f * scale;
    p.radius = 2.5f * scale;
    p.dotX = contentWidth - 12.0f * scale;
    p.startY = contentHeight * 0.5f - p.spacing * static_cast<float>(tabCount - 1) * 0.5f;
    return p;
}

inline int HitTestPaginationDot(const PaginationDotLayout& layout, float x, float y, float hitTolerance = 6.5f) {
    if (layout.tabCount <= 1) return -1;
    if (std::abs(x - layout.dotX) > hitTolerance * 2.0f) return -1;
    for (int i = 0; i < layout.tabCount; ++i) {
        float dotY = layout.startY + layout.spacing * static_cast<float>(i);
        if (std::abs(y - dotY) <= hitTolerance) {
            return i;
        }
    }
    return -1;
}

} // namespace island_layout
