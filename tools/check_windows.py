import ctypes, subprocess
from ctypes import wintypes

user32 = ctypes.windll.user32
hDesk = user32.OpenDesktopW("Default", 0, False, 0x01FF)
if hDesk:
    user32.SetThreadDesktop(hDesk)

out = subprocess.check_output('tasklist /FI "IMAGENAME eq DynamicIsland.exe" /FO CSV /NH', shell=True, text=True)
target_pids = set()
for line in out.strip().splitlines():
    parts = [p.strip(' "') for p in line.split('","')]
    if len(parts) >= 2 and parts[1].isdigit():
        target_pids.add(int(parts[1]))

print("Target PIDs:", target_pids)

def enum_cb(hwnd, lparam):
    pid = wintypes.DWORD()
    user32.GetWindowThreadProcessId(hwnd, ctypes.byref(pid))
    if pid.value in target_pids:
        buf = (ctypes.c_wchar * 256)()
        user32.GetWindowTextW(hwnd, buf, 256)
        cls = (ctypes.c_wchar * 256)()
        user32.GetClassNameW(hwnd, cls, 256)
        rect = wintypes.RECT()
        user32.GetWindowRect(hwnd, ctypes.byref(rect))
        vis = user32.IsWindowVisible(hwnd)
        w = rect.right - rect.left
        h = rect.bottom - rect.top
        print(f"HWND {hwnd}: class='{cls.value}', title='{buf.value}', visible={vis}, pos=({rect.left}, {rect.top}), size=({w}x{h})")
    return True

WNDENUMPROC = ctypes.WINFUNCTYPE(ctypes.c_bool, wintypes.HWND, wintypes.LPARAM)
user32.EnumWindows(WNDENUMPROC(enum_cb), 0)
