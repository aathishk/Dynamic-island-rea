// island_policy.h - Core domain policies for Dynamic Island
// Single source of truth shared between dynamic-island-ultra.cpp and unit tests.
#pragma once
#include <algorithm>
#include <string>
#include <cmath>

#ifndef DYNAMIC_ISLAND_BT_CATEGORY_DEFINED
#define DYNAMIC_ISLAND_BT_CATEGORY_DEFINED
enum class BluetoothDeviceCategory {
    Headphones,
    Speaker,
    Mouse,
    Keyboard,
    Gamepad,
    Phone,
    Generic,
};
#endif

namespace island_policy {

// Calculates fractional opacity (0.10f to 1.0f) from an integer percentage.
// Guarantees that the 16% crystal clear transparent theme is preserved at 0.16f.
inline float CalculatePillOpacity(int percent) {
    float f = static_cast<float>(percent) / 100.0f;
    return std::clamp(f, 0.10f, 1.0f);
}

// Converts fractional pill opacity to Win32 BLENDFUNCTION SourceConstantAlpha byte (25 to 255).
inline unsigned char PillOpacityToAlphaByte(float opacity) {
    float clamped = std::clamp(opacity, 0.10f, 1.0f);
    return static_cast<unsigned char>(std::round(clamped * 255.0f));
}

// Determines whether a Bluetooth device disconnect event should be suppressed.
// - wasConnected: true if the device was actively connected prior to disconnect.
// - isBle: true if the device uses Bluetooth Low Energy transport.
// - cat: category classification (Headphones, Speaker, Mouse, Keyboard, Gamepad, Phone, Generic).
// - allowGenericBle: user configuration (Modules.BluetoothAlertOnGenericBle).
inline bool ShouldSuppressBluetoothDisconnect(
    bool wasConnected,
    bool isBle,
    BluetoothDeviceCategory cat,
    bool allowGenericBle) {
    if (!wasConnected) {
        return true;  // Never alert if device was not actively connected
    }
    // Generic BLE peripherals (e.g. GP325) poll and sleep continuously;
    // suppress disconnect cards by default unless user explicitly enabled raw BLE alerts.
    // Audio, input peripherals, and gamepads are never suppressed.
    if (isBle && cat == BluetoothDeviceCategory::Generic && !allowGenericBle) {
        return true;
    }
    return false;
}

// Fast synchronous friendly-name classifier
inline BluetoothDeviceCategory ClassifyDeviceByName(const std::wstring& nameLower) {
    if (nameLower.find(L"headphone") != std::wstring::npos ||
        nameLower.find(L"headset") != std::wstring::npos ||
        nameLower.find(L"earbud") != std::wstring::npos ||
        nameLower.find(L"buds") != std::wstring::npos ||
        nameLower.find(L"airpods") != std::wstring::npos) {
        return BluetoothDeviceCategory::Headphones;
    }
    if (nameLower.find(L"speaker") != std::wstring::npos ||
        nameLower.find(L"soundbar") != std::wstring::npos ||
        nameLower.find(L"boombox") != std::wstring::npos) {
        return BluetoothDeviceCategory::Speaker;
    }
    if (nameLower.find(L"mouse") != std::wstring::npos ||
        nameLower.find(L"trackpad") != std::wstring::npos) {
        return BluetoothDeviceCategory::Mouse;
    }
    if (nameLower.find(L"keyboard") != std::wstring::npos ||
        nameLower.find(L"keypad") != std::wstring::npos) {
        return BluetoothDeviceCategory::Keyboard;
    }
    if (nameLower.find(L"controller") != std::wstring::npos ||
        nameLower.find(L"gamepad") != std::wstring::npos ||
        nameLower.find(L"xbox") != std::wstring::npos ||
        nameLower.find(L"dualsense") != std::wstring::npos ||
        nameLower.find(L"dualshock") != std::wstring::npos) {
        return BluetoothDeviceCategory::Gamepad;
    }
    if (nameLower.find(L"iphone") != std::wstring::npos ||
        nameLower.find(L"phone") != std::wstring::npos ||
        nameLower.find(L"galaxy") != std::wstring::npos ||
        nameLower.find(L"pixel") != std::wstring::npos) {
        return BluetoothDeviceCategory::Phone;
    }
    return BluetoothDeviceCategory::Generic;
}

}  // namespace island_policy
