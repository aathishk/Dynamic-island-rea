import subprocess, os, sys

compiler_dir = r"C:\Users\Aathish_Kamaraj\Windhawk\Compiler"
clang = os.path.join(compiler_dir, "bin", "clang++.exe")
lib = r"C:\Users\Aathish_Kamaraj\Windhawk\Engine\1.7.3\64\windhawk.lib"
src = os.path.abspath(r"devcode90-island\dynamic-island-for-windows.wh.cpp")
out_dll = os.path.abspath(r"test_island.dll")

cmd = [
    clang,
    "-std=c++23", "-O2", "-shared",
    "-DUNICODE", "-D_UNICODE",
    "-DWINVER=0x0A00", "-D_WIN32_WINNT=0x0A00", "-D_WIN32_IE=0x0A00", "-DNTDDI_VERSION=0x0A000008",
    "-D__USE_MINGW_ANSI_STDIO=0",
    "-DWH_MOD",
    '-DWH_MOD_ID=L"dynamic-island-for-windows"',
    '-DWH_MOD_VERSION=L"1.4.0"',
    lib,
    src,
    "-include", "windhawk_api.h",
    "-target", "x86_64-w64-mingw32",
    "-Wl,--export-all-symbols",
    "-lole32", "-loleaut32", "-lshcore", "-ld2d1", "-ldwrite", "-ldwmapi",
    "-lgdi32", "-luser32", "-lshell32", "-lruntimeobject", "-lwindowscodecs",
    "-lsetupapi", "-lwinhttp", "-lpdh", "-lwinmm",
    "-o", out_dll
]

print("Compiling devcode90...")
p = subprocess.run(cmd, capture_output=True, text=True, cwd=compiler_dir)
print("Exit code:", p.returncode)
if p.returncode != 0:
    print("Stderr:", p.stderr[:2000])
else:
    print("Success! Output size:", os.path.getsize(out_dll))
