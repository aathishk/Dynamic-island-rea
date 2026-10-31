# Dynamic Edge — Dynamic Island for Windows 10 & 11

A hardware-accelerated (Direct2D) Dynamic Island / edge dock for Windows: media controls, volume flyout, calendar, weather, hardware monitor, file tray, quick lookup and app launcher in a spring-animated pill.

> **Based on** [devcode90/Dynamic-Island-for-Windows](https://github.com/devcode90/Dynamic-Island-for-Windows) (MIT) and its contributors. This repository adds the *Dynamic Edge* changes and a standalone (non-Windhawk) build. See [Credits](#credits) and [LICENSE](LICENSE).

## What this fork adds

| Area | Change |
| :-- | :-- |
| Standalone app | Runs as a normal EXE without Windhawk: **tray icon with menu** (show/hide, position, settings, run at startup, exit), single-instance guard, Explorer-restart recovery |
| Settings GUI | **Native Windows 11 Fluent Dark Settings window** with tabs for Appearance, Glass & Effects, Animation Physics, and Modules. Instantly hot-reloads the live island |
| Look & Glass | **Crystal-Clear Obsidian Glass** (0% CPU default) + optional **Optical Liquid Glass** (`liquid_glass.h`: live backdrop capture, separable 2-pass blur, and capsule lens refraction, drawn with Direct2D) |
| Placement | **Adaptive Edge Blade** (auto) position plus a position submenu in both the right-click and tray menus |
| Animation | `applespring` animation style |
| Media | Master-volume flyout with mouse-wheel scrubbing over the speaker icon, middle-click to cycle media sessions, hardware media-key fallback |
| Feed filtering | "Quiet feed" mode that suppresses repeated banners while short-form video is playing |
| Bluetooth Stability | **BLE anti-flapping guard**: suppresses phantom disconnect banners from background BLE peripherals (e.g. sleep/idle devices) and debounces repeated events |
| Launcher | **User-editable quick launchers** (`[Launchers]` in `settings.ini`) in the Quick Lookup box |
| Logging | Opt-in, rotated, written to `%APPDATA%\DynamicIsland` (no more `debug.log` in the working directory) |

Everything else (calendar, weather, hardware monitor, lyrics, file tray, themes, 12 languages, privacy dots, …) comes from the upstream project; see its README for details.

## Install

Download `DynamicIsland.exe` from the [Releases](../../releases) page (built by GitHub Actions; a `.sha256` file is attached) and run it. Use the tray icon → **Run at startup** to autostart it.

Settings can be configured through the **Dynamic Island Settings GUI** (accessible via System Tray → **Settings...** or right-clicking the island → **⚙️ Dynamic Island Settings**). Changes apply immediately without restarting. Advanced settings can still be hand-edited in `%APPDATA%\DynamicIsland\settings.ini`.

### Quick launchers

```ini
[Launchers]
1=Windows Terminal|wt.exe|terminal,wt
2=File Explorer|explorer.exe|explorer,files
3=Web Browser|msedge.exe|browser,edge
```

Format: `label | command | aliases`. Press **Ctrl+Alt+Space**, type an alias and press Enter. Only commands in this list can be launched from the box; any other text is a dictionary/Wikipedia lookup. If the section is absent, a built-in default list is used.

## Privacy

There is **no telemetry**. The app does make network requests for the features that need them, all over HTTPS:

- Weather → `wttr.in`
- Synced lyrics → `lrclib.net`
- Quick Lookup (only after you press Enter) → Wikipedia / Wiktionary / dictionaryapi.dev

Notifications, clipboard, microphone/camera-in-use state and media sessions are read locally to render the island and are not sent anywhere.

## Build

Requires an LLVM-MinGW toolchain and C++/WinRT headers.

```
python build.py --toolchain C:\llvm-mingw --cppwinrt "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\cppwinrt"
```

With a Windhawk install you can instead pass `--toolchain <Windhawk>\Compiler` (it bundles the WinRT headers). `ci/build.yml` is a ready-made GitHub Actions workflow (builds the EXE, attaches a SHA-256, and publishes a release when you push a `vX.Y.Z` tag). Copy it to `.github/workflows/build.yml` to enable it.

## Known limitations

- **The Windhawk-mod variant is not currently verified.** `legacy/compile_mod.py` is kept for reference; a fallback `GetIniFilePath()` was added so the file no longer depends on the standalone shim, but it has not been rebuilt against Windhawk.
- **Backdrop Material & Optical Glass:** The default theme uses a crystal-clear obsidian glass base (16% alpha with dynamic album-art accent bloom and specular rim highlight), requiring zero background capture and 0% CPU. An optional optical liquid glass engine (`Themes.RealLiquidGlass=1`, toggleable in Settings) provides live backdrop capture, separable 2-pass blur, and capsule lens refraction rendered with Direct2D.

## Repository layout

| Path | Purpose |
| :-- | :-- |
| `dynamic-island-ultra.cpp` | The island (single translation unit) |
| `standalone_shim.h` | Replaces Windhawk's settings/log API with `settings.ini` |
| `standalone_app.h` | Standalone entry point, tray icon and menu |
| `settings_window.h` | Native Windows 11 Fluent dark-mode settings dialog |
| `liquid_glass.h` | Real optical liquid glass engine (backdrop capture, blur, lens refraction) |
| `launchers.h` | Editable launcher list |
| `build.py` | Build script |
| `tests/` | Compile-time stub check for the Win32 parts |
| `ci/` | GitHub Actions workflow (copy to `.github/workflows/`) |
| `tools/` | Developer helper scripts (RAM/CPU probe, window inspector, …) |
| `legacy/` | The original string-patch pipeline used to derive this file from upstream (historical, not needed to build) |

## Credits

Original project: **devcode90** — [Dynamic-Island-for-Windows](https://github.com/devcode90/Dynamic-Island-for-Windows). Upstream contributors: Sarthak Singh (sarthakaksh), ciizerr, ChrisSch-dev, thevioletto, David Ravelo (DavidRaveloU), Retr0dev-jpg — see the header of `dynamic-island-ultra.cpp` for their contributions.

Dynamic Edge modifications and standalone build: **Aathish Kamaraj** ([@aathishk](https://github.com/aathishk)).

Licensed under the [MIT License](LICENSE).
