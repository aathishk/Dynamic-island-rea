# Changelog

## 1.7.0 — Adaptive Desktop Companion
- **Visual Quality & Interaction Sprint (Audit Verdict Remediation):**
  - **Overview Measured Typography & Zero-Overlap Layout:** Replaced fixed vertical layout rectangles in `DrawOverviewDashboard()` with true DirectWrite layout measurement (`CreateTextLayout` & `GetMetrics`), enforcing a strict Date $\rightarrow$ Clock $\rightarrow$ Status Chips vertical hierarchy with strictly $\ge 14.0\text{px}$ visual separation between clock glyph bounds and chip backgrounds across all display scales.
  - **Status Chips Ellipsis Trimming & Constrained Row:** Implemented proportional chip width distribution and DirectWrite character trimming with ellipsis (`CreateEllipsisTrimmingSign`), guaranteeing background pills never shrink below $56\text{px}$ or overflow row margins/pagination dots even with extreme localized weather descriptions.
  - **Active Dashboard Page Surface Decoupling & Opaque Obsidian Backdrop:** Decoupled surface material in `DrawPillSurface()` by active dashboard page (`activePageId`) rather than activity kind, guaranteeing data dashboards (Hardware Monitor, Overview, Calendar, Weather, File Tray) receive a deep opaque obsidian backdrop ($\ge 0.96\text{--}1.0$ alpha) and solid cards ($0.94\text{--}0.96$ alpha) with zero text bleed-through over bright documents and code editors, while strictly preserving crystal-clear $16\%$ liquid glass for the collapsed pill and translucent reactive bloom for the Media Player. Wired `CalculateOverviewLayoutMetrics()` directly into the runtime `DrawOverviewDashboard()` rendering path for 100% unit-tested code coverage.
  - **Settings & Profile Studio Layout & Full Client Sizing:** Redesigned tab header separation using $y=56\text{px}$ anchoring to completely eliminate tab header overlap, eliminated white tab pane background by sizing tab control to header height and subclassing for dark `RGB(28, 28, 32)` theme, and enforced `AdjustWindowRectEx` for full $670 \times 535$ client area without clipping.
  - **Initial Expanded Page on Playback Transition:** Automatically switches to the dedicated Media Player once upon transition into active playback, preserves user navigation to other dashboards during playback, and safely falls back to configured default dashboard on pause timeout expiration.
  - **Full Configuration Coverage for Profile Studio:** Added serialization, INI persistence, and Save-As capture for all 7 companion settings: `defaultDashboardPage`, `mediaPausedTimeoutSeconds`, `rememberLastPage`, `genericDuplicateDebounce`, `moduleQuickControls`, `moduleGameOverlay`, and `gameOverlayMode`.
  - **Comprehensive Regression Test Coverage:** Expanded test suite 12 in `tests/standalone_stub_check.cpp` to verify layout metric calculations, chip width clamping, media playback state transitions, default page parsing, and complete profile disk round-trip persistence.
