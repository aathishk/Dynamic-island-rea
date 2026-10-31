import os, subprocess

startup = os.path.expandvars(r"%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup")
lnk_path = os.path.join(startup, "DynamicIsland.lnk")
target_exe = r"C:\Users\Aathish_Kamaraj\Downloads\rea\DynamicIsland.exe"
work_dir = r"C:\Users\Aathish_Kamaraj\Downloads\rea"

vbs_code = f'''Set oWS = WScript.CreateObject("WScript.Shell")
sLinkFile = "{lnk_path}"
Set oLink = oWS.CreateShortcut(sLinkFile)
oLink.TargetPath = "{target_exe}"
oLink.WorkingDirectory = "{work_dir}"
oLink.Description = "Dynamic Island for Windows 11 Ultra"
oLink.Save
'''

vbs_path = "create_shortcut.vbs"
with open(vbs_path, "w", encoding="utf-8") as f:
    f.write(vbs_code)

res = subprocess.run(["cscript", "//nologo", vbs_path], capture_output=True, text=True)
print("CScript output:", res.stdout, res.stderr)
print("Shortcut exists:", os.path.exists(lnk_path))
if os.path.exists(vbs_path):
    os.remove(vbs_path)
