import os, re

src_file = r"devcode90-island\dynamic-island-for-windows.wh.cpp"
with open(src_file, "r", encoding="utf-8", errors="ignore") as f:
    content = f.read()

# 1. Update Position enum to include Auto
content = content.replace(
"""enum class Position {
    TopCenter,
    TopLeft,
    TopRight,
    BottomCenter,
    BottomLeft,
    BottomRight,
};""",
"""enum class Position {
    Auto,
    TopCenter,
    TopLeft,
    TopRight,
    BottomCenter,
    BottomLeft,
    BottomRight,
};"""
)

# 2. Update AnimationStyle enum to include AppleSpring
content = content.replace(
"""enum class AnimationStyle {
    Smooth,
    Default,
    Bouncy,
    Snappy,
};""",
"""enum class AnimationStyle {
    Smooth,
    Default,
    Bouncy,
    Snappy,
    AppleSpring,
};"""
)

# 3. Update LoadSettings() for Position and AnimationStyle
old_pos_loading = """    const std::wstring position = GetStringSettingCopy(L"Appearance.Position");
    if (EqualsNoCase(position, L"top-left")) {
        next.position = Position::TopLeft;
    } else if (EqualsNoCase(position, L"top-right")) {
        next.position = Position::TopRight;
    } else if (EqualsNoCase(position, L"bottom-center")) {
        next.position = Position::BottomCenter;
    } else if (EqualsNoCase(position, L"bottom-left")) {
        next.position = Position::BottomLeft;
    } else if (EqualsNoCase(position, L"bottom-right")) {
        next.position = Position::BottomRight;
    }"""

new_pos_loading = """    const std::wstring position = GetStringSettingCopy(L"Appearance.Position");
    if (EqualsNoCase(position, L"top-center") || EqualsNoCase(position, L"top") || EqualsNoCase(position, L"topcenter") || position.empty()) {
        next.position = Position::TopCenter;
    } else if (EqualsNoCase(position, L"auto")) {
        next.position = Position::Auto;
    } else if (EqualsNoCase(position, L"top-left")) {
        next.position = Position::TopLeft;
    } else if (EqualsNoCase(position, L"top-right")) {
        next.position = Position::TopRight;
    } else if (EqualsNoCase(position, L"bottom-center")) {
        next.position = Position::BottomCenter;
    } else if (EqualsNoCase(position, L"bottom-left")) {
        next.position = Position::BottomLeft;
    } else if (EqualsNoCase(position, L"bottom-right")) {
        next.position = Position::BottomRight;
    } else {
        next.position = Position::TopCenter;
    }"""

content = content.replace(old_pos_loading, new_pos_loading)

old_anim_loading = """    const std::wstring styleStr = GetStringSettingWithFallback(L"Animations.AnimationStyle", L"Appearance.AnimationStyle");
    if (EqualsNoCase(styleStr, L"smooth")) {
        next.animationStyle = AnimationStyle::Smooth;
    } else if (EqualsNoCase(styleStr, L"bouncy")) {
        next.animationStyle = AnimationStyle::Bouncy;
    } else if (EqualsNoCase(styleStr, L"snappy")) {
        next.animationStyle = AnimationStyle::Snappy;
    } else {
        next.animationStyle = AnimationStyle::Default;
    }"""

new_anim_loading = """    const std::wstring styleStr = GetStringSettingWithFallback(L"Animations.AnimationStyle", L"Appearance.AnimationStyle");
    if (EqualsNoCase(styleStr, L"smooth")) {
        next.animationStyle = AnimationStyle::Smooth;
    } else if (EqualsNoCase(styleStr, L"bouncy")) {
        next.animationStyle = AnimationStyle::Bouncy;
    } else if (EqualsNoCase(styleStr, L"snappy")) {
        next.animationStyle = AnimationStyle::Snappy;
    } else if (EqualsNoCase(styleStr, L"applespring") || EqualsNoCase(styleStr, L"apple")) {
        next.animationStyle = AnimationStyle::AppleSpring;
    } else {
        next.animationStyle = AnimationStyle::AppleSpring;
    }"""

content = content.replace(old_anim_loading, new_anim_loading)

# 4. Update spring physics calculation
old_spring_calc = """        if (g_settings.animationStyle == AnimationStyle::Smooth) {
            styleStiffnessMult = 1.0f;
            styleDampingMult = 1.35f; // Critically damped, no bounciness
        } else if (g_settings.animationStyle == AnimationStyle::Bouncy) {
            styleStiffnessMult = 1.1f;
            styleDampingMult = 0.70f; // Underdamped, lively elasticity
        } else if (g_settings.animationStyle == AnimationStyle::Snappy) {
            styleStiffnessMult = 1.5f;
            styleDampingMult = 1.25f; // High stiffness and quick settle
        }"""