- **Personalisation Studio & Profile Manager (`profile_policy.h`, `profile_manager.h`):**
  - Six built-in personality profiles: `Obsidian` (minimal 16% crystal obsidian base, balanced battery efficiency), `Frost` (translucent frosted optical glass), `Midnight` (deep navy with electric blue accents), `Aurora` (dynamic album bloom & color gradients), `Minimal` (compact 0.8x, reduced motion, maximum battery saver), and `Studio` (information-dense workstation layout).
  - Full profile management: Apply, Save Current as Custom, Duplicate, Rename, Delete, Reset, and Import/Export via `%APPDATA%\DynamicIsland\profiles\`.
  - Atomic configuration writes with `.bak` safety backups and corrupt profile fallback.
- **Context Engine & Application Rules (`context_policy.h`):**
  - Adaptive Context Modes: `Normal`, `Focus`, `Gaming`, `Presentation`, and `Custom`.
  - Application-aware rule matcher: maps foreground executables (e.g. `Code.exe` $\rightarrow$ Focus / Studio) with priority resolution and case-insensitivity.
  - Windows presentation and fullscreen game detection via `SHQueryUserNotificationState`.
  - Manual context locking with optional temporary duration timers.
- **Central Notification Engine & Quiet Hours (`notification_policy.h`, `notification_history.h`):**
  - Priority system: `Critical`, `Normal`, and `Quiet`.
  - Quiet hours schedule with support for midnight wrap-around (e.g. 22:00 to 07:00).
  - Fullscreen and gaming suppression for non-critical alert cards.
  - Priority-arbitrated, de-duplicated notification queue.
  - Opt-in, privacy-first local notification history with individual dismiss and clear-all actions.
- **Battery-Conscious Performance Engine (`performance_policy.h`):**
  - Live system power state monitoring (`GetSystemPowerStatus`, `WM_POWERBROADCAST`).
  - Automatic Battery Saver adaptation on DC battery power / low battery (<20%), automatically throttling idle rendering and disabling expensive optical backdrop capture.
  - Real measurable process diagnostics: process CPU percentage delta (`GetProcessTimes`), working-set memory (`GetProcessMemoryInfo`), and worker health metrics.
- **Quick Control Centre (`quick_controls.h`):**
  - Native companion control panel with master volume (`IAudioEndpointVolume`), display brightness, Wi-Fi, Bluetooth, power settings, Night Light, Task Manager, and screen snipping (`ms-screenclip:`).
- **Pomodoro Focus Timer Improvements (`focus_timer.h`):**
  - Pomodoro work sessions, short breaks, long breaks, and session counting using monotonic clocks (`GetTickCount64`).
  - Automatic Focus context activation when a session starts.
- **Privacy-First Local Text Clipboard Manager (`clipboard_manager.h`):**
  - 100% local, opt-in (disabled by default) text clipboard panel with pin/delete and one-click clear.
- **Adaptive Island Layout Model (`island_layout.h`):**
  - Coherent priority arbitration between Collapsed, Compact Media, Compact Timer, Notification Card, Expanded Dashboard, and Quick Control states.
- **Configurable Gestures & Safe Quick Actions (`gesture_manager.h`):**
  - Configurable island clicks (single, double, middle) and safe execution of quick actions (disallowing command interpreters).
- **Settings & Profile Studio Redesign (`settings_window.h`):**
  - Upgraded to 6 comprehensive Fluent dark tabs: Overview & Profiles, Personalisation, Behaviour & Gestures, Notifications & History, Performance & Diagnostics, and Productivity & Modules.
  - Integrated 30-second first-run onboarding setup dialog.
- **Single Authoritative Dashboard Navigation Model (island_layout.h, dynamic-island-ultra.cpp):**
  - Unified DashboardPageId navigation model (Overview, Media, Calendar, Weather, HardwareMonitor, FileTray, Lyrics, QuickControls, GameOverlay).
  - Single authoritative collection BuildActiveDashboardPages(...) driving expanded rendering, tab transitions, pagination dots, mouse wheel, and keyboard arrow/Tab navigation with zero index drifting.
  - Eliminated phantom tabs by strictly gating Lyrics and Media tab availability on unblocked media sessions (!MediaExpandBlocked).
- **Glanceable Overview Dashboard (DrawOverviewDashboard):**
  - Integrated typographic Clock and Date with radial accent bloom.
  - Glanceable status chips row: Battery level / charging state, live Weather summary, and real-time CPU/RAM telemetry chip.
  - Contextual compact media line or active camera/microphone privacy indicators.
- **Game Overlay Mode & Hover Overhaul (Modules.GameOverlayMode):**
  - Prevented hover expansion suppression when Game Overlay is active.
  - Added user-selectable GameOverlayMode: DashboardOnly (tab only), CollapsedHud (persistent HUD strip), or GamingContextOnly (HUD strip only during gaming context/fullscreen).
  - Renamed metrics to clear standard definitions: Overlay FPS and C: STORAGE.
- **Media Session Arbitration & Idle Timeout (Modules.MediaPausedTimeoutSeconds):**
  - Monotonic paused timestamp tracking (pausedAt) with configurable fallback timeout (default 300s) returning cleanly to Idle Clock state.
- **Category-Specific Notification Debouncing:**
  - Separated Notifications.SuppressBleSensorDuplicates (BLE connection/sensor spams) from Notifications.GenericDuplicateDebounce (generic application toasts).
- **Theme System Single Source of Truth (island_policy::kAllThemePalettes):**
  - Direct consumption of unified palette definitions across runtime, right-click context menu, settings UI, and unit test suites.
- **Comprehensive 12-Group Unit & Regression Test Suite:**
  - Expanded `tests/standalone_stub_check.cpp` from 5 to 11 test suites covering all policies with 100% test pass.

## 1.5.5
- **Thread Shutdown Safety & Wait Classification:**
  - `StopThreads()` now strictly requires `WAIT_OBJECT_0` before closing worker thread handles.
  - `WAIT_TIMEOUT`, `WAIT_FAILED` (logging `GetLastError()`), and unexpected wait codes preserve thread handles and keep synchronization events alive, preventing handle invalidation and use-after-free crashes.
  - Added shared `WorkerWaitResult` classification policy (`island_policy.h`) with unit test verification.
- **Dedicated Gamepad Category & Glyph:**
  - Added `BluetoothDeviceCategory::Gamepad` mapped to Xbox, DualSense, DualShock, and controller peripherals.
  - Dedicated Segoe Fluent/MDL2 gamepad glyph `\uE7FC` in `DrawBluetoothCategoryIcon()`.
  - Disconnect policy ensures gamepad disconnects are never suppressed.
- **Bluetooth Crash Guard for Windows N / Server Editions:**
  - Wrapped `BluetoothThreadProc` in a comprehensive exception guard distinguishing `winrt::hresult_error` (specifically detecting `REGDB_E_CLASSNOTREG` `0x80040154` when WinRT Bluetooth classes are unregistered on stripped Windows editions) and `std::exception` (logging `ex.what()`).
  - Implemented RAII `ApartmentScope` ensuring `winrt::uninit_apartment()` runs cleanly on all exit and exception paths.
- **Complete Direct2D Device-Dependent Resource Recovery:**
  - Added `ReleaseDeviceDependentResources()` in `Renderer`: resets all persistent brushes, bitmap textures, tab layers, and gradient slots upon `D2DERR_RECREATE_TARGET`, preventing `D2DERR_WRONG_RESOURCE_DOMAIN` errors.
  - Added `EnsureRenderTarget()` and `CreateBackingBitmap()` return value verification during device-loss recovery; safe fallback (`stageValid_ = false`) if GPU allocation fails.
- **Fixed Bluetooth Startup State Tracking & Debounce:**
  - Track devices discovered at launch with `isConnected = true` in the initial device cache, enabling `Modules.BluetoothAlertOnGenericBle = 1` to alert on disconnects for devices connected before app boot.
  - Reset `lastDisconnectTime = -100.0` upon new device connections so valid disconnects following a reconnect are never falsely suppressed.
- **Live Worker Health Lifecycle & Clean Diagnostics:**
  - `spawnOptionalWorker` manages a runner wrapper recording worker names and logging exception details on termination.
- **Active GitHub Actions CI Workflow:**
  - Moved CI workflow into standard `.github/workflows/build.yml` with Linux Wine testing and Windows native build/test gating.
  - Expanded unit test suite to 5 test groups (launchers, opacity policy, BT policy, gamepad classification, thread wait shutdown policy).


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
