import ctypes
from ctypes import wintypes
import time, os, sys

user32 = ctypes.windll.user32
kernel32 = ctypes.windll.kernel32

class STARTUPINFO(ctypes.Structure):
    _fields_ = [
        ('cb', wintypes.DWORD),
        ('lpReserved', wintypes.LPWSTR),
        ('lpDesktop', wintypes.LPWSTR),
        ('lpTitle', wintypes.LPWSTR),
        ('dwX', wintypes.DWORD),
        ('dwY', wintypes.DWORD),
        ('dwXSize', wintypes.DWORD),
        ('dwYSize', wintypes.DWORD),
        ('dwXCountChars', wintypes.DWORD),
        ('dwYCountChars', wintypes.DWORD),
        ('dwFillAttribute', wintypes.DWORD),
        ('dwFlags', wintypes.DWORD),
        ('wShowWindow', wintypes.WORD),
        ('cbReserved2', wintypes.WORD),
        ('lpReserved2', ctypes.c_char_p),
        ('hStdInput', wintypes.HANDLE),
        ('hStdOutput', wintypes.HANDLE),
        ('hStdError', wintypes.HANDLE),
    ]

class PROCESS_INFORMATION(ctypes.Structure):
    _fields_ = [
        ('hProcess', wintypes.HANDLE),
        ('hThread', wintypes.HANDLE),
        ('dwProcessId', wintypes.DWORD),
        ('dwThreadId', wintypes.DWORD),
    ]

# Logging is opt-in; the standalone app writes %APPDATA%\\DynamicIsland\\debug.log
os.environ['DYNAMIC_ISLAND_DEBUG'] = '1'
log_path = os.path.join(os.environ.get('APPDATA', '.'), 'DynamicIsland', 'debug.log')
if os.path.exists(log_path):
    try:
        os.remove(log_path)
    except Exception:
        pass

si = STARTUPINFO()
si.cb = ctypes.sizeof(STARTUPINFO)
si.lpDesktop = 'WinSta0\\Default'

pi = PROCESS_INFORMATION()

exe_path = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else 'DynamicIsland.exe')
work_dir = os.path.dirname(exe_path)

success = kernel32.CreateProcessW(
    None, exe_path, None, None, False, 0, None, work_dir,
    ctypes.byref(si), ctypes.byref(pi)
)

print('Spawned DynamicIsland.exe on WinSta0\\\\Default, success:', success, 'PID:', pi.dwProcessId)

time.sleep(2.5)

hDesk = user32.OpenDesktopW('Default', 0, False, 0x01FF)
found = []
def enum_cb(hwnd, lparam):
    pid = wintypes.DWORD()
    user32.GetWindowThreadProcessId(hwnd, ctypes.byref(pid))
    if pid.value == pi.dwProcessId:
        buf = (ctypes.c_wchar * 512)()
        user32.GetWindowTextW(hwnd, buf, 512)
        cls = (ctypes.c_wchar * 512)()
        user32.GetClassNameW(hwnd, cls, 512)
        rect = wintypes.RECT()
        user32.GetWindowRect(hwnd, ctypes.byref(rect))
        vis = user32.IsWindowVisible(hwnd)
        found.append({
            'hwnd': hex(hwnd),
            'title': buf.value,
            'class': cls.value,
            'visible': bool(vis),
            'x': rect.left,
            'y': rect.top,
            'width': rect.right - rect.left,
            'height': rect.bottom - rect.top
        })
    return True

WNDENUMPROC = ctypes.WINFUNCTYPE(ctypes.c_bool, ctypes.c_void_p, ctypes.c_void_p)
user32.EnumDesktopWindows(hDesk, WNDENUMPROC(enum_cb), 0)
user32.CloseDesktop(hDesk)

print(f'Windows for PID {pi.dwProcessId}:')
for win in found:
    print(win)

if os.path.exists(log_path):
    print('\n--- debug.log ---')
    with open(log_path, 'r', encoding='utf-8', errors='ignore') as f:
        print(f.read())
