# Changelog

## 1.5.1
- **Critical Fix:** Statically link C++ runtime (`-static`), removing all dependencies on external `libc++.whl`/`libunwind.whl` DLLs so the standalone EXE runs out-of-the-box on clean Windows 10/11 PCs without any developer tools installed.
- **Embedded Win32 Manifest:** Embedded `app.manifest` supporting Common Controls v6 (modern visual styles), PerMonitorV2 DPI awareness, and Windows 10/11 compatibility declarations.
- **Fixed Liquid Glass Fallback:** Added explicit default of 0 for `Themes.RealLiquidGlass` in `standalone_shim.h` and `settings_window.h`, ensuring clean installations always default to the fast, 0% CPU transparent obsidian glass.
- **Perfected Bluetooth State Machine:** Separated connection state from alert timestamps, enabling disconnect alerts for headsets connected prior to launch while suppressing phantom BLE alerts and duplicate flapping.
- **Dynamic Debug Logging:** Removed stale caching from `DiLogEnabled()`, allowing Debug Logging in settings to take effect immediately without restarting.
- **Web Identity:** Updated User-Agent strings for weather, lyrics, and lookup to point to `github.com/aathishk/Dynamic-island-rea`.
- **Validation Guard:** Added `tools/check_imports.py` to verify that only system DLLs are imported and that the manifest is present.

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
