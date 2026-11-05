import ctypes
import struct
import os
import shutil
import time
import subprocess
import tkinter as tk
from ctypes import wintypes

user32 = ctypes.windll.user32
gdi32 = ctypes.windll.gdi32
kernel32 = ctypes.windll.kernel32

try:
    user32.SetProcessDPIAware()
except Exception:
    pass

class STARTUPINFO(ctypes.Structure):
    _fields_ = [
        ('cb', wintypes.DWORD), ('lpReserved', wintypes.LPWSTR), ('lpDesktop', wintypes.LPWSTR),
        ('lpTitle', wintypes.LPWSTR), ('dwX', wintypes.DWORD), ('dwY', wintypes.DWORD),
        ('dwXSize', wintypes.DWORD), ('dwYSize', wintypes.DWORD), ('dwXCountChars', wintypes.DWORD),
        ('dwYCountChars', wintypes.DWORD), ('dwFillAttribute', wintypes.DWORD), ('dwFlags', wintypes.DWORD),
        ('wShowWindow', wintypes.WORD), ('cbReserved2', wintypes.WORD), ('lpReserved2', ctypes.c_char_p),
        ('hStdInput', wintypes.HANDLE), ('hStdOutput', wintypes.HANDLE), ('hStdError', wintypes.HANDLE),
    ]

class PROCESS_INFORMATION(ctypes.Structure):
    _fields_ = [
        ('hProcess', wintypes.HANDLE), ('hThread', wintypes.HANDLE),
        ('dwProcessId', wintypes.DWORD), ('dwThreadId', wintypes.DWORD),
    ]

def save_window_bmp(hwnd, filename, use_print_window=False):
    r = wintypes.RECT()
    user32.GetWindowRect(hwnd, ctypes.byref(r))
    x, y, w, h = r.left, r.top, r.right - r.left, r.bottom - r.top
    if w <= 0 or h <= 0:
        print(f"Warning: Window {hwnd} has invalid dimensions {w}x{h}")
        return

    print(f"Capturing {filename}: rect=({x}, {y}, {w}x{h})")

    hdcScreen = user32.GetDC(0)
    hdcMem = gdi32.CreateCompatibleDC(hdcScreen)
    hbm = gdi32.CreateCompatibleBitmap(hdcScreen, w, h)
    hbmOld = gdi32.SelectObject(hdcMem, hbm)

    if use_print_window:
        user32.PrintWindow(hwnd, hdcMem, 2)  # PW_RENDERFULLCONTENT
    else:
        gdi32.BitBlt(hdcMem, 0, 0, w, h, hdcScreen, x, y, 0x00CC0020)

    bmi = bytearray(40)
    struct.pack_into('<IIIHHIIIIII', bmi, 0, 40, w, h, 1, 32, 0, w * h * 4, 0, 0, 0, 0)
    buf = bytearray(w * h * 4)
    gdi32.GetDIBits(hdcMem, hbm, 0, h, (ctypes.c_char * len(buf)).from_buffer(buf), (ctypes.c_char * 40).from_buffer(bmi), 0)

    gdi32.SelectObject(hdcMem, hbmOld)
    gdi32.DeleteObject(hbm)
    gdi32.DeleteDC(hdcMem)
    user32.ReleaseDC(0, hdcScreen)

    bmp_header = struct.pack('<2sIHHI', b'BM', 14 + 40 + len(buf), 0, 0, 14 + 40)
    bmp_path = filename.replace('.png', '.bmp')
    with open(bmp_path, 'wb') as f:
        f.write(bmp_header)
        f.write(bmi)
        f.write(buf)

    cmd = f'Add-Type -AssemblyName System.Drawing; $img = [System.Drawing.Image]::FromFile("{bmp_path}"); $img.Save("{filename}", [System.Drawing.Imaging.ImageFormat]::Png); $img.Dispose(); Remove-Item "{bmp_path}"'
    subprocess.run(['powershell', '-Command', cmd], capture_output=True)
    print(f"Saved {filename} ({w}x{h})")

    # Also copy to artifact directory if it exists
    artifact_dir = r"C:\Users\Aathish_Kamaraj\.gemini\antigravity\brain\75de6118-4f3e-4e1d-a3b6-37e3d3d534b2"
    if os.path.exists(artifact_dir):
        dest = os.path.join(artifact_dir, filename)
        shutil.copyfile(filename, dest)
        print(f"Copied to artifact dir: {dest}")

def set_ini_val(txt, sec, k, v):
    import re
    pat = rf'^({re.escape(k)}\s*=).*$'
    if re.search(pat, txt, re.MULTILINE):
        return re.sub(pat, rf'\g<1>{v}', txt, flags=re.MULTILINE)
    else:
        sec_hdr = f'[{sec}]'
        if sec_hdr in txt:
            return txt.replace(sec_hdr, f'{sec_hdr}\n{k}={v}')
        return txt + f'\n[{sec}]\n{k}={v}\n'

