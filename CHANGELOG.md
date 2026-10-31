# Changelog

## 1.5.0
- **Real Optical Liquid Glass Engine** (`liquid_glass.h`): Direct2D live backdrop capture, separable dual-pass blur, and lens refraction edge displacement (no square grey boxes or GDI stair-stepping).
- **Native Settings GUI Window** (`settings_window.h`): Fluent dark-themed settings window with tabs for Appearance, Glass & Effects, Animation Physics, and Modules. Changes apply instantly without restarting. Accessible from both System Tray and right-click context menu.
- **Modularized Architecture**: Code split into dedicated headers (`liquid_glass.h`, `settings_window.h`, `standalone_app.h`, `launchers.h`).
- Standalone app: tray icon with working context menu, Run at startup, single-instance guard, message loop that survives Explorer restarts.
- Quick-launch list is now user-editable (`[Launchers]` in `settings.ini`); aliases such as `terminal`, `browser` and `edge` now actually launch (previously only literal exe names worked).
- Logging is opt-in (`Debug.Log=1` or `DYNAMIC_ISLAND_DEBUG=1`), written to `%APPDATA%\DynamicIsland\debug.log` and rotated at 512 KB.
- `GetIniFilePath()` no longer exists only in the standalone shim, so the source no longer hard-depends on it.
- New `build.py` (no hard-coded user paths) and GitHub Actions build/release workflow; binary no longer committed to git.
- Added LICENSE (MIT, with upstream copyright), proper credits, README rewritten to match actual behaviour.
- Moved scratch scripts to `tools/` and the original patch pipeline to `legacy/`.

## 1.4.0
- Initial Dynamic Edge release (see `legacy/build_ultra.py` for the list of patches).
