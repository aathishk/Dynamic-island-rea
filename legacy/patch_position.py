with open("build_ultra.py", "r", encoding="utf-8") as f:
    s = f.read()

# Replace the position menu IDs 101-107 with 71-77 to prevent conflicting with kMediaSourceMenuIdBase (100)
old_pos = """    HMENU posMenu = CreatePopupMenu();
    AppendMenuW(posMenu, MF_STRING | (g_settings.position == Position::TopCenter ? MF_CHECKED : 0), 101, L"Top Center (Default)");
    AppendMenuW(posMenu, MF_STRING | (g_settings.position == Position::TopLeft ? MF_CHECKED : 0), 102, L"Top Left");
    AppendMenuW(posMenu, MF_STRING | (g_settings.position == Position::TopRight ? MF_CHECKED : 0), 103, L"Top Right");
    AppendMenuW(posMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(posMenu, MF_STRING | (g_settings.position == Position::BottomCenter ? MF_CHECKED : 0), 104, L"Bottom Center");
    AppendMenuW(posMenu, MF_STRING | (g_settings.position == Position::BottomLeft ? MF_CHECKED : 0), 105, L"Bottom Left");
    AppendMenuW(posMenu, MF_STRING | (g_settings.position == Position::BottomRight ? MF_CHECKED : 0), 106, L"Bottom Right");
    AppendMenuW(posMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(posMenu, MF_STRING | (g_settings.position == Position::Auto ? MF_CHECKED : 0), 107, L"Adaptive Edge Blade (Auto)");"""

new_pos = """    HMENU posMenu = CreatePopupMenu();
    AppendMenuW(posMenu, MF_STRING | (g_settings.position == Position::TopCenter ? MF_CHECKED : 0), 71, L"Top Center (Default)");
    AppendMenuW(posMenu, MF_STRING | (g_settings.position == Position::TopLeft ? MF_CHECKED : 0), 72, L"Top Left");
    AppendMenuW(posMenu, MF_STRING | (g_settings.position == Position::TopRight ? MF_CHECKED : 0), 73, L"Top Right");
    AppendMenuW(posMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(posMenu, MF_STRING | (g_settings.position == Position::BottomCenter ? MF_CHECKED : 0), 74, L"Bottom Center");
    AppendMenuW(posMenu, MF_STRING | (g_settings.position == Position::BottomLeft ? MF_CHECKED : 0), 75, L"Bottom Left");
    AppendMenuW(posMenu, MF_STRING | (g_settings.position == Position::BottomRight ? MF_CHECKED : 0), 76, L"Bottom Right");
    AppendMenuW(posMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(posMenu, MF_STRING | (g_settings.position == Position::Auto ? MF_CHECKED : 0), 77, L"Adaptive Edge Blade (Auto)");"""

assert old_pos in s, "old_pos not found in build_ultra.py!"
s = s.replace(old_pos, new_pos)

old_switch = """new_switch_cmd = \"\"\"    switch (cmd) {
        case 101:
            WritePrivateProfileStringW(L"Settings", L"Appearance.Position", L"top-center", GetIniFilePath().c_str());
            LoadSettings();
            g_layoutDirty = true;
            break;
        case 102:
            WritePrivateProfileStringW(L"Settings", L"Appearance.Position", L"top-left", GetIniFilePath().c_str());
            LoadSettings();
            g_layoutDirty = true;
            break;
        case 103:
            WritePrivateProfileStringW(L"Settings", L"Appearance.Position", L"top-right", GetIniFilePath().c_str());
            LoadSettings();
            g_layoutDirty = true;
            break;
        case 104:
            WritePrivateProfileStringW(L"Settings", L"Appearance.Position", L"bottom-center", GetIniFilePath().c_str());
            LoadSettings();
            g_layoutDirty = true;
            break;
        case 105:
            WritePrivateProfileStringW(L"Settings", L"Appearance.Position", L"bottom-left", GetIniFilePath().c_str());
            LoadSettings();
            g_layoutDirty = true;
            break;
        case 106:
            WritePrivateProfileStringW(L"Settings", L"Appearance.Position", L"bottom-right", GetIniFilePath().c_str());
            LoadSettings();
            g_layoutDirty = true;
            break;
        case 107:
            WritePrivateProfileStringW(L"Settings", L"Appearance.Position", L"auto", GetIniFilePath().c_str());
            LoadSettings();
            g_layoutDirty = true;
            break;
        case 1:\"\"\""""

new_switch = """new_switch_cmd = \"\"\"    switch (cmd) {
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
        case 1:\"\"\""""

assert old_switch in s, "old_switch not found in build_ultra.py!"
s = s.replace(old_switch, new_switch)

# Also ensure SetWindowPos on moved is hooked into build_ultra.py
old_presented_hook = """content = content.replace(old_switch_cmd, new_switch_cmd)"""

new_presented_hook = """content = content.replace(old_switch_cmd, new_switch_cmd)

old_presented = \"\"\"        const bool presented = UpdateLayeredWindowIndirect(hwnd_, &info) != FALSE;
        if (presented) {
            stageValid_ = true;
            lastOrigin_ = stage.origin;\"\"\"

new_presented = \"\"\"        const bool presented = UpdateLayeredWindowIndirect(hwnd_, &info) != FALSE;
        if (presented) {
            stageValid_ = true;
            if (moved) {
                SetWindowPos(hwnd_, nullptr, stage.origin.x, stage.origin.y, 0, 0,
                             SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            }
            lastOrigin_ = stage.origin;\"\"\"

content = content.replace(old_presented, new_presented)"""

assert old_presented_hook in s, "old_presented_hook not found!"
s = s.replace(old_presented_hook, new_presented_hook, 1)

with open("build_ultra.py", "w", encoding="utf-8") as f:
    f.write(s)

print("build_ultra.py successfully patched!")