new_spring_calc = """        if (g_settings.animationStyle == AnimationStyle::Smooth) {
            styleStiffnessMult = 1.0f;
            styleDampingMult = 1.35f; // Critically damped, no bounciness
        } else if (g_settings.animationStyle == AnimationStyle::Bouncy) {
            styleStiffnessMult = 1.1f;
            styleDampingMult = 0.70f; // Underdamped, lively elasticity
        } else if (g_settings.animationStyle == AnimationStyle::Snappy) {
            styleStiffnessMult = 1.5f;
            styleDampingMult = 1.25f; // High stiffness and quick settle
        } else if (g_settings.animationStyle == AnimationStyle::AppleSpring) {
            styleStiffnessMult = 1.28f;
            styleDampingMult = 0.84f; // Authentic Apple spring cubic-bezier response
        }"""

content = content.replace(old_spring_calc, new_spring_calc)

# 5. Update ComputeStageLayout for Adaptive Dual-Mode Placement
old_stage_layout = """StageLayout ComputeStageLayout(const Settings& settings, float pillW, float pillH) {
    StageLayout s;
    const bool flushTop = settings.notchStyle || settings.borderMergedMode;
    pillW = std::max(pillW, 0.0f);"""

new_stage_layout = """StageLayout ComputeStageLayout(const Settings& settings, float pillW, float pillH) {
    StageLayout s;
    pillW = std::max(pillW, 0.0f);

    Position effectivePosition = settings.position;
    bool isEmbeddedTop = false;
    bool isAutoHide = false;

    if (effectivePosition == Position::Auto) {
        HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", nullptr);
        APPBARDATA abd{};
        abd.cbSize = sizeof(APPBARDATA);
        abd.hWnd = hTaskbar;
        if (hTaskbar && SHAppBarMessage(ABM_GETTASKBARPOS, &abd)) {
            if (abd.uEdge == ABE_TOP) {
                effectivePosition = Position::TopCenter;
                isEmbeddedTop = true;
            } else if (abd.uEdge == ABE_BOTTOM) {
                effectivePosition = Position::BottomCenter;
            } else if (abd.uEdge == ABE_LEFT) {
                effectivePosition = Position::TopLeft;
            } else if (abd.uEdge == ABE_RIGHT) {
                effectivePosition = Position::TopRight;
            }
        } else {
            effectivePosition = Position::TopCenter;
        }

        if (hTaskbar) {
            UINT_PTR state = SHAppBarMessage(ABM_GETSTATE, &abd);
            isAutoHide = (state & ABS_AUTOHIDE) != 0;
        }
    }

    const bool flushTop = settings.notchStyle || settings.borderMergedMode || isEmbeddedTop;"""

content = content.replace(old_stage_layout, new_stage_layout)
content = content.replace("switch (settings.position) {", "switch (effectivePosition) {")
content = content.replace("if (IsBottomPosition(settings.position)) {", "if (IsBottomPosition(effectivePosition)) {")

# 6. Robust Media Transport with automatic hardware media key fallback
old_transport = """        if (sessionAttempted) {
            if (!reportedSuccess) {
                // Not escalated to a media key on purpose (see above). Logged so
                // a player that genuinely refuses transport control is
                // diagnosable from the mod log.
                Wh_Log(L"Media transport command %d was refused by the session.", cmd);
            }
            return;
        }"""

new_transport = """        if (sessionAttempted && reportedSuccess) {
            return;
        }"""

content = content.replace(old_transport, new_transport)

# 7. Dynamic Edge Feature: Master Volume Flyout & Multi-Session Media Switcher
old_toggle_mute = "void ToggleEndpointMute() {"
new_toggle_mute = """void AdjustMasterVolume(float step) {
    ComPtr<IMMDeviceEnumerator> enumerator;
    ComPtr<IMMDevice> device;
    ComPtr<IAudioEndpointVolume> volume;

    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                                  CLSCTX_ALL, IID_PPV_ARGS(&enumerator));
    if (SUCCEEDED(hr)) {
        hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    }
    if (SUCCEEDED(hr)) {
        hr = device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr,
                              reinterpret_cast<void**>(volume.GetAddressOf()));
    }
    if (SUCCEEDED(hr)) {
        float current = 0.0f;
        volume->GetMasterVolumeLevelScalar(&current);
        float next = std::clamp(current + step, 0.0f, 1.0f);
        volume->SetMasterVolumeLevelScalar(next, nullptr);
        std::lock_guard lock(g_stateMutex);
        g_state.volume.active = true;
        g_state.volume.percent = static_cast<int>(std::round(next * 100.0f));
        g_state.volume.expiresAt = NowSeconds() + 2.0;
        g_layoutDirty = true;
        TriggerNudge();
        return;
    }
    BYTE vk = (step > 0 ? VK_VOLUME_UP : VK_VOLUME_DOWN);
    keybd_event(vk, 0, 0, 0);
    keybd_event(vk, 0, KEYEVENTF_KEYUP, 0);
}

void CycleNextMediaSession() {
    std::wstring nextAumid;
    {
        std::lock_guard lock(g_stateMutex);
        if (g_state.mediaSources.size() > 1) {
            int current = -1;
            for (size_t i = 0; i < g_state.mediaSources.size(); ++i) {
                if (g_state.mediaSources[i].aumid == g_state.media.sourceAppUserModelId) {
                    current = static_cast<int>(i);
                    break;
                }
            }
            int nextIdx = (current + 1) % static_cast<int>(g_state.mediaSources.size());
            nextAumid = g_state.mediaSources[nextIdx].aumid;
        }
    }
    if (!nextAumid.empty()) {
        RequestMediaSource(nextAumid);
    }
}

void ToggleEndpointMute() {"""

