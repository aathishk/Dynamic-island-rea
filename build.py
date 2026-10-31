#!/usr/bin/env python3
"""Build DynamicIsland.exe (standalone) with an LLVM-MinGW clang toolchain.

Usage:
    python build.py [--toolchain DIR] [--cppwinrt DIR] [--out FILE] [--opt O2]

--toolchain  Directory containing bin/clang++ (an llvm-mingw release, or the
             "Compiler" folder of a Windhawk install). Defaults to $DI_TOOLCHAIN,
             then whatever clang++ is on PATH.
--cppwinrt   Directory that contains the C++/WinRT "winrt" headers. Not needed
             with Windhawk's compiler (it bundles them). Defaults to
             $DI_CPPWINRT, then the newest Windows SDK found on this machine.
--out        Output exe (default: ./DynamicIsland.exe).
"""
import argparse
import glob
import os
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
VERSION = "1.5.0"


def find_clang(toolchain):
    names = ["x86_64-w64-mingw32-clang++", "clang++"]
    if toolchain:
        for n in names:
            for ext in ("", ".exe"):
                p = os.path.join(toolchain, "bin", n + ext)
                if os.path.exists(p):
                    return p
        sys.exit(f"error: no clang++ found in {toolchain}\\bin")
    for n in names:
        p = shutil.which(n)
        if p:
            return p
    sys.exit("error: clang++ not found. Pass --toolchain or set DI_TOOLCHAIN.")


def find_cppwinrt(explicit, toolchain):
    if explicit:
        return explicit
    # Windhawk's compiler ships the winrt headers next to the mingw headers.
    if toolchain and glob.glob(os.path.join(toolchain, "**", "winrt", "base.h"), recursive=True):
        return None
    kits = os.path.join(os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)"),
                        "Windows Kits", "10", "Include")
    found = sorted(glob.glob(os.path.join(kits, "*", "cppwinrt")))
    return found[-1] if found else None


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--toolchain", default=os.environ.get("DI_TOOLCHAIN"))
    ap.add_argument("--cppwinrt", default=os.environ.get("DI_CPPWINRT"))
    ap.add_argument("--out", default=os.path.join(HERE, "DynamicIsland.exe"))
    ap.add_argument("--opt", default="O2")
    args = ap.parse_args()

    clang = find_clang(args.toolchain)
    cppwinrt = find_cppwinrt(args.cppwinrt, args.toolchain)

    cmd = [
        clang, "-std=c++23", "-" + args.opt,
        "-mwindows", "-municode",
        "-DDYNAMIC_ISLAND_STANDALONE", "-DUNICODE", "-D_UNICODE",
        "-DWINVER=0x0A00", "-D_WIN32_WINNT=0x0A00", "-D_WIN32_IE=0x0A00", "-DNTDDI_VERSION=0x0A000008",
        "-D__USE_MINGW_ANSI_STDIO=0",
        '-DWH_MOD_ID=L"dynamic-island-for-windows"',
        f'-DWH_MOD_VERSION=L"{VERSION}"',
        "-I", HERE,
        "-include", os.path.join(HERE, "standalone_shim.h"),
    ]
    if cppwinrt:
        cmd += ["-I", cppwinrt]
    cmd += [
        os.path.join(HERE, "dynamic-island-ultra.cpp"),
        "-target", "x86_64-w64-mingw32",
        "-lole32", "-loleaut32", "-lshcore", "-ld2d1", "-ldwrite", "-ldwmapi",
        "-lgdi32", "-luser32", "-lshell32", "-lruntimeobject", "-lwindowscodecs",
        "-lsetupapi", "-lwinhttp", "-lpdh", "-lwinmm", "-ladvapi32",
        "-o", args.out,
    ]
    print("Compiler :", clang)
    print("C++/WinRT:", cppwinrt or "(bundled with toolchain)")
    print("Building ", args.out, "...")
    p = subprocess.run(cmd, capture_output=True, text=True)
    if p.returncode != 0:
        print(p.stderr[-6000:])
        sys.exit(p.returncode)
    print(f"OK - {os.path.getsize(args.out) / 1024:.0f} KiB")


if __name__ == "__main__":
    main()
