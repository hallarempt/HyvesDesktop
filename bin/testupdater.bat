set OLDVERSION=1.2.4546
set HOSTSFILE=C:\WINDOWS\system32\drivers\etc\hosts

copy "%HOSTSFILE%" "%HOSTSFILE%.bak"
echo 172.31.0.51 download.hyves.org >> "%HOSTSFILE%"

echo Currently installed version should be %OLDVERSION%
"C:\Program Files\Hyves Desktop\bin\HyvesDesktop.exe" --version | more

"C:\Program Files\Hyves Desktop\bin\HyvesDesktop.exe" --version | findstr /i %OLDVERSION% > nul
if not %errorlevel% equ 0 exit 2

"C:\Program Files\Hyves Desktop\bin\HyvesDesktop.exe" --silent-update
set EXITCODE=%errorlevel%

move /Y "%HOSTSFILE%.bak" "%HOSTSFILE%"

if %EXITCODE% equ 1337 set EXITCODE=57
if not %EXITCODE% equ 57 exit 1

ping 127.0.0.1 -n 60 -w 1000 > nul

echo Updated to version should differ from %OLDVERSION%:
"C:\Program Files\Hyves Desktop\bin\HyvesDesktop.exe" --version | more

"C:\Program Files\Hyves Desktop\bin\HyvesDesktop.exe" --version | findstr /i %OLDVERSION% > nul
if %errorlevel% equ 0 exit 2