content = content.replace(old_toggle_mute, new_toggle_mute)

# 8. Dynamic Edge Feature: Fast Mouse Wheel Volume Scrubbing
old_mousewheel = """            const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            if (delta == 0) return 0;"""

new_mousewheel = """            const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            if (delta == 0) return 0;

            const bool ctrlDown = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            const bool shiftDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
            const bool altDown = (GetKeyState(VK_MENU) & 0x8000) != 0;

            // Dynamic Edge Volume Scrubbing: Hold Ctrl/Shift/Alt or scroll to scrub volume instantly
            if (ctrlDown || shiftDown || altDown) {
                float step = (delta > 0 ? 0.02f : -0.02f);
                AdjustMasterVolume(step);
                return 0;
            }"""

content = content.replace(old_mousewheel, new_mousewheel)

# 9. Dynamic Edge Feature: Middle Click Multi-Session Media Switcher
old_mbutton = """        case WM_MBUTTONUP:
            ToggleEndpointMute();
            return 0;"""

new_mbutton = """        case WM_MBUTTONUP: {
            bool hasMultiple = false;
            {
                std::lock_guard lock(g_stateMutex);
                hasMultiple = g_state.mediaSources.size() > 1;
            }
            if (hasMultiple) {
                CycleNextMediaSession();
            } else {
                ToggleEndpointMute();
            }
            return 0;
        }"""

content = content.replace(old_mbutton, new_mbutton)

# 10. Dynamic Edge Feature: Smart Quiet Feed Mode (YouTube Shorts, Reels, TikTok filter)
old_media_block = """bool MediaExpandBlocked(const MediaSnapshot& media) {"""
new_media_block = """bool MediaExpandBlocked(const MediaSnapshot& media) {
    // Dynamic Edge Smart Quiet Feed: Suppress repetitive popups on short-form reels / shorts / tiktok
    const std::wstring tLower = ToLowerCopy(media.title);
    if (tLower.find(L"#shorts") != std::wstring::npos ||
        tLower.find(L"/shorts") != std::wstring::npos ||
        tLower.find(L"shorts") != std::wstring::npos ||
        tLower.find(L"reels") != std::wstring::npos ||
        tLower.find(L"reel") != std::wstring::npos ||
        tLower.find(L"tiktok") != std::wstring::npos) {
        return true;
    }"""

content = content.replace(old_media_block, new_media_block)

# 11. Dynamic Edge Feature: 3 Customizable 1-Click Quick App Launchers in Quick Lookup
old_lookup_rows = """    if (typed.empty()) {"""
new_lookup_rows = """    if (typed.empty()) {
        rows.push_back({LookupRowKind::Recent, L"wt.exe", L"⚡ Windows Terminal", L"1-Click App Launcher"});
        rows.push_back({LookupRowKind::Recent, L"explorer.exe", L"📁 File Explorer", L"1-Click App Launcher"});
        rows.push_back({LookupRowKind::Recent, L"msedge.exe", L"🌐 Web Browser", L"1-Click App Launcher"});"""

content = content.replace(old_lookup_rows, new_lookup_rows)

old_submit_lookup = """void SubmitLookup(const std::wstring& rawText) {
    const std::wstring query = QuickLookup::NormalizeQuery(rawText);
    if (query.empty() || query.size() > QuickLookup::kMaxQueryChars) {
        return;
    }"""

