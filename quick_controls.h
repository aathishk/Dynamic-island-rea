// quick_controls.h - Native Quick Control companion panel and Windows setting URI actions for Dynamic Edge v1.7.0
#pragma once
#include <windows.h>
#include <shellapi.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <string>
#include <vector>

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
inline bool ExecuteControlAction(ControlAction action, HWND parentHwnd = nullptr) {
    switch (action) {
    case ControlAction::OpenWifiSettings:
        ShellExecuteW(parentHwnd, L"open", L"ms-settings:network-wifi", nullptr, nullptr, SW_SHOWNORMAL);
        return true;

    case ControlAction::OpenBluetoothSettings:
        ShellExecuteW(parentHwnd, L"open", L"ms-settings:bluetooth", nullptr, nullptr, SW_SHOWNORMAL);
        return true;

    case ControlAction::OpenSoundSettings:
        ShellExecuteW(parentHwnd, L"open", L"ms-settings:sound", nullptr, nullptr, SW_SHOWNORMAL);
        return true;

    case ControlAction::OpenDisplaySettings:
        ShellExecuteW(parentHwnd, L"open", L"ms-settings:display", nullptr, nullptr, SW_SHOWNORMAL);
        return true;

    case ControlAction::OpenPowerSettings:
        ShellExecuteW(parentHwnd, L"open", L"ms-settings:powersleep", nullptr, nullptr, SW_SHOWNORMAL);
        return true;

    case ControlAction::OpenNightLightSettings:
        ShellExecuteW(parentHwnd, L"open", L"ms-settings:nightlight", nullptr, nullptr, SW_SHOWNORMAL);
        return true;

    case ControlAction::LaunchScreenClip:
        ShellExecuteW(parentHwnd, L"open", L"ms-screenclip:", nullptr, nullptr, SW_SHOWNORMAL);
        return true;

    case ControlAction::OpenTaskManager:
        ShellExecuteW(parentHwnd, L"open", L"taskmgr.exe", nullptr, nullptr, SW_SHOWNORMAL);
        return true;

    case ControlAction::OpenWindowsSettings:
        ShellExecuteW(parentHwnd, L"open", L"ms-settings:", nullptr, nullptr, SW_SHOWNORMAL);
        return true;

    case ControlAction::OpenDynamicEdgeSettings:
        // Handled directly by callers via settings_ui::ShowSettingsWindow
        return true;

    default:
        return false;
    }
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
