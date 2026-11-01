// quick_controls.h - Native Quick Control companion panel and Windows setting URI actions for Dynamic Edge v1.7.0
#pragma once
#include <windows.h>
#include <shellapi.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <string>
#include <vector>
#include <algorithm>
#include "context_policy.h"

namespace island_controls {

enum class ControlAction {
    OpenWifiSettings,
    OpenBluetoothSettings,
    OpenSoundSettings,
    OpenDisplaySettings,
    OpenPowerSettings,
    OpenNightLightSettings,
    OpenTaskManager,
    OpenWindowsSettings,
    LaunchScreenClip,
    ToggleFocusMode,
    ToggleGamingMode,
    TogglePresentationMode,
    OpenDynamicEdgeSettings,
};

struct ControlTile {
    ControlAction action;
    const wchar_t* label;
    const wchar_t* description;
    const wchar_t* glyph; // Segoe Fluent / MDL2 Assets glyph
};

inline std::vector<ControlTile> GetStandardControlTiles() {
    return {
        {ControlAction::OpenSoundSettings, L"Sound Output", L"Open Sound & Audio devices", L"\uE767"},
        {ControlAction::OpenDisplaySettings, L"Display & Brightness", L"Adjust display settings", L"\uE706"},
        {ControlAction::OpenWifiSettings, L"Network & Wi-Fi", L"Wi-Fi networks and status", L"\uE701"},
        {ControlAction::OpenBluetoothSettings, L"Bluetooth", L"Bluetooth & device pairing", L"\uE702"},
        {ControlAction::OpenPowerSettings, L"Power & Battery", L"Power modes and battery saver", L"\uE83E"},
        {ControlAction::OpenNightLightSettings, L"Night Light", L"Blue light filter & schedule", L"\uE708"},
        {ControlAction::LaunchScreenClip, L"Screen Snipping", L"Capture screenshot (Win+Shift+S)", L"\uE722"},
        {ControlAction::OpenTaskManager, L"Task Manager", L"Monitor processes & performance", L"\uE7F4"},
        {ControlAction::ToggleFocusMode, L"Focus Mode", L"Silence alerts and start session", L"\uE7C1"},
        {ControlAction::ToggleGamingMode, L"Gaming Mode", L"Suppress popups & throttle effects", L"\uE7FC"},
        {ControlAction::TogglePresentationMode, L"Presentation", L"Hide personal cards on screen", L"\uE7F4"},
        {ControlAction::OpenDynamicEdgeSettings, L"Dynamic Edge", L"Open Settings & Profile Studio", L"\uE713"},
    };
}

// Executes a quick control action safely using standard Windows URI schemes or official APIs
inline bool ExecuteControlAction(ControlAction action, HWND parentHwnd = nullptr, island_context::ContextEngine* pContext = nullptr) {
    auto runShell = [&](const wchar_t* target) -> bool {
        HINSTANCE hInst = ShellExecuteW(parentHwnd, L"open", target, nullptr, nullptr, SW_SHOWNORMAL);
        return (reinterpret_cast<INT_PTR>(hInst) > 32);
    };

    switch (action) {
    case ControlAction::OpenWifiSettings:
        return runShell(L"ms-settings:network-wifi");

    case ControlAction::OpenBluetoothSettings:
        return runShell(L"ms-settings:bluetooth");

    case ControlAction::OpenSoundSettings:
        return runShell(L"ms-settings:sound");

    case ControlAction::OpenDisplaySettings:
        return runShell(L"ms-settings:display");

    case ControlAction::OpenPowerSettings:
        return runShell(L"ms-settings:powersleep");

    case ControlAction::OpenNightLightSettings:
        return runShell(L"ms-settings:nightlight");

    case ControlAction::LaunchScreenClip:
        return runShell(L"ms-screenclip:");

    case ControlAction::OpenTaskManager:
        return runShell(L"taskmgr.exe");

    case ControlAction::OpenWindowsSettings:
        return runShell(L"ms-settings:");

    case ControlAction::ToggleFocusMode:
        if (pContext) {
            if (pContext->GetActiveMode() == island_context::ContextMode::Focus) {
                pContext->ClearManualLock();
            } else {
                pContext->SetManualLock(island_context::ContextMode::Focus, 60);
            }
            return true;
        }
        return false;

    case ControlAction::ToggleGamingMode:
        if (pContext) {
            if (pContext->GetActiveMode() == island_context::ContextMode::Gaming) {
                pContext->ClearManualLock();
            } else {
                pContext->SetManualLock(island_context::ContextMode::Gaming, 120);
            }
            return true;
        }
        return false;

    case ControlAction::TogglePresentationMode:
        if (pContext) {
            if (pContext->GetActiveMode() == island_context::ContextMode::Presentation) {
                pContext->ClearManualLock();
            } else {
                pContext->SetManualLock(island_context::ContextMode::Presentation, 60);
            }
            return true;
        }
        return false;

    case ControlAction::OpenDynamicEdgeSettings:
        // Handled directly by callers via settings_ui::ShowSettingsWindow
        return true;

    default:
        return false;
    }
}

// Sets current master audio volume percentage (0 - 100) using Core Audio APIs
inline bool SetMasterVolumePercent(int percent) {
    percent = std::clamp(percent, 0, 100);
    float scalar = static_cast<float>(percent) / 100.0f;
    bool success = false;
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    IMMDeviceEnumerator* enumerator = nullptr;
    if (SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                  __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&enumerator)))) {
        IMMDevice* device = nullptr;
        if (SUCCEEDED(enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device))) {
            IAudioEndpointVolume* endpointVolume = nullptr;
            if (SUCCEEDED(device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr,
                                          reinterpret_cast<void**>(&endpointVolume)))) {
                if (SUCCEEDED(endpointVolume->SetMasterVolumeLevelScalar(scalar, nullptr))) {
                    success = true;
                }
                endpointVolume->Release();
            }
            device->Release();
        }
        enumerator->Release();
    }
    if (SUCCEEDED(hr)) {
        CoUninitialize();
    }
    return success;
}

// Retrieves current master audio volume percentage (0 - 100) using Core Audio APIs
inline int GetMasterVolumePercent() {
    int percent = 50;
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    IMMDeviceEnumerator* enumerator = nullptr;
    if (SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                  __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&enumerator)))) {
        IMMDevice* device = nullptr;
        if (SUCCEEDED(enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device))) {
            IAudioEndpointVolume* endpointVolume = nullptr;
            if (SUCCEEDED(device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr,
                                          reinterpret_cast<void**>(&endpointVolume)))) {
                float currentVolume = 0.5f;
                if (SUCCEEDED(endpointVolume->GetMasterVolumeLevelScalar(&currentVolume))) {
                    percent = static_cast<int>(currentVolume * 100.0f + 0.5f);
                }
                endpointVolume->Release();
            }
            device->Release();
        }
        enumerator->Release();
    }
    if (SUCCEEDED(hr)) {
        CoUninitialize();
    }
    return percent;
}

} // namespace island_controls
