# Dynamic Edge: Liquid Glass Dynamic Island for Windows 11 & 10

A fluid, crystal-clear Dynamic Island and edge dock experience engineered for Windows 11 and 10 with sub-pixel spring physics, universal media controls, master volume flyouts, and quick application launching.

## Features

- **Crystal-Clear Liquid Glassmorphism**: Translucent dark glass aesthetics with optical specular gradient, ambient album art bloom, and hairline dual-tone bevel rims.
- **Universal Media Controls & SMTC**: Full integration with Windows System Media Transport Controls (Spotify, Apple Music, YouTube, Chrome, Edge, VLC).
- **Master Volume Flyout & Mouse Wheel Scrubbing**: Real-time volume percentage readout with mute toggle and instant mouse-wheel adjustments over the speaker icon.
- **Multi-Session Media Switcher**: Cycle across multiple active playing media apps with a single click.
- **Smart Adaptive Squircles**: Semicircular pill capsule when resting/collapsed; modern 26px squircle when expanded with spacious 440px layout for calendar, weather, and dashboards.
- **Smart Quiet Feed Mode**: Suppresses repetitive notification banners when browsing short-form videos (Reels, TikTok, Shorts).
- **1-Click Quick App Launchers**: Instant access to Terminal, File Explorer, Browser, and customizable tools.
- **Private & Lightweight**: Operates 100% offline with zero telemetry and minimal RAM footprint.

## Project Structure

- `dynamic-island-ultra.cpp` — Full monolithic C++ Direct2D engine with Dynamic Edge features.
- `standalone_shim.h` — Standalone runtime shim providing configuration management and desktop hook isolation.
- `compile_exe.py` — Compiles `DynamicIsland.exe` using Clang C++23.
- `compile_mod.py` — Builds the Windhawk DLL mod variant.
- `build_ultra.py` — Monolithic source transformer and patch pipeline.
- `run_island.py` — Service and process launcher with crash recovery.

## Building from Source

```powershell
python compile_exe.py
```
Outputs `DynamicIsland.exe` directly ready to run.