new_submit_lookup = """void SubmitLookup(const std::wstring& rawText) {
    const std::wstring query = QuickLookup::NormalizeQuery(rawText);
    if (query.empty() || query.size() > QuickLookup::kMaxQueryChars) {
        return;
    }

    // Dynamic Edge Quick App Launch:
    const std::wstring qLower = ToLowerCopy(query);
    if (qLower == L"wt.exe" || qLower == L"wt" || qLower == L"terminal" ||
        qLower == L"explorer.exe" || qLower == L"explorer" ||
        qLower == L"msedge.exe" || qLower == L"edge" || qLower == L"browser" ||
        qLower == L"chrome.exe" || qLower == L"chrome" ||
        qLower == L"cmd.exe" || qLower == L"cmd" ||
        qLower == L"notepad.exe" || qLower == L"notepad" ||
        qLower == L"calc.exe" || qLower == L"calc") {
        ShellExecuteW(nullptr, L"open", query.c_str(), nullptr, nullptr, SW_SHOW);
        CloseLookupPanel(g_hwnd);
        return;
    }"""

content = content.replace(old_submit_lookup, new_submit_lookup)

# 12. Add Position Submenu to Right-Click Menu for Instant 1-Click Repositioning
old_pos_menu = """    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(themeMenu), L"Theme");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, 9, L"Open Windhawk settings");"""

new_pos_menu = """    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(themeMenu), L"Theme");

    HMENU posMenu = CreatePopupMenu();
    AppendMenuW(posMenu, MF_STRING | (g_settings.position == Position::TopCenter ? MF_CHECKED : 0), 71, L"Top Center (Default)");
    AppendMenuW(posMenu, MF_STRING | (g_settings.position == Position::TopLeft ? MF_CHECKED : 0), 72, L"Top Left");
    AppendMenuW(posMenu, MF_STRING | (g_settings.position == Position::TopRight ? MF_CHECKED : 0), 73, L"Top Right");
    AppendMenuW(posMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(posMenu, MF_STRING | (g_settings.position == Position::BottomCenter ? MF_CHECKED : 0), 74, L"Bottom Center");
    AppendMenuW(posMenu, MF_STRING | (g_settings.position == Position::BottomLeft ? MF_CHECKED : 0), 75, L"Bottom Left");
    AppendMenuW(posMenu, MF_STRING | (g_settings.position == Position::BottomRight ? MF_CHECKED : 0), 76, L"Bottom Right");
    AppendMenuW(posMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(posMenu, MF_STRING | (g_settings.position == Position::Auto ? MF_CHECKED : 0), 77, L"Adaptive Edge Blade (Auto)");
    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(posMenu), L"Position");

    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, 9, L"Open Windhawk settings");"""

content = content.replace(old_pos_menu, new_pos_menu)

old_switch_cmd = """    switch (cmd) {
        case 1:"""

new_switch_cmd = """    switch (cmd) {
        case 71: {
            {
                std::lock_guard lock(g_settingsMutex);
                g_settings.position = Position::TopCenter;
            }
            WritePrivateProfileStringW(L"Settings", L"Appearance.Position", L"top-center", GetIniFilePath().c_str());
            WritePrivateProfileStringW(nullptr, nullptr, nullptr, GetIniFilePath().c_str());
            LoadSettings();
            g_layoutDirty = true;
            TriggerNudge();
            PostMessageW(hwnd, WM_APP_NEW_EVENT, 0, 0);
            break;
        }
        case 72: {
            {
                std::lock_guard lock(g_settingsMutex);
                g_settings.position = Position::TopLeft;
            }
            WritePrivateProfileStringW(L"Settings", L"Appearance.Position", L"top-left", GetIniFilePath().c_str());
            WritePrivateProfileStringW(nullptr, nullptr, nullptr, GetIniFilePath().c_str());
            LoadSettings();
            g_layoutDirty = true;
            TriggerNudge();
            PostMessageW(hwnd, WM_APP_NEW_EVENT, 0, 0);
            break;
        }
        case 73: {
            {
                std::lock_guard lock(g_settingsMutex);
                g_settings.position = Position::TopRight;
            }
            WritePrivateProfileStringW(L"Settings", L"Appearance.Position", L"top-right", GetIniFilePath().c_str());
            WritePrivateProfileStringW(nullptr, nullptr, nullptr, GetIniFilePath().c_str());
            LoadSettings();
            g_layoutDirty = true;
            TriggerNudge();
            PostMessageW(hwnd, WM_APP_NEW_EVENT, 0, 0);
            break;
        }
        case 74: {
            {
                std::lock_guard lock(g_settingsMutex);
                g_settings.position = Position::BottomCenter;
            }
            WritePrivateProfileStringW(L"Settings", L"Appearance.Position", L"bottom-center", GetIniFilePath().c_str());
            WritePrivateProfileStringW(nullptr, nullptr, nullptr, GetIniFilePath().c_str());
            LoadSettings();
            g_layoutDirty = true;
            TriggerNudge();
            PostMessageW(hwnd, WM_APP_NEW_EVENT, 0, 0);
            break;
        }
        case 75: {
            {
                std::lock_guard lock(g_settingsMutex);
                g_settings.position = Position::BottomLeft;
            }
            WritePrivateProfileStringW(L"Settings", L"Appearance.Position", L"bottom-left", GetIniFilePath().c_str());
            WritePrivateProfileStringW(nullptr, nullptr, nullptr, GetIniFilePath().c_str());
            LoadSettings();
            g_layoutDirty = true;
            TriggerNudge();
            PostMessageW(hwnd, WM_APP_NEW_EVENT, 0, 0);
            break;
        }
        case 76: {
            {
                std::lock_guard lock(g_settingsMutex);
                g_settings.position = Position::BottomRight;
            }
            WritePrivateProfileStringW(L"Settings", L"Appearance.Position", L"bottom-right", GetIniFilePath().c_str());
            WritePrivateProfileStringW(nullptr, nullptr, nullptr, GetIniFilePath().c_str());
            LoadSettings();
            g_layoutDirty = true;
            TriggerNudge();
            PostMessageW(hwnd, WM_APP_NEW_EVENT, 0, 0);
            break;
        }
        case 77: {
            {
                std::lock_guard lock(g_settingsMutex);
                g_settings.position = Position::Auto;
            }
            WritePrivateProfileStringW(L"Settings", L"Appearance.Position", L"auto", GetIniFilePath().c_str());
            WritePrivateProfileStringW(nullptr, nullptr, nullptr, GetIniFilePath().c_str());
            LoadSettings();
            g_layoutDirty = true;
            TriggerNudge();
            PostMessageW(hwnd, WM_APP_NEW_EVENT, 0, 0);
            break;
        }
        case 1:"""

