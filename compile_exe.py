import subprocess, os, sys

compiler_dir = r"C:\Users\Aathish_Kamaraj\Windhawk\Compiler"
clang = os.path.join(compiler_dir, "bin", "clang++.exe")
src = os.path.abspath("dynamic-island-ultra.cpp")
out_exe = os.path.abspath("DynamicIsland.exe")
shim = os.path.abspath("standalone_shim.h")

cmd = [
    clang,
    "-std=c++23", "-O2",
    "-mwindows", "-municode",
    "-DDYNAMIC_ISLAND_STANDALONE",
    "-DUNICODE", "-D_UNICODE",
    "-DWINVER=0x0A00", "-D_WIN32_WINNT=0x0A00", "-D_WIN32_IE=0x0A00", "-DNTDDI_VERSION=0x0A000008",
    "-D__USE_MINGW_ANSI_STDIO=0",
    '-DWH_MOD_ID=L"dynamic-island-for-windows"',
    '-DWH_MOD_VERSION=L"1.4.0"',
    "-include", shim,
    src,
    "-target", "x86_64-w64-mingw32",
    "-lole32", "-loleaut32", "-lshcore", "-ld2d1", "-ldwrite", "-ldwmapi",
    "-lgdi32", "-luser32", "-lshell32", "-lruntimeobject", "-lwindowscodecs",
    "-lsetupapi", "-lwinhttp", "-lpdh", "-lwinmm",
    "-o", out_exe
]

print("Compiling DynamicIsland.exe...")
p = subprocess.run(cmd, capture_output=True, text=True, cwd=compiler_dir)
print("Exit code:", p.returncode)
if p.returncode != 0:
    print("Stderr:", p.stderr[:3000])
else:
    print("SUCCESS! Output EXE generated:", out_exe)
    print("Size:", os.path.getsize(out_exe), "bytes")
