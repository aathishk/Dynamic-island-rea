import ctypes
from ctypes import wintypes
import time

user32 = ctypes.windll.user32
dwmapi = ctypes.windll.dwmapi

class AccentPolicy(ctypes.Structure):
    _fields_ = [
        ('accentState', ctypes.c_int),
        ('accentFlags', ctypes.c_int),
        ('gradientColor', wintypes.DWORD),
        ('animationId', wintypes.DWORD),
    ]

class CompositionAttribData(ctypes.Structure):
    _fields_ = [
        ('attrib', ctypes.c_int),
        ('data', ctypes.c_void_p),
        ('size', ctypes.c_size_t),
    ]

pSetWca = user32.SetWindowCompositionAttribute

print('pSetWca callable:', bool(pSetWca))
