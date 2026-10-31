import subprocess

res = subprocess.run(['powershell', '-NoProfile', '-Command', 'Get-StartApps'], capture_output=True, text=True)
for line in res.stdout.splitlines():
    lower = line.lower()
    if any(k in lower for k in ['phone', 'link', 'mobile', 'device', 'connect', 'bluetooth']):
        print(line)
