// focus_timer.h - Enhanced Pomodoro & Focus session state machine for Dynamic Edge v1.7.0
#pragma once
#include <windows.h>
#include <string>
#include <algorithm>

namespace island_focus {

enum class FocusState {
    Idle,
    WorkSession,
    ShortBreak,
    LongBreak,
    Paused,
};

struct FocusConfig {
    int workMinutes = 25;
    int shortBreakMinutes = 5;
    int longBreakMinutes = 15;
    int sessionsBeforeLongBreak = 4;
    bool autoActivateFocusContext = true;
    bool alertOnComplete = true;
};

class FocusTimerSession {
private:
    FocusState state_ = FocusState::Idle;
    FocusState previousState_ = FocusState::Idle;
    FocusConfig config_;
    int completedSessions_ = 0;
    ULONGLONG targetTick_ = 0;
    ULONGLONG remainingOnPauseMs_ = 0;
    ULONGLONG totalDurationMs_ = 0;

public:
    FocusTimerSession(const FocusConfig& cfg = {}) : config_(cfg) {}

    void StartWork(int customMinutes = 0) {
        int mins = (customMinutes > 0) ? customMinutes : config_.workMinutes;
        state_ = FocusState::WorkSession;
        totalDurationMs_ = static_cast<ULONGLONG>(mins) * 60ULL * 1000ULL;
        targetTick_ = GetTickCount64() + totalDurationMs_;
        remainingOnPauseMs_ = 0;
    }

    void StartBreak(bool isLong = false, int customMinutes = 0) {
        int mins = customMinutes > 0 ? customMinutes : (isLong ? config_.longBreakMinutes : config_.shortBreakMinutes);
        state_ = isLong ? FocusState::LongBreak : FocusState::ShortBreak;
        totalDurationMs_ = static_cast<ULONGLONG>(mins) * 60ULL * 1000ULL;
        targetTick_ = GetTickCount64() + totalDurationMs_;
        remainingOnPauseMs_ = 0;
    }

    void Pause() {
        if (state_ == FocusState::Idle || state_ == FocusState::Paused) return;
        ULONGLONG now = GetTickCount64();
        if (now < targetTick_) {
            remainingOnPauseMs_ = targetTick_ - now;
        } else {
            remainingOnPauseMs_ = 0;
        }
        previousState_ = state_;
        state_ = FocusState::Paused;
    }

    void Resume() {
        if (state_ != FocusState::Paused) return;
        targetTick_ = GetTickCount64() + remainingOnPauseMs_;
        state_ = previousState_;
        remainingOnPauseMs_ = 0;
    }

    void TogglePause() {
        if (state_ == FocusState::Paused) {
            Resume();
        } else if (state_ != FocusState::Idle) {
            Pause();
        }
    }

    void Stop() {
        state_ = FocusState::Idle;
        targetTick_ = 0;
        remainingOnPauseMs_ = 0;
        totalDurationMs_ = 0;
    }

    void Skip() {
        if (state_ == FocusState::WorkSession) {
            completedSessions_++;
            bool needLongBreak = (completedSessions_ % config_.sessionsBeforeLongBreak == 0);
            StartBreak(needLongBreak);
        } else if (state_ == FocusState::ShortBreak || state_ == FocusState::LongBreak) {
            StartWork();
        } else {
            Stop();
        }
    }

    // Called periodically (e.g. per second). Returns true if a state transition just occurred
    bool Tick(bool& outCompletedWork) {
        outCompletedWork = false;
        if (state_ == FocusState::Idle || state_ == FocusState::Paused) {
            return false;
        }

        if (GetTickCount64() >= targetTick_) {
            // Timer expired!
            if (state_ == FocusState::WorkSession) {
                completedSessions_++;
                outCompletedWork = true;
                bool needLongBreak = (completedSessions_ % config_.sessionsBeforeLongBreak == 0);
                StartBreak(needLongBreak);
            } else {
                StartWork();
            }
            return true;
        }
        return false;
    }

    FocusState GetState() const { return state_; }
    int GetCompletedSessions() const { return completedSessions_; }

    int GetRemainingSeconds() const {
        if (state_ == FocusState::Idle) return 0;
        if (state_ == FocusState::Paused) {
            return static_cast<int>(remainingOnPauseMs_ / 1000ULL);
        }
        ULONGLONG now = GetTickCount64();
        if (now >= targetTick_) return 0;
        return static_cast<int>((targetTick_ - now) / 1000ULL);
    }

    float GetProgress() const {
        if (totalDurationMs_ == 0 || state_ == FocusState::Idle) return 0.0f;
        int remainingSec = GetRemainingSeconds();
        float totalSec = static_cast<float>(totalDurationMs_) / 1000.0f;
        float elapsed = totalSec - static_cast<float>(remainingSec);
        return std::clamp(elapsed / totalSec, 0.0f, 1.0f);
    }

    const FocusConfig& GetConfig() const { return config_; }
    void SetConfig(const FocusConfig& cfg) { config_ = cfg; }
};

} // namespace island_focus
