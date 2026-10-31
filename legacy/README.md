# legacy/

Historical scripts. `build_ultra.py` (with `patch_position.py` and `update_script.py`, which patch the patcher) transformed upstream's `dynamic-island-for-windows.wh.cpp` into `dynamic-island-ultra.cpp` using plain string replacement. The checked-in `.cpp` in the repository root is now the source of truth; nothing here is needed to build.

These scripts expect an upstream checkout in `devcode90-island/` and hard-coded Windhawk paths; they are kept only as a record of what was changed relative to upstream.