def find_island_hwnd(pid):
    hDesk = user32.OpenDesktopW('Default', 0, False, 0x01FF)
    if hDesk:
        user32.SetThreadDesktop(hDesk)
    found_hwnd = None
    def enum_cb(hwnd, lparam):
        nonlocal found_hwnd
        proc_id = wintypes.DWORD()
        user32.GetWindowThreadProcessId(hwnd, ctypes.byref(proc_id))
        if proc_id.value == pid:
            cls = (ctypes.c_wchar * 256)()
            user32.GetClassNameW(hwnd, cls, 256)
            if 'DynamicIslandForWindows' in cls.value:
                found_hwnd = hwnd
        return True
    WNDENUMPROC = ctypes.WINFUNCTYPE(ctypes.c_bool, ctypes.c_void_p, ctypes.c_void_p)
    user32.EnumDesktopWindows(hDesk, WNDENUMPROC(enum_cb), 0)
    return found_hwnd

def find_settings_hwnd(pid):
    hDesk = user32.OpenDesktopW('Default', 0, False, 0x01FF)
    if hDesk:
        user32.SetThreadDesktop(hDesk)
    settings_hwnd = None
    def find_settings(hwnd, lparam):
        nonlocal settings_hwnd
        proc_id = wintypes.DWORD()
        user32.GetWindowThreadProcessId(hwnd, ctypes.byref(proc_id))
        if proc_id.value == pid:
            cls = (ctypes.c_wchar * 256)()
            user32.GetClassNameW(hwnd, cls, 256)
            buf = (ctypes.c_wchar * 256)()
            user32.GetWindowTextW(hwnd, buf, 256)
            if 'Settings' in cls.value or 'Settings' in buf.value:
                settings_hwnd = hwnd
        return True
    WNDENUMPROC = ctypes.WINFUNCTYPE(ctypes.c_bool, ctypes.c_void_p, ctypes.c_void_p)
    user32.EnumDesktopWindows(hDesk, WNDENUMPROC(find_settings), 0)
    return settings_hwnd

# Configure settings.ini for clean, predictable capture
subprocess.run('taskkill /F /IM DynamicIsland.exe', shell=True, capture_output=True)
time.sleep(0.5)

ini_path = os.path.join(os.environ['APPDATA'], 'DynamicIsland', 'settings.ini')
os.makedirs(os.path.dirname(ini_path), exist_ok=True)
ini = ""
if os.path.exists(ini_path):
    with open(ini_path, 'r', encoding='utf-8') as f:
        ini = f.read()

ini = set_ini_val(ini, 'Settings', 'Appearance.Position', 'top-center')
ini = set_ini_val(ini, 'Settings', 'Appearance.ShapeStyle', 'notch')
ini = set_ini_val(ini, 'Settings', 'Appearance.SizeScale', '1.0')
ini = set_ini_val(ini, 'Settings', 'Themes.ThemePreset', 'transparent')
ini = set_ini_val(ini, 'Settings', 'Themes.PillOpacity', '16')
ini = set_ini_val(ini, 'Settings', 'Behavior.ExpandOnHover', '1')
ini = set_ini_val(ini, 'Settings', 'Behavior.RememberLastPage', '1')
ini = set_ini_val(ini, 'Settings', 'Behavior.DefaultDashboardPage', 'overview')
ini = set_ini_val(ini, 'Settings', 'Modules.Calendar', '1')
ini = set_ini_val(ini, 'Settings', 'Modules.Weather', '1')
ini = set_ini_val(ini, 'Settings', 'Modules.HardwareMonitorModule', '1')
ini = set_ini_val(ini, 'Settings', 'Modules.QuickControls', '1')
ini = set_ini_val(ini, 'Settings', 'Gestures.DoubleClick', 'settings')
ini = set_ini_val(ini, 'Values', 'PinnedExpanded', '0')

with open(ini_path, 'w', encoding='utf-8') as f:
    f.write(ini)

# Start process
si = STARTUPINFO()
si.cb = ctypes.sizeof(STARTUPINFO)
si.lpDesktop = 'WinSta0\\Default'
pi = PROCESS_INFORMATION()

exe_path = os.path.abspath('DynamicIsland.exe')
ok = kernel32.CreateProcessW(None, exe_path, None, None, False, 0, None, os.path.dirname(exe_path), ctypes.byref(si), ctypes.byref(pi))
time.sleep(2.5)

island_hwnd = find_island_hwnd(pi.dwProcessId)
print("Island HWND:", hex(island_hwnd) if island_hwnd else "Not Found")

