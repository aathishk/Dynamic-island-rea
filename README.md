# Dynamic Edge — Dynamic Island for Windows 10 & 11

A hardware-accelerated (Direct2D) Dynamic Island / edge dock for Windows: media controls, volume flyout, calendar, weather, hardware monitor, file tray, quick lookup and app launcher in a spring-animated pill.

> **Based on** [devcode90/Dynamic-Island-for-Windows](https://github.com/devcode90/Dynamic-Island-for-Windows) (MIT) and its contributors. This repository adds the *Dynamic Edge* changes and a standalone (non-Windhawk) build. See [Credits](#credits) and [LICENSE](LICENSE).

## What this fork adds

| Area | Change |
| :-- | :-- |
| Standalone app | Runs as a normal EXE without Windhawk: **tray icon with menu** (show/hide, position, run at startup, open settings, exit), single-instance guard, Explorer-restart recovery |
| Placement | **Adaptive Edge Blade** (auto) position plus a position submenu in both the right-click and tray menus |
| Animation | `applespring` animation style |
| Media | Master-volume flyout with mouse-wheel scrubbing over the speaker icon, middle-click to cycle media sessions, hardware media-key fallback |
| Feed filtering | "Quiet feed" mode that suppresses repeated banners while short-form video is playing |
| Launcher | **User-editable quick launchers** (`[Launchers]` in `settings.ini`) in the Quick Lookup box |
| Look | Flat glass theme with album-art adaptive colour and soft shadow |
| Logging | Opt-in, rotated, written to `%APPDATA%\DynamicIsland` (no more `debug.log` in the working directory) |

Everything else (calendar, weather, hardware monitor, lyrics, file tray, themes, 12 languages, privacy dots, …) comes from the upstream project; see its README for details.

## Install

Download `DynamicIsland.exe` from the [Releases](../../releases) page (built by GitHub Actions; a `.sha256` file is attached) and run it. Use the tray icon → **Run at startup** to autostart it.

Settings live in `%APPDATA%\DynamicIsland\settings.ini` (tray icon → *Open settings file*). Restart the app after editing it.

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

With a Windhawk install you can instead pass `--toolchain <Windhawk>\Compiler` (it bundles the WinRT headers). `.github/workflows/build.yml` shows the exact CI setup; tagging `vX.Y.Z` publishes a release with the EXE and checksum.

## Known limitations

- **Blur / Acrylic backdrop is disabled in this build.** The backdrop windows are parked on purpose to avoid rectangular grey artifacts, so the "Backdrop material" option has no visible effect. The glass look is drawn with Direct2D gradients, not real refraction.
- **The Windhawk-mod variant is not currently verified.** `legacy/compile_mod.py` is kept for reference; a fallback `GetIniFilePath()` was added so the file no longer depends on the standalone shim, but it has not been rebuilt against Windhawk.
- Settings in the standalone build are edited via `settings.ini` (no settings window yet); unspecified numeric toggles default to `1` (enabled).

## Repository layout

| Path | Purpose |
| :-- | :-- |
| `dynamic-island-ultra.cpp` | The island (single translation unit) |
| `standalone_shim.h` | Replaces Windhawk's settings/log API with `settings.ini` |
| `standalone_app.h` | Standalone entry point, tray icon and menu |
| `launchers.h` | Editable launcher list |
| `build.py` | Build script |
| `tests/` | Compile-time stub check for the Win32 parts |
| `tools/` | Developer helper scripts (RAM/CPU probe, window inspector, …) |
| `legacy/` | The original string-patch pipeline used to derive this file from upstream (historical, not needed to build) |

## Credits

Original project: **devcode90** — [Dynamic-Island-for-Windows](https://github.com/devcode90/Dynamic-Island-for-Windows). Upstream contributors: Sarthak Singh (sarthakaksh), ciizerr, ChrisSch-dev, thevioletto, David Ravelo (DavidRaveloU), Retr0dev-jpg — see the header of `dynamic-island-ultra.cpp` for their contributions.

Dynamic Edge modifications and standalone build: **Aathish Kamaraj** ([@aathishk](https://github.com/aathishk)).

Licensed under the [MIT License](LICENSE).