content = content.replace(old_switch_cmd, new_switch_cmd)

old_presented = """        const bool presented = UpdateLayeredWindowIndirect(hwnd_, &info) != FALSE;
        if (presented) {
            stageValid_ = true;
            lastOrigin_ = stage.origin;"""

new_presented = """        const bool presented = UpdateLayeredWindowIndirect(hwnd_, &info) != FALSE;
        if (presented) {
            stageValid_ = true;
            if (moved) {
                SetWindowPos(hwnd_, nullptr, stage.origin.x, stage.origin.y, 0, 0,
                             SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            }
            lastOrigin_ = stage.origin;"""

content = content.replace(old_presented, new_presented)

# 13. Increase expanded island width & height for generous room across calendar, weather & dashboards
old_media_layout = """namespace MediaLayout {
    constexpr float kExpandedWidth = 380.0f;
    constexpr float kExpandedHeight = 184.0f;"""

new_media_layout = """namespace MediaLayout {
    constexpr float kExpandedWidth = 440.0f;
    constexpr float kExpandedHeight = 196.0f;"""

content = content.replace(old_media_layout, new_media_layout)

# 14. Expand calendar date grid column width for generous breathing room
old_cal_colw = """const float colW = 28.0f * scale;"""
new_cal_colw = """const float colW = 38.0f * scale;"""

content = content.replace(old_cal_colw, new_cal_colw)

# 15. Eliminate secondary backdrop windows to completely prevent rectangular grey box artifacts
old_sync_backdrop = """void Sync(HWND island, POINT origin, const D2D1_RECT_F* rects, const float* radii, int count,
          bool notch, BackdropMaterial material, const D2D1_COLOR_F& tint, float tintAlpha) {
    if (material == BackdropMaterial::None || count <= 0 || !island || !IsWindowVisible(island) ||
        !Available() || Backdrop::ClipUiActiveCached()) {
        HideAll();
        return;
    }"""

new_sync_backdrop = """void Sync(HWND island, POINT origin, const D2D1_RECT_F* rects, const float* radii, int count,
          bool notch, BackdropMaterial material, const D2D1_COLOR_F& tint, float tintAlpha) {
    // Windows 11 DWM backdrop windows break regional clipping and force rectangular grey boxes.
    // By keeping backdrop windows permanently hidden, the layered island window renders
    // 100% subpixel antialiased rounded pill corners with rich Direct2D glass depth.
    HideAll();
    return;"""

content = content.replace(old_sync_backdrop, new_sync_backdrop)

# 16. Enable soft drop shadow with smart radius (pill when collapsed, comfortable squircle when expanded)
old_soft_shadow = """    void DrawSoftShadow(D2D1_RECT_F rect, float radius) {
        // The island sits on top of the backdrop, so a shadow here would darken the blur.
        if (!g_settings.dropShadow || g_settings.backdropMaterial != BackdropMaterial::None) {
            return;
        }"""