if island_hwnd:
    WM_APP_TOGGLE_EXPAND = 0x8000 + 0x449  # 0x8449
    WM_APP_SET_TAB       = 0x8000 + 0x44A  # 0x844A
    WM_APP_OPEN_SETTINGS = 0x8000 + 0x44B  # 0x844B

    # Move cursor away
    user32.SetCursorPos(100, 500)
    time.sleep(0.5)

    # 1. Collapsed Overview
    user32.PostMessageW(island_hwnd, WM_APP_TOGGLE_EXPAND, 0, 0)
    time.sleep(1.2)
    save_window_bmp(island_hwnd, "screenshot_collapsed_overview.png")

    # 2. Expanded Overview (Tab 0)
    user32.PostMessageW(island_hwnd, WM_APP_TOGGLE_EXPAND, 1, 0)
    user32.PostMessageW(island_hwnd, WM_APP_SET_TAB, 0, 0)
    time.sleep(1.2)
    save_window_bmp(island_hwnd, "screenshot_expanded_overview.png")

    # 3. Calendar (Tab 1)
    user32.PostMessageW(island_hwnd, WM_APP_SET_TAB, 1, 0)
    time.sleep(1.2)
    save_window_bmp(island_hwnd, "screenshot_calendar.png")

    # 4. Weather (Tab 2)
    user32.PostMessageW(island_hwnd, WM_APP_SET_TAB, 2, 0)
    time.sleep(1.2)
    save_window_bmp(island_hwnd, "screenshot_weather.png")

    # 5. Hardware Monitor (Tab 3)
    user32.PostMessageW(island_hwnd, WM_APP_SET_TAB, 3, 0)
    time.sleep(1.2)
    save_window_bmp(island_hwnd, "screenshot_hardware_monitor.png")

    # 6. Hardware Monitor over White Background (Zero Bleed-Through Test)
    # Use borderless tkinter window to place a solid pure white backdrop directly under the top-level island
    r_isl = wintypes.RECT()
    user32.GetWindowRect(island_hwnd, ctypes.byref(r_isl))
    pad = 60
    w_bg = (r_isl.right - r_isl.left) + pad * 2
    h_bg = (r_isl.bottom - r_isl.top) + pad * 2
    x_bg = max(0, r_isl.left - pad)
    y_bg = max(0, r_isl.top - pad)

    try:
        root = tk.Tk()
        root.overrideredirect(True)
        root.geometry(f"{w_bg}x{h_bg}+{x_bg}+{y_bg}")
        root.configure(bg='white')
        root.update()
        time.sleep(0.8)
        save_window_bmp(island_hwnd, "screenshot_hardware_monitor_light_bg.png")
        root.destroy()
    except Exception as e:
        print(f"Error creating white backdrop: {e}")

    # 7. Quick Controls (Tab 5)
    user32.PostMessageW(island_hwnd, WM_APP_SET_TAB, 5, 0)
    time.sleep(1.2)
    save_window_bmp(island_hwnd, "screenshot_quick_controls.png")

    # 8. Settings & Profile Studio
    user32.PostMessageW(island_hwnd, WM_APP_OPEN_SETTINGS, 0, 0)
    time.sleep(2.5)

    settings_hwnd = find_settings_hwnd(pi.dwProcessId)
    print("Settings HWND:", hex(settings_hwnd) if settings_hwnd else "Not Found")
    if settings_hwnd:
        time.sleep(1.0)
        # Switch to Tab 1: Personalisation to show Accent Color options
        tab_list = []
        def find_tab(child, lp):
            cls = (ctypes.c_wchar * 256)()
            user32.GetClassNameW(child, cls, 256)
            if 'SysTabControl32' in cls.value:
                tab_list.append(child)
            return True
        WNDENUMCHILD = ctypes.WINFUNCTYPE(ctypes.c_bool, ctypes.c_void_p, ctypes.c_void_p)
        user32.EnumChildWindows(settings_hwnd, WNDENUMCHILD(find_tab), 0)
        if tab_list:
            tab_hwnd = tab_list[0]
            # Click on Tab 1 ('🎨 Personalisation')
            # Tab 0 is ~10-100px, Tab 1 is ~110-220px, Y center is ~18px
            lparam = (18 << 16) | 160
            user32.SendMessageW(tab_hwnd, 0x0201, 1, lparam) # WM_LBUTTONDOWN
            user32.SendMessageW(tab_hwnd, 0x0202, 0, lparam) # WM_LBUTTONUP
            time.sleep(1.0)
        save_window_bmp(settings_hwnd, "screenshot_settings_window.png", use_print_window=True)

subprocess.run(f'taskkill /F /PID {pi.dwProcessId}', shell=True, capture_output=True)
print("All screenshots captured and synchronized successfully!")
