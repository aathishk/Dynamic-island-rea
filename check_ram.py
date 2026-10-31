import subprocess
import ctypes, time
from ctypes import wintypes

class PROCESS_MEMORY_COUNTERS_EX(ctypes.Structure):
    _fields_ = [
        ('cb', wintypes.DWORD),
        ('PageFaultCount', wintypes.DWORD),
        ('PeakWorkingSetSize', ctypes.c_size_t),
        ('WorkingSetSize', ctypes.c_size_t),
        ('QuotaPeakPagedPoolUsage', ctypes.c_size_t),
        ('QuotaPagedPoolUsage', ctypes.c_size_t),
        ('QuotaPeakNonPagedPoolUsage', ctypes.c_size_t),
        ('QuotaNonPagedPoolUsage', ctypes.c_size_t),
        ('PagefileUsage', ctypes.c_size_t),
        ('PeakPagefileUsage', ctypes.c_size_t),
        ('PrivateUsage', ctypes.c_size_t),
    ]

class FILETIME(ctypes.Structure):
    _fields_ = [('dwLowDateTime', wintypes.DWORD), ('dwHighDateTime', wintypes.DWORD)]

def ft_to_uint64(ft):
    return (ft.dwHighDateTime << 32) | ft.dwLowDateTime

res = subprocess.run(['tasklist', '/FI', 'IMAGENAME eq DynamicIsland.exe', '/FO', 'CSV', '/NH'], capture_output=True, text=True)
lines = [l.strip() for l in res.stdout.strip().splitlines() if l.strip()]

for line in lines:
    parts = [p.strip('"') for p in line.split('","')]
    if len(parts) >= 2 and parts[0].lower() == 'dynamicisland.exe':
        pid = int(parts[1])
        hProc = ctypes.windll.kernel32.OpenProcess(0x0410, False, pid)
        if hProc:
            pmc = PROCESS_MEMORY_COUNTERS_EX()
            pmc.cb = ctypes.sizeof(PROCESS_MEMORY_COUNTERS_EX)
            ctypes.windll.psapi.GetProcessMemoryInfo(hProc, ctypes.byref(pmc), ctypes.sizeof(pmc))
            
            # CPU measurement
            c1, e1, k1, u1 = FILETIME(), FILETIME(), FILETIME(), FILETIME()
            c2, e2, k2, u2 = FILETIME(), FILETIME(), FILETIME(), FILETIME()
            ctypes.windll.kernel32.GetProcessTimes(hProc, ctypes.byref(c1), ctypes.byref(e1), ctypes.byref(k1), ctypes.byref(u1))
            t1 = time.time()
            time.sleep(1.0)
            ctypes.windll.kernel32.GetProcessTimes(hProc, ctypes.byref(c2), ctypes.byref(e2), ctypes.byref(k2), ctypes.byref(u2))
            t2 = time.time()
            
            proc_time_diff = (ft_to_uint64(k2) - ft_to_uint64(k1) + ft_to_uint64(u2) - ft_to_uint64(u1)) * 100e-9 # 100ns units to seconds
            num_cpus = ctypes.c_uint()
            # Get system info
            class SYSTEM_INFO(ctypes.Structure):
                _fields_ = [('wProcessorArchitecture', wintypes.WORD), ('wReserved', wintypes.WORD),
                            ('dwPageSize', wintypes.DWORD), ('lpMinimumApplicationAddress', ctypes.c_void_p),
                            ('lpMaximumApplicationAddress', ctypes.c_void_p), ('dwActiveProcessorMask', ctypes.c_size_t),
                            ('dwNumberOfProcessors', wintypes.DWORD), ('dwProcessorType', wintypes.DWORD),
                            ('dwAllocationGranularity', wintypes.DWORD), ('wProcessorLevel', wintypes.WORD),
                            ('wProcessorRevision', wintypes.WORD)]
            si = SYSTEM_INFO()
            ctypes.windll.kernel32.GetSystemInfo(ctypes.byref(si))
            cpu_pct = (proc_time_diff / ((t2 - t1) * si.dwNumberOfProcessors)) * 100.0

            ctypes.windll.kernel32.CloseHandle(hProc)
            print(f"Process: DynamicIsland.exe (PID: {pid})")
            print(f"RAM Active (Working Set): {pmc.WorkingSetSize / (1024*1024):.1f} MB")
            print(f"RAM Dedicated (Private Bytes): {pmc.PrivateUsage / (1024*1024):.1f} MB")
            print(f"Peak RAM: {pmc.PeakWorkingSetSize / (1024*1024):.1f} MB")
            print(f"CPU Usage (Idle): {cpu_pct:.2f}% (across {si.dwNumberOfProcessors} CPU cores)")
