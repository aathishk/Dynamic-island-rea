# Dynamic Edge — Adaptive Desktop Companion for Windows 10 & 11

A hardware-accelerated (Direct2D) native desktop companion and edge dock for Windows: personalisation profiles studio, context engine, smart notifications, battery-conscious performance policy, quick controls, focus timer, file shelf, and launcher in a spring-animated pill.

> **Based on** [devcode90/Dynamic-Island-for-Windows](https://github.com/devcode90/Dynamic-Island-for-Windows) (MIT) and its contributors. This repository adds the *Dynamic Edge* architecture, native standalone execution, and desktop companion capabilities. See [Credits](#credits) and [LICENSE](LICENSE).

---

## What Dynamic Edge Adds (v1.7.0)

| Capability | Implementation |
| :-- | :-- |
| **Profile Studio** | **6 Built-in Presets** (`Obsidian`, `Frost`, `Midnight`, `Aurora`, `Minimal`, `Studio`) plus custom user profiles. One-click apply, save, duplicate, and atomic `%APPDATA%\DynamicIsland\profiles\` persistence |
| **Context Engine** | **Adaptive Context Modes** (`Normal`, `Focus`, `Gaming`, `Presentation`, `Custom`). Auto-detects foreground applications (e.g. `Code.exe` $\rightarrow$ Focus) and Windows fullscreen/presentation states (`SHQueryUserNotificationState`) with manual lock priority |
| **Central Notifications** | **Smart Priority & Quiet Hours**: Priority levels (`Critical`, `Normal`, `Quiet`), midnight-crossing quiet hours, fullscreen suppression, duplicate sensor debounce, and bounded opt-in local notification history |
| **Power & Battery Engine** | **Battery-Conscious Performance**: Automatically throttles idle rendering and disables expensive optical backdrop capture on DC battery power / battery saver (`GetSystemPowerStatus`) |
| **Live Diagnostics** | **Real Measured Process Diagnostics**: Live process CPU percentage delta (`GetProcessTimes`), Working Set RAM (`GetProcessMemoryInfo`), uptime, and auxiliary worker health |
| **Quick Control Centre** | Master volume, brightness, power settings, display, Wi-Fi, Bluetooth, Task Manager, screen snipping (`ms-screenclip:`), and Night Light shortcuts |
| **Focus & Pomodoro Timer** | Pomodoro cycles, custom work/break intervals, session tracking, monotonic clock (`GetTickCount64`), and automatic Focus context activation |
| **Local Text Clipboard** | **Privacy-First**: Disabled by default (opt-in), local-only storage, bounded entry limits, pin/delete, one-click pause and clear |
| **Configurable Gestures** | Single-click, double-click, middle-click, and mouse-wheel gestures over the island |
| **File Tray Shelf** | OLE drag-and-drop file shelf (`#33`), drag out to any app or folder, native shell icons, and shelf persistence |
| **First-Run Onboarding** | 30-second first-launch setup dialog: appearance profile $\rightarrow$ screen placement $\rightarrow$ essentials |

---

## Quick Launchers

Quick launchers can be typed directly into Quick Lookup (`Ctrl+Alt+Space`):

```ini
[Launchers]
1=Windows Terminal|wt.exe|terminal,wt
2=Calculator|calc.exe|calc
3=Notepad|notepad.exe|notes,txt
```

---

## Build

Requires an LLVM-MinGW toolchain and C++/WinRT headers.

```powershell
python build.py --toolchain C:\llvm-mingw --cppwinrt "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\cppwinrt"
```

With Windhawk installed:
```powershell
python build.py --toolchain "C:\Users\<User>\Windhawk\Compiler"
```

Automated GitHub Actions CI/CD ([.github/workflows/build.yml](.github/workflows/build.yml)) compiles and validates on both Linux (Wine runner) and Windows native runners, running the 11-group regression test suite, import verification, and checksum sidecar generation.

---

## Repository Layout

| Path | Purpose |
| :-- | :-- |
| `dynamic-island-ultra.cpp` | Main island runtime & Direct2D hardware-accelerated rendering engine |
| `profile_policy.h` | Profile schemas, built-in definitions (`Obsidian`, `Frost`, etc.), and validation |
| `profile_manager.h` | Atomic profile serialization, `%APPDATA%\DynamicIsland\profiles\` storage, import/export |
| `context_policy.h` | Context modes, application-aware rules, and fullscreen/presentation detection |
| `notification_policy.h` | Priority arbitration, quiet hours, fullscreen suppression, and debounce |
| `notification_history.h` | Bounded, privacy-first notification history panel (opt-in) |
| `performance_policy.h` | Power state awareness (`GetSystemPowerStatus`), battery saver adaptation, process diagnostics |
| `quick_controls.h` | Native companion control center, master volume, and Windows settings URI actions |
| `clipboard_manager.h` | Privacy-first opt-in local text clipboard manager |
| `focus_timer.h` | Pomodoro session state machine, custom work/break intervals, monotonic timer |
| `gesture_manager.h` | Configurable island clicks/wheel gestures and safe quick actions |
| `island_layout.h` | Adaptive island presentation states and priority geometry arbitration |
| `liquid_glass.h` | Real optical liquid glass engine (backdrop capture, 2-pass blur, lens refraction) |
| `launchers.h` | Editable launcher list |
| `settings_window.h` | Native Windows 11 Fluent dark-mode Settings & Profile Studio GUI |
| `standalone_app.h` | Standalone entry point, single-instance guard, tray menu, first-run onboarding |
| `standalone_shim.h` | Standalone settings persistence and logging (`%APPDATA%\DynamicIsland\settings.ini`) |
| `tests/standalone_stub_check.cpp` | Comprehensive 11-suite unit & regression test runner |
| `.github/workflows/build.yml` | Automated GitHub Actions CI/CD workflow |

---

## Credits

Original project: **devcode90** — [Dynamic-Island-for-Windows](https://github.com/devcode90/Dynamic-Island-for-Windows). Upstream contributors: Sarthak Singh (sarthakaksh), ciizerr, ChrisSch-dev, thevioletto, David Ravelo (DavidRaveloU), Retr0dev-jpg — see the header of `dynamic-island-ultra.cpp` for their contributions.

Dynamic Edge desktop companion architecture and standalone release: **Aathish Kamaraj** ([@aathishk](https://github.com/aathishk)).

Licensed under the [MIT License](LICENSE).
