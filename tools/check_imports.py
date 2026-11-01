#!/usr/bin/env python3
"""Fail if a release exe depends on DLLs that a clean Windows install lacks.

usage: python tools/check_imports.py DynamicIsland.exe   (needs: pip install pefile)
"""
import sys
import pefile

ALLOWED = {
    "kernel32.dll", "user32.dll", "gdi32.dll", "advapi32.dll", "shell32.dll", "ole32.dll",
    "oleaut32.dll", "shcore.dll", "d2d1.dll", "dwrite.dll", "dwmapi.dll", "setupapi.dll",
    "winhttp.dll", "pdh.dll", "winmm.dll", "comctl32.dll", "uxtheme.dll", "msvcrt.dll", "ucrtbase.dll",
}
pe = pefile.PE(sys.argv[1])
bad = sorted({e.dll.decode(errors="replace") for e in pe.DIRECTORY_ENTRY_IMPORT
              if not e.dll.decode(errors="replace").lower().startswith("api-ms")
              and e.dll.decode(errors="replace").lower() not in ALLOWED})
has_manifest = any(r.id == 24 for r in getattr(pe, "DIRECTORY_ENTRY_RESOURCE", None).entries) \
    if hasattr(pe, "DIRECTORY_ENTRY_RESOURCE") else False
if bad:
    sys.exit(f"FAIL: depends on DLLs not shipped with Windows: {bad}")
if not has_manifest:
    sys.exit("FAIL: no embedded manifest")
print("OK: only system DLLs imported, manifest present")