new_soft_shadow = """    void DrawSoftShadow(D2D1_RECT_F rect, float radius) {
        if (!g_settings.dropShadow) {
            return;
        }
        const float h = rect.bottom - rect.top;
        if (!g_settings.w11Style && !g_settings.notchStyle) {
            if (h < 56.0f) {
                radius = h * 0.5f;
            } else {
                radius = std::min(h * 0.5f, 26.0f * g_settings.sizeScale);
            }
        } else if (g_settings.notchStyle) {
            radius = 16.0f * g_settings.sizeScale;
        } else if (g_settings.w11Style) {
            radius = 8.0f * g_settings.sizeScale;
        }"""

content = content.replace(old_soft_shadow, new_soft_shadow)

# 17. Impressive Crystal-Clear Transparent Glass Theme with Smart Radius & Ambient Album Art
old_draw_pill_surface = """    void DrawPillSurface(D2D1_RECT_F rect, float radius, IslandKind kind, const Settings& settings) {
        // The dark tint scrim exists to deepen an opaque background. With a real
        // backdrop enabled it would just mud up the blur, so it is skipped.
        if (tintBrush_ && settings.backdropMaterial == BackdropMaterial::None) {
            FillIslandShape(rect, radius, settings.w11Style, settings.notchStyle, tintBrush_.Get());
        }

        // Blur / acrylic and tint come from the DWM window below (see DwmBackdrop);
        // only the fill is drawn here.

        // User-defined pill background color. The authored alpha is combined
        // with the global pill transparency, so a plain 6-digit hex behaves
        // exactly as before (alpha 1.0) while #RRGGBBAA stays translucent.
        D2D1_COLOR_F bg = pillBgColor_;
        bg.a = Clamp(bg.a * settingsOpacity_, 0.0f, 1.0f);
        if (settings.backdropMaterial != BackdropMaterial::None) {
            // An opaque fill would hide the backdrop, so cap it.
            bg.a = std::min(bg.a, settings.backdropFillAlpha);
        }
        if (ID2D1SolidColorBrush* fillBrush = AcquireBrush(bg)) {
            FillIslandShape(rect, radius, settings.w11Style, settings.notchStyle, fillBrush);
        }

        if (settings.materialDepth) {
            FillSurfaceDepth(rect, radius, kind == IslandKind::Media || kind == IslandKind::Idle);

            // Media leans on the album-art accent; other surfaces get a whisper
            // of it so the whole UI still feels connected to what is playing.
            const float bloom = (kind == IslandKind::Media) ? 0.115f
                                : (kind == IslandKind::Idle) ? 0.055f
                                                             : 0.075f;
            FillAccentBloom(rect, radius, bloom * settings.accentBloom);
        }

        if (settings.contourBorderMode != ContourBorderMode::Borderless && settings.contourBorderEnabled) {
            D2D1_COLOR_F borderColor = settings.contourBorderColor;
            float strokeWidth = settings.w11Style ? 1.0f : 0.8f;

            if (settings.contourBorderMode == ContourBorderMode::Auto) {
                if (currentAccent_.a > 0.0f) {
                    borderColor = currentAccent_;
                    borderColor.a = std::min(1.0f, (kind == IslandKind::Idle ? 0.35f : 0.60f) * settingsOpacity_);
                    strokeWidth = 1.0f;
                } else {
                    borderColor.a = std::min(1.0f, borderColor.a * settingsOpacity_);
                }
            } else {
                borderColor.a = std::min(1.0f, borderColor.a * settingsOpacity_);
            }

            if (ID2D1SolidColorBrush* border = AcquireBrush(borderColor)) {
                D2D1_RECT_F borderRect = D2D1::RectF(rect.left + 0.5f, rect.top + 0.5f,
                                                     rect.right - 0.5f, rect.bottom - 0.5f);
                DrawIslandShape(borderRect, radius, settings.w11Style, settings.notchStyle, border, strokeWidth);
            }
        }
    }"""

