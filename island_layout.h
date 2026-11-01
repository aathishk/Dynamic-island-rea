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
        dims.height = (tabType == 33 /* File Tray */) ? (190.0f * scale) : (180.0f * scale);
        dims.cornerRadius = 28.0f * scale;
        break;
    }

    return dims;
}

} // namespace island_layout
