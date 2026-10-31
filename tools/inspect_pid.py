import ctypes
import sys
from ctypes import wintypes

if len(sys.argv) != 2:
    sys.exit("usage: python inspect_pid.py <pid>")
TARGET_PID = int(sys.argv[1])

user32 = ctypes.windll.user32
hDesk = user32.OpenInputDesktop(0, False, 0x01FF)
user32.SetThreadDesktop(hDesk)

found = []
def enum_cb(hwnd, lparam):
    pid = wintypes.DWORD()
    user32.GetWindowThreadProcessId(hwnd, ctypes.byref(pid))
    if pid.value == TARGET_PID:
        buf = (ctypes.c_wchar * 512)()
        user32.GetWindowTextW(hwnd, buf, 512)
        cls = (ctypes.c_wchar * 512)()
        user32.GetClassNameW(hwnd, cls, 512)
        rect = wintypes.RECT()
        user32.GetWindowRect(hwnd, ctypes.byref(rect))
        vis = user32.IsWindowVisible(hwnd)
        found.append((hwnd, buf.value, cls.value, vis, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top))
    return True

WNDENUMPROC = ctypes.WINFUNCTYPE(ctypes.c_bool, ctypes.c_void_p, ctypes.c_void_p)
user32.EnumDesktopWindows(hDesk, WNDENUMPROC(enum_cb), 0)

print(f"Total windows for PID {TARGET_PID}: {len(found)}")
for item in found:
    print(item)
