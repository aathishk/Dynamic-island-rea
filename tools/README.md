# tools/

Developer helpers (Windows only, Python 3):

- `check_ram.py` – prints working set / private bytes / idle CPU of a running `DynamicIsland.exe`.
- `check_windows.py`, `inspect_pid.py <pid>` – list the top-level windows of the process.
- `run_island.py [path\to\DynamicIsland.exe]` – start the exe on the interactive desktop with debug logging on and print its windows and log.
- `find_apps.py`, `test_accent.py` – small probes used during development.
