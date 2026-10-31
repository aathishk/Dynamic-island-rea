# Changelog

## 1.5.1
- **Fix:** the exe no longer depends on `libc++`/`libunwind` DLLs (built with `-static`); v1.5.0 could fail to start on a clean PC.
- Embedded manifest (Common Controls v6 visual styles, Per-Monitor-V2 DPI, Windows 10/11 compatibility).
- Web requests identify as this project instead of upstream.
- `tools/check_imports.py` guards against unshipped DLL dependencies in releases.

## 1.5.0
- **Bluetooth Anti-Flapping & Phantom Disconnect Fix**: Suppressed unprompted disconnect banners for paired Bluetooth Low Energy (BLE) peripherals (e.g. idle/sleeping devices like `GP325`), debounced flapping events, and guarded initial watcher enumeration.
- **Real Optical Liquid Glass Refraction Engine** (`liquid_glass.h`): Live desktop backdrop capture with separable sliding-window box blur and capsule lens refraction edge displacement, rendered via Direct2D (opt-in under Settings; default is lightweight crystal-clear obsidian glass).
- **Native Settings GUI Window** (`settings_window.h`): Fluent dark-themed settings window with tabs for Appearance, Glass & Effects, Animation Physics, and Modules. Changes apply instantly without restarting. Accessible from both System Tray and right-click context menu.
- **Modularized Architecture & Tests**: Code split into dedicated headers (`liquid_glass.h`, `settings_window.h`, `standalone_app.h`, `launchers.h`), and `tests/standalone_stub_check.cpp` verified with clean compilation.
- Standalone app: tray icon with working context menu, Run at startup, single-instance guard, message loop that survives Explorer restarts.
- Quick-launch list is now user-editable (`[Launchers]` in `settings.ini`); aliases such as `terminal`, `browser` and `edge` now actually launch (previously only literal exe names worked).
- Logging is opt-in (`Debug.Log=1` or `DYNAMIC_ISLAND_DEBUG=1`), written to `%APPDATA%\DynamicIsland\debug.log` and rotated at 512 KB.
- `GetIniFilePath()` no longer exists only in the standalone shim, so the source no longer hard-depends on it.
- New `build.py` (no hard-coded user paths) and GitHub Actions build/release workflow; binary no longer committed to git.
- Added LICENSE (MIT, with upstream copyright), proper credits, README rewritten to match actual behaviour.
- Moved scratch scripts to `tools/` and the original patch pipeline to `legacy/`.

## 1.4.0
- Initial Dynamic Edge release (see `legacy/build_ultra.py` for the list of patches).
