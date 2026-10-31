with open("build_ultra.py", "r", encoding="utf-8") as f:
    text = f.read()

pos11 = text.find("# 11. Dynamic Edge Feature: 3 Customizable 1-Click Quick App Launchers")
submit_pos = text.find("content = content.replace(old_submit_lookup, new_submit_lookup)", pos11)
split_pos = submit_pos + len("content = content.replace(old_submit_lookup, new_submit_lookup)")

pos18 = text.find("# 18. Ensure RenderThreadProc")

replacement_block = '''

# 12. Increase expanded island width & height for generous room across calendar, weather & dashboards
old_media_layout = """namespace MediaLayout {
    constexpr float kExpandedWidth = 380.0f;
    constexpr float kExpandedHeight = 184.0f;"""

new_media_layout = """namespace MediaLayout {
    constexpr float kExpandedWidth = 440.0f;
    constexpr float kExpandedHeight = 196.0f;"""

content = content.replace(old_media_layout, new_media_layout)

# 13. Expand calendar date grid column width for generous breathing room
old_cal_colw = """const float colW = 28.0f * scale;"""
new_cal_colw = """const float colW = 38.0f * scale;"""

content = content.replace(old_cal_colw, new_cal_colw)

# 14. Eliminate secondary backdrop windows to completely prevent rectangular grey box artifacts
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

# 15. Enable soft drop shadow with smart radius (pill when collapsed, comfortable squircle when expanded)
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

# 16. Impressive Crystal-Clear Transparent Glass Theme with Smart Radius & Ambient Album Art
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

# 17. Dynamic Screen-Behind & Album Art Adaptive Color:
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

# 18. Suppress automatic Do Not Disturb (DND) popups permanently
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

'''

new_text = text[:split_pos] + replacement_block + text[pos18:]
with open("build_ultra.py", "w", encoding="utf-8") as f:
    f.write(new_text)

print("Updated build_ultra.py successfully!")
