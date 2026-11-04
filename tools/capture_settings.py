import ctypes
import struct
import os
import time
import subprocess
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

subprocess.run('taskkill /F /IM DynamicIsland.exe', shell=True, capture_output=True)
time.sleep(0.5)

si = STARTUPINFO()
si.cb = ctypes.sizeof(STARTUPINFO)
si.lpDesktop = 'WinSta0\\Default'
pi = PROCESS_INFORMATION()

exe_path = os.path.abspath('DynamicIsland.exe')
ok = kernel32.CreateProcessW(None, exe_path, None, None, False, 0, None, os.path.dirname(exe_path), ctypes.byref(si), ctypes.byref(pi))
time.sleep(2.5)

hDesk = user32.OpenDesktopW('Default', 0, False, 0x01FF)
if hDesk:
    user32.SetThreadDesktop(hDesk)

island_hwnd = None
def enum_cb(hwnd, lparam):
    global island_hwnd
    proc_id = wintypes.DWORD()
    user32.GetWindowThreadProcessId(hwnd, ctypes.byref(proc_id))
    if proc_id.value == pi.dwProcessId:
        cls = (ctypes.c_wchar * 256)()
        user32.GetClassNameW(hwnd, cls, 256)
        if 'DynamicIslandForWindows' in cls.value:
            island_hwnd = hwnd
    return True

WNDENUMPROC = ctypes.WINFUNCTYPE(ctypes.c_bool, ctypes.c_void_p, ctypes.c_void_p)
user32.EnumDesktopWindows(hDesk, WNDENUMPROC(enum_cb), 0)

if island_hwnd:
    WM_LBUTTONDBLCLK = 0x0203
    user32.PostMessageW(island_hwnd, WM_LBUTTONDBLCLK, 0, 0)
    time.sleep(2.0)

    settings_hwnd = None
    def find_settings(hwnd, lparam):
        global settings_hwnd
        proc_id = wintypes.DWORD()
        user32.GetWindowThreadProcessId(hwnd, ctypes.byref(proc_id))
        if proc_id.value == pi.dwProcessId:
            cls = (ctypes.c_wchar * 256)()
            user32.GetClassNameW(hwnd, cls, 256)
            buf = (ctypes.c_wchar * 256)()
            user32.GetWindowTextW(hwnd, buf, 256)
            if 'Settings' in cls.value or 'Settings' in buf.value:
                settings_hwnd = hwnd
        return True

    user32.EnumDesktopWindows(hDesk, WNDENUMPROC(find_settings), 0)

    if settings_hwnd:
        r = wintypes.RECT()
        user32.GetWindowRect(settings_hwnd, ctypes.byref(r))
        w, h = r.right - r.left, r.bottom - r.top
        print(f'Capturing Settings window: {w}x{h}')

        hdcScreen = user32.GetDC(0)
        hdcMem = gdi32.CreateCompatibleDC(hdcScreen)
        hbm = gdi32.CreateCompatibleBitmap(hdcScreen, w, h)
        hbmOld = gdi32.SelectObject(hdcMem, hbm)

        user32.PrintWindow(settings_hwnd, hdcMem, 2)

        bmi = bytearray(40)
        struct.pack_into('<IIIHHIIIIII', bmi, 0, 40, w, h, 1, 32, 0, w * h * 4, 0, 0, 0, 0)
        buf = bytearray(w * h * 4)
        gdi32.GetDIBits(hdcMem, hbm, 0, h, (ctypes.c_char * len(buf)).from_buffer(buf), (ctypes.c_char * 40).from_buffer(bmi), 0)

        gdi32.SelectObject(hdcMem, hbmOld)
        gdi32.DeleteObject(hbm)
        gdi32.DeleteDC(hdcMem)
        user32.ReleaseDC(0, hdcScreen)

        bmp_header = struct.pack('<2sIHHI', b'BM', 14 + 40 + len(buf), 0, 0, 14 + 40)
        with open('screenshot_settings_window.bmp', 'wb') as f:
            f.write(bmp_header)
            f.write(bmi)
            f.write(buf)

        cmd = 'Add-Type -AssemblyName System.Drawing; $img = [System.Drawing.Image]::FromFile("screenshot_settings_window.bmp"); $img.Save("screenshot_settings_window.png", [System.Drawing.Imaging.ImageFormat]::Png); $img.Dispose(); Remove-Item "screenshot_settings_window.bmp"'
        subprocess.run(['powershell', '-Command', cmd], capture_output=True)
        print('Saved screenshot_settings_window.png successfully!')

subprocess.run(f'taskkill /F /PID {pi.dwProcessId}', shell=True, capture_output=True)