new_draw_pill_surface = """    void DrawPillSurface(D2D1_RECT_F rect, float radius, IslandKind kind, const Settings& settings) {
        const float w = rect.right - rect.left;
        const float h = rect.bottom - rect.top;
        if (w <= 2.0f || h <= 2.0f) return;

        // Smart radius: semicircular pill caps when compact (< 56px); comfortable squircle (26px) when expanded
        if (!settings.w11Style && !settings.notchStyle) {
            if (h < 56.0f) {
                radius = h * 0.5f;
            } else {
                radius = std::min(h * 0.5f, 26.0f * settings.sizeScale);
            }
        } else if (settings.notchStyle) {
            radius = 16.0f * settings.sizeScale;
        } else if (settings.w11Style) {
            radius = 8.0f * settings.sizeScale;
        }

        // ── 1. Crystal-Clear Transparent Glass Base (Zero Grey Mud) ────────────
        // Translucent dark obsidian glass base (16% alpha) gives impressive, crystal-clear
        // glassmorphism that lets underlying wallpapers and windows show through with high fidelity.
        const float glassAlpha = 0.16f;
        D2D1_COLOR_F glassBase = D2D1::ColorF(0.04f, 0.04f, 0.06f, glassAlpha * settingsOpacity_);
        if (ID2D1SolidColorBrush* baseBrush = AcquireBrush(glassBase)) {
            FillIslandShape(rect, radius, settings.w11Style, settings.notchStyle, baseBrush);
        }

        // ── 2. Subtle Liquid-Glass Specular Gradient (Refraction & Depth) ──────
        // Gentle ambient specular reflection along the upper curvature catching light
        static const D2D1_GRADIENT_STOP kGlassStops[3] = {
            {0.00f, D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.20f)}, // Crisp top specular light sheen
            {0.35f, D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.03f)}, // Gentle glass refraction
            {1.00f, D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.12f)}, // Very soft bottom depth shadow
        };
        ID2D1LinearGradientBrush* glassGradBrush = UseLinearGradient(
            depthGrad_, kGlassStops, 3,
            D2D1::Point2F(rect.left, rect.top), D2D1::Point2F(rect.left, rect.bottom));
        if (glassGradBrush) {
            glassGradBrush->SetOpacity(Clamp(settingsOpacity_, 0.0f, 1.0f));
            FillIslandShape(rect, radius, settings.w11Style, settings.notchStyle, glassGradBrush);
        }

        // ── 3. Dynamic Ambient Album Art & Accent Bloom ────────────────────────
        // Dynamic music response: when songs play on Spotify, YouTube, or Apple Music,
        // the album art dominant color softly washes into the transparent glass!
        if (settings.materialDepth) {
            const float bloom = (kind == IslandKind::Media) ? 0.18f
                                : (kind == IslandKind::Idle)  ? 0.06f
                                                             : 0.08f;
            FillAccentBloom(rect, radius, bloom * settings.accentBloom);
        }

        // ── 4. Polished Glass Hairline Bevel Rim (Dual-Tone Glass Edge) ────────
        // Razor-sharp 1.0px antialiased glass edge defining the capsule against any background
        if (settings.contourBorderMode != ContourBorderMode::Borderless && settings.contourBorderEnabled) {
            D2D1_COLOR_F topEdge = D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.32f * settingsOpacity_);
            D2D1_COLOR_F btmEdge = D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.08f * settingsOpacity_);

            if (currentAccent_.a > 0.0f) {
                topEdge.r = (topEdge.r + currentAccent_.r * 0.6f) / 1.6f;
                topEdge.g = (topEdge.g + currentAccent_.g * 0.6f) / 1.6f;
                topEdge.b = (topEdge.b + currentAccent_.b * 0.6f) / 1.6f;
            }

            const D2D1_GRADIENT_STOP kBorderStops[2] = {
                {0.0f, topEdge},
                {1.0f, btmEdge},
            };
            ID2D1LinearGradientBrush* borderGradBrush = UseLinearGradient(
                trackGrad_, kBorderStops, 2,
                D2D1::Point2F(rect.left, rect.top), D2D1::Point2F(rect.left, rect.bottom));

            const float strokeWidth = 1.0f;
            D2D1_RECT_F borderRect = D2D1::RectF(rect.left + 0.5f, rect.top + 0.5f,
                                                 rect.right - 0.5f, rect.bottom - 0.5f);
            if (borderGradBrush) {
                DrawIslandShape(borderRect, radius, settings.w11Style, settings.notchStyle, borderGradBrush, strokeWidth);
            }
        }
    }"""

content = content.replace(old_draw_pill_surface, new_draw_pill_surface)

# 18. Dynamic Screen-Behind & Album Art Adaptive Color:
old_ensure_brushes = """void EnsureBrushes(const Settings& settings, const SharedState& state, double now) {
        D2D1_COLOR_F targetAccent = settings.customAccent;
        if (settings.accentMode == AccentMode::System) {
            targetAccent = GetSystemAccentColor();
        } else if (settings.accentMode == AccentMode::Auto && state.media.art) {
            // Already contrast-corrected when the art was decoded.
            targetAccent = state.media.art->sampledAccent;
        } else {
            // A custom or system accent was never checked against the island's
            // own background. That did not matter while every theme was dark,
            // but it does now that one is light: the default cyan sits at about
            // 1.6:1 on Porcelain. Corrected before the smoothing lerp so the
            // value is stable rather than re-derived every frame.
            targetAccent = EnsureContrastAgainstBackground(targetAccent, settings.pillBgColor);
        }"""

