// island_alpha.h - Alpha-compositing, surface opacity, and layered-window composition policies
#pragma once
#include <windows.h>
#include <algorithm>
#include <cmath>

namespace island_layout {
    enum class DashboardPageId : int;
}

namespace island_alpha {

enum class ElementKind {
    CollapsedSurface,
    ExpandedMediaSurface,
    ExpandedDataDashboardSurface,
    NotificationSurface,
    ProgressSurface,
    CardSurface,
    CardSurfaceActive,
    CardStroke,
    StatusChipSurface,
    StatusChipStroke,
    PrimaryContent,    // Clock digits, large titles, hardware metric values
    SecondaryContent,  // Date header, subtitles, metric labels, icon glyphs
    TertiaryContent,   // Hairline dividers, subtle indicators
    DropShadow,
    SpecularSheen,
    ContourRimTop,
    ContourRimBottom,
};

// 1. Composition Opacity (Layered-window BLENDFUNCTION.SourceConstantAlpha byte)
// Controls window-level presence/transitions (fade-in, fade-out, auto-hide).
// In per-pixel alpha mode (AC_SRC_ALPHA), 255 allows Direct2D to dictate
// each pixel's exact transparency without capping maximum opacity.
inline BYTE CalculateCompositionAlphaByte(float windowFade = 1.0f) {
    float clamped = std::clamp(windowFade, 0.0f, 1.0f);
    return static_cast<BYTE>(std::round(clamped * 255.0f));
}

// 2. Surface Opacity (Direct2D per-pixel alpha for pill background & cards)
inline float CalculateSurfaceOpacity(
    bool isExpanded,
    island_layout::DashboardPageId activePageId,
    float settingPillOpacity
) {
    if (!isExpanded) {
        // Collapsed compact pill: follows the theme's pill opacity (e.g. 0.16 for Obsidian)
        return std::clamp(settingPillOpacity, 0.10f, 1.0f);
    }
    if (static_cast<int>(activePageId) == 1 /* Media */) {
        // Expanded Media Player: reactive translucent glass
        return 0.35f;
    }
    // Expanded Data Dashboards (Hardware Monitor, Overview, Calendar, Weather, Quick Controls, File Tray, Game Overlay):
    // Fully opaque solid obsidian backdrop (1.0f alpha) for 100% crisp readability over any background.
    return 1.0f;
}

// 3. Content Opacity (Direct2D per-pixel alpha for foreground text & glyphs)
// Foreground content is NEVER multiplied by the background glass opacity!
inline float CalculateContentOpacity(
    ElementKind kind,
    float contentAnimationAlpha = 1.0f
) {
    float base = 1.0f;
    switch (kind) {
    case ElementKind::PrimaryContent:
        base = 0.98f;
        break;
    case ElementKind::SecondaryContent:
        base = 0.85f;
        break;
    case ElementKind::TertiaryContent:
        base = 0.35f;
        break;
    default:
        base = 1.0f;
        break;
    }
    return base * std::clamp(contentAnimationAlpha, 0.0f, 1.0f);
}

// 4. Element Alpha Resolution: Single-Source of Truth for each visual component
inline float ResolveElementAlpha(
    ElementKind kind,
    bool isExpanded,
    island_layout::DashboardPageId activePageId,
    float settingPillOpacity,
    float contentAnimationAlpha = 1.0f
) {
    switch (kind) {
    case ElementKind::CollapsedSurface:
        return std::clamp(settingPillOpacity, 0.10f, 1.0f);
    case ElementKind::ExpandedMediaSurface:
        return 0.35f;
    case ElementKind::ExpandedDataDashboardSurface:
        return 1.0f;
    case ElementKind::NotificationSurface:
    case ElementKind::ProgressSurface:
        return 1.0f;
    case ElementKind::CardSurface:
        return 1.0f;
    case ElementKind::CardSurfaceActive:
        return 1.0f;
    case ElementKind::CardStroke:
        return 0.35f;
    case ElementKind::StatusChipSurface:
        return 0.92f;
    case ElementKind::StatusChipStroke:
        return 0.35f;
    case ElementKind::PrimaryContent:
        return 0.98f * std::clamp(contentAnimationAlpha, 0.0f, 1.0f);
    case ElementKind::SecondaryContent:
        return 0.85f * std::clamp(contentAnimationAlpha, 0.0f, 1.0f);
    case ElementKind::TertiaryContent:
        return 0.35f * std::clamp(contentAnimationAlpha, 0.0f, 1.0f);
    case ElementKind::DropShadow:
        return 0.22f;
    case ElementKind::SpecularSheen:
        return 0.18f;
    case ElementKind::ContourRimTop:
        return 0.32f;
    case ElementKind::ContourRimBottom:
        return 0.08f;
    }
    return 1.0f;
}

// 5. Model Composed Effective Alpha on Screen after Win32 Layered Window Composition
// Win32 Layered Window AC_SRC_ALPHA blends: ScreenAlpha = PixelAlpha * (SourceConstantAlpha / 255.0)
inline float CalculateComposedEffectiveAlpha(
    float direct2dAlpha,
    BYTE sourceConstantAlpha
) {
    return direct2dAlpha * (static_cast<float>(sourceConstantAlpha) / 255.0f);
}

} // namespace island_alpha
