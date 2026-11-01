# Changelog

## 1.5.5
- **Bluetooth Crash Guard for Windows N / Server Editions:**
  - Wrapped `BluetoothThreadProc` in a top-level `try/catch` guard. On Windows editions lacking the WinRT Bluetooth subsystem, `winrt::hresult_class_not_registered` is caught cleanly, logging a warning and disabling only the Bluetooth module rather than terminating the application with `std::terminate`.
- **Complete Direct2D Device-Dependent Resource Recovery:**
  - Added `ReleaseDeviceDependentResources()` in `Renderer`: resets all persistent brushes, bitmap textures, tab layers, and gradient slots upon `D2DERR_RECREATE_TARGET`, preventing `D2DERR_WRONG_RESOURCE_DOMAIN` errors.
  - Added `EnsureRenderTarget()` to guard `target_` creation and prevent null dereferences during active GPU resets.
- **Fixed Bluetooth Startup State Tracking & Debounce:**
  - Track devices discovered at launch with `isConnected = true` in the initial device cache, enabling `Modules.BluetoothAlertOnGenericBle = 1` to alert on disconnects for devices connected before app boot.
  - Reset `lastDisconnectTime = -100.0` upon new device connections so valid disconnects following a reconnect are never falsely suppressed.
  - Added classification rules for gamepads and wireless controllers (`Xbox`, `DualSense`, `DualShock`).
- **Live Worker Health Lifecycle & Clean Shutdown:**
  - `spawnOptionalWorker` now manages a runner wrapper ensuring worker status flags automatically reset to `false` if a worker thread exits.
  - `StopThreads()` detects wait timeouts and prevents closing synchronization events out from under running threads.
- **CI Pipeline Gating:**
  - Configured `exe` job to explicitly depend on `needs: stub-check` so release builds only proceed after successful test execution.


## 1.5.4
- **Shared Domain Policy Architecture (`island_policy.h`):**
  - Extracted opacity calculation (`CalculatePillOpacity`), window alpha byte conversion (`PillOpacityToAlphaByte`), Bluetooth disconnect suppression policy (`ShouldSuppressBluetoothDisconnect`), and friendly-name device classification (`ClassifyDeviceByName`) into a shared header.
  - Both production code in `dynamic-island-ultra.cpp` and unit tests in `tests/standalone_stub_check.cpp` now execute the exact same single source of truth.
- **Fixed Layered Window 16% Opacity Clamp:**
  - Removed remaining hardcoded `Clamp(..., 0.35f, 1.0f)` on line 12248 in `Renderer::Render()`, allowing true 16% obsidian glass transparency to reach `UPDATELAYEREDWINDOWINFO`.
- **True Direct2D Render Target Device-Loss Recovery:**
  - Added `D2DERR_RECREATE_TARGET` recovery in `Renderer::Render()` on `EndDraw()` failure, rebuilding the Direct2D DC render target and backing DIB on GPU resets and display mode switches.
- **Leak-Proof Thread Lifecycle & Worker Status:**
  - `StartThreads()` unwinds and terminates already-created events/threads immediately if critical initialization fails.
  - Added atomic `WorkerStatus` tracking (`g_workerStatus`) providing live operational status for all auxiliary workers.
- **Full Production Regression Suite:**
  - All 4 test groups in `tests/standalone_stub_check.cpp` execute and validate production symbols directly.


## 1.5.3
- **Settings GUI Fluent Dark Styling:**
  - Integrated `uxtheme.dll` (`SetWindowTheme(..., L"DarkMode_Explorer", nullptr)`) across tab controls, combo boxes, checkboxes, and buttons.
  - Added color handling for `WM_CTLCOLORLISTBOX` and `WM_CTLCOLOREDIT` so dropdown menus and text fields match the dark Fluent palette.
  - Resolved `MONITORINFO` struct initialization warning.
- **Direct2D Device Loss Recovery:**
  - Enhanced `liquid_glass.h` to automatically reset and recreate Direct2D bitmaps upon `CopyFromMemory` failures or display mode changes, preventing stale textures or crashes during GPU device loss.
- **Robust Worker Thread Lifecycle:**
  - Categorized startup into critical core threads (`g_renderThread`, events) which fail startup safely on error, and auxiliary feature workers (media, weather, lyrics, input) which gracefully degrade with explicit diagnostics.
- **Configurable Bluetooth Filtering & Full State Tracking:**
  - Added `Modules.BluetoothAlertOnGenericBle` setting (default `false`) allowing users to optionally receive alerts for raw BLE sensors without default background polling spam.
  - Track `isBle` property dynamically across runtime connection events as well as initial enumeration.
- **Automated CI Test Execution:**
  - Updated `ci/build.yml` to compile and execute the test suite in CI natively on Windows and via Wine on Linux.
  - Expanded `tests/standalone_stub_check.cpp` with unit tests for launcher parsing, 16% opacity clamping, and Bluetooth filtering logic.


## 1.5.2
- **Fixed Opacity Clamp:** Lowered minimum opacity clamp from 35% to 10% in `dynamic-island-ultra.cpp`, so selecting **16% (Crystal Clear Transparent)** in the Settings GUI now accurately renders at 16% opacity.
- **Perfected Bluetooth State Machine:**
  - Resolved startup suppression bug where `NowSeconds() - 0.0 < 5.0` blocked valid disconnects during the first 5 seconds.
  - Suppressed phantom disconnect alerts from generic background BLE peripherals (e.g. `GP325`) transitioning to sleep, while retaining disconnect alerts for active headsets, speakers, and input devices.
  - Guaranteed tracking state updates even when notification popups are disabled.
- **Branded Weather User-Agent:** Updated weather HTTP client from generic `DynamicIsland/1.0` to `DynamicIslandForWindows/1.5.2 (https://github.com/aathishk/Dynamic-island-rea)`.
- **Settings GUI Dark Theme & Multi-Monitor Centering:**
  - Window class now uses a solid dark background brush (`RGB(28, 28, 32)`), preventing white background flashes.
  - Handled `WM_CTLCOLORDLG`, `WM_CTLCOLORSTATIC`, `WM_CTLCOLORBTN` for consistent Fluent dark styling.
  - Settings window now centers on the active monitor containing the island instead of forcing primary screen coordinates.
- **Liquid Glass Error Handling:** Added explicit checks for `BitBlt` capture failure and Direct2D texture memory copy HRESULTs.
- **Thread Lifecycle & Logging:** Added error logging for every background worker thread handle created in `StartThreads()`.
- **Standalone Stub Test Runner:** `tests/standalone_stub_check.cpp` now includes a runnable `main()` routine that executes the launcher parser self-test and asserts success.

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