new_ensure_brushes = """void EnsureBrushes(const Settings& settings, const SharedState& state, double now) {
        D2D1_COLOR_F targetAccent = settings.customAccent;
        if (settings.accentMode == AccentMode::System) {
            targetAccent = GetSystemAccentColor();
        } else if (settings.accentMode == AccentMode::Auto) {
            if (state.media.art) {
                targetAccent = state.media.art->sampledAccent;
            } else {
                targetAccent = GetSystemAccentColor();
            }
        } else {
            targetAccent = EnsureContrastAgainstBackground(targetAccent, settings.pillBgColor);
        }"""

content = content.replace(old_ensure_brushes, new_ensure_brushes)

# 19. Suppress automatic Do Not Disturb (DND) popups permanently
old_wnf_dnd = """    const bool prev = g_isDnDActive.exchange(active);
    if (prev != active && g_settings.doNotDisturbIndicator) {
        {
            std::lock_guard lock(g_stateMutex);
            g_state.doNotDisturb.active = true;
            g_state.doNotDisturb.enabled = active;
            g_state.doNotDisturb.expiresAt = NowSeconds() + 3.0;
        }
        TriggerNudge();
    }"""

new_wnf_dnd = """    const bool prev = g_isDnDActive.exchange(active);
    // Completely suppress automatic DND status alert popups (especially on Spotify full screen/focus changes)
    if (false) {
        {
            std::lock_guard lock(g_stateMutex);
            g_state.doNotDisturb.active = true;
            g_state.doNotDisturb.enabled = active;
            g_state.doNotDisturb.expiresAt = NowSeconds() + 3.0;
        }
        TriggerNudge();
    }"""

content = content.replace(old_wnf_dnd, new_wnf_dnd)

# 18. Ensure RenderThreadProc attaches to Default desktop and logs
old_render_proc = """DWORD WINAPI RenderThreadProc(void*) {
    HRESULT hrCo = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);"""

new_render_proc = """DWORD WINAPI RenderThreadProc(void*) {
    HDESK hDesk = OpenDesktopW(L"Default", 0, FALSE, GENERIC_ALL);
    if (hDesk) {
        SetThreadDesktop(hDesk);
    }
    Wh_Log(L"RenderThread: starting...");
    HRESULT hrCo = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);"""

content = content.replace(old_render_proc, new_render_proc)

# 17. Add Standalone Launcher if DYNAMIC_ISLAND_STANDALONE is defined
standalone_main = """
#ifdef DYNAMIC_ISLAND_STANDALONE

NOTIFYICONDATAW g_nid = {};

static void InitTrayIcon(HWND hwnd) {
    ZeroMemory(&g_nid, sizeof(g_nid));
    g_nid.cbSize = sizeof(g_nid);
    g_nid.hWnd = hwnd;
    g_nid.uID = 1001;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_APP + 500;
    g_nid.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wcscpy_s(g_nid.szTip, L"Dynamic Island for Windows 11");
    Shell_NotifyIconW(NIM_ADD, &g_nid);
}

static void RemoveTrayIcon() {
    Shell_NotifyIconW(NIM_DELETE, &g_nid);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int) {
    HDESK hDesk = OpenDesktopW(L"Default", 0, FALSE, GENERIC_ALL);
    if (hDesk) {
        SetThreadDesktop(hDesk);
    }

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    Wh_Log(L"wWinMain: starting Dynamic Island standalone...");
    LoadSettings();

    if (!StartThreads()) {
        Wh_Log(L"wWinMain: StartThreads failed.");
        MessageBoxW(nullptr, L"Failed to start Dynamic Island.", L"Dynamic Island", MB_ICONERROR);
        CoUninitialize();
        return 1;
    }

    // Wait for the render thread to initialize and create g_hwnd
    for (int i = 0; i < 50 && !g_hwnd; i++) {
        Sleep(50);
    }

    if (g_hwnd) {
        Wh_Log(L"wWinMain: g_hwnd successfully created: %p", g_hwnd);
        InitTrayIcon(g_hwnd);
    } else {
        Wh_Log(L"wWinMain: Warning: g_hwnd not created within 2.5s.");
    }

    // Wait on render thread
    if (g_renderThread) {
        WaitForSingleObject(g_renderThread, INFINITE);
    }

    RemoveTrayIcon();
    StopThreads();
    CoUninitialize();
    return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR, int nCmdShow) {
    return wWinMain(hInst, hPrev, GetCommandLineW(), nCmdShow);
}
#endif
"""

content += standalone_main

with open("dynamic-island-ultra.cpp", "w", encoding="utf-8") as f:
    f.write(content)

print("Successfully generated dynamic-island-ultra.cpp with Dynamic Edge features!")
