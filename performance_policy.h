// performance_policy.h - Power-state awareness, battery saver adaptation, and live process diagnostics for Dynamic Edge v1.7.0
#pragma once
#include <windows.h>
#include <psapi.h>
#include <string>
#include <algorithm>

namespace island_performance {

enum class EffectivePowerMode {
    VisualQuality,
    Balanced,
    BatterySaver,
};

struct PowerStateInfo {
    bool hasBattery = false;
    bool isAcConnected = true;
    bool isBatterySaverActive = false;
    int batteryPercent = 100;
};

// Reads actual Win32 system power status
inline PowerStateInfo ReadSystemPowerStatus() {
    PowerStateInfo info;
    SYSTEM_POWER_STATUS sps;
    if (GetSystemPowerStatus(&sps)) {
        info.isAcConnected = (sps.ACLineStatus == 1);
        info.hasBattery = (sps.BatteryFlag != 128 && sps.BatteryFlag != 255);
        info.batteryPercent = (sps.BatteryLifePercent <= 100) ? sps.BatteryLifePercent : 100;
        info.isBatterySaverActive = (sps.SystemStatusFlag == 1); // 1 = Battery Saver is ON
    }
    return info;
}

// Determines the effective rendering power policy given user setting and live battery state
inline EffectivePowerMode DetermineEffectivePowerMode(
    const std::wstring& configuredMode,
    bool autoBatterySaver,
    const PowerStateInfo& power) {

    if (_wcsicmp(configuredMode.c_str(), L"battery_saver") == 0) {
        return EffectivePowerMode::BatterySaver;
    }

    // Auto battery-saver adaptation: if on battery and battery saver flag is active or <= 20%
    if (autoBatterySaver && !power.isAcConnected && power.hasBattery) {
        if (power.isBatterySaverActive || power.batteryPercent <= 20) {
            return EffectivePowerMode::BatterySaver;
        }
        // If configured as visual_quality on DC battery, drop to balanced to protect battery
        if (_wcsicmp(configuredMode.c_str(), L"visual_quality") == 0) {
            return EffectivePowerMode::Balanced;
        }
    }

    if (_wcsicmp(configuredMode.c_str(), L"visual_quality") == 0) {
        return EffectivePowerMode::VisualQuality;
    }
    return EffectivePowerMode::Balanced;
}

// True if expensive live backdrop capture (optical glass) is permitted under current power policy
inline bool IsLiveBackdropCaptureAllowed(EffectivePowerMode mode, bool userSetting) {
    if (!userSetting) return false;
    if (mode == EffectivePowerMode::BatterySaver) return false;
    return true;
}

// Live measurable process resource diagnostics
struct ProcessDiagnostics {
    double processCpuPercent = 0.0;
    size_t workingSetBytes = 0;
    size_t peakWorkingSetBytes = 0;
    PowerStateInfo power;
    EffectivePowerMode effectiveMode = EffectivePowerMode::Balanced;
    DWORD uptimeSeconds = 0;
};

class DiagnosticsTracker {
private:
    ULONGLONG lastTimeMs_ = 0;
    FILETIME lastKernelTime_ = {};
    FILETIME lastUserTime_ = {};
    double cachedCpuPercent_ = 0.0;

    static ULONGLONG FileTimeToUint64(const FILETIME& ft) {
        ULARGE_INTEGER li;
        li.LowPart = ft.dwLowDateTime;
        li.HighPart = ft.dwHighDateTime;
        return li.QuadPart;
    }

public:
    ProcessDiagnostics Sample(EffectivePowerMode currentMode) {
        ProcessDiagnostics diag;
        diag.effectiveMode = currentMode;
        diag.power = ReadSystemPowerStatus();

        // 1. Process Memory
        PROCESS_MEMORY_COUNTERS pmc = {};
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            diag.workingSetBytes = pmc.WorkingSetSize;
            diag.peakWorkingSetBytes = pmc.PeakWorkingSetSize;
        }

        // 2. Process CPU delta
        FILETIME creationTime, exitTime, kernelTime, userTime;
        if (GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime)) {
            ULONGLONG now = GetTickCount64();
            if (lastTimeMs_ > 0 && now > lastTimeMs_) {
                ULONGLONG elapsedMs = now - lastTimeMs_;
                ULONGLONG kDiff = FileTimeToUint64(kernelTime) - FileTimeToUint64(lastKernelTime_);
                ULONGLONG uDiff = FileTimeToUint64(userTime) - FileTimeToUint64(lastUserTime_);
                // 100-nanosecond intervals to milliseconds: divide by 10,000
                double totalCpuMs = static_cast<double>(kDiff + uDiff) / 10000.0;

                SYSTEM_INFO sysInfo;
                GetSystemInfo(&sysInfo);
                DWORD numCores = (sysInfo.dwNumberOfProcessors > 0) ? sysInfo.dwNumberOfProcessors : 1;

                cachedCpuPercent_ = (totalCpuMs / (elapsedMs * numCores)) * 100.0;
                cachedCpuPercent_ = std::clamp(cachedCpuPercent_, 0.0, 100.0);
            }
            lastTimeMs_ = now;
            lastKernelTime_ = kernelTime;
            lastUserTime_ = userTime;
        }
        diag.processCpuPercent = cachedCpuPercent_;

        // 3. Process Uptime
        FILETIME dummy1, dummy2;
        if (GetProcessTimes(GetCurrentProcess(), &creationTime, &dummy1, &dummy2, &dummy1)) {
            FILETIME nowFt;
            GetSystemTimeAsFileTime(&nowFt);
            ULONGLONG diff100ns = FileTimeToUint64(nowFt) - FileTimeToUint64(creationTime);
            diag.uptimeSeconds = static_cast<DWORD>(diff100ns / 10000000ULL);
        }

        return diag;
    }
};

} // namespace island_performance
