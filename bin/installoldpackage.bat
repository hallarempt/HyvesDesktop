taskkill /F /IM HyvesDesktop.exe /IM CrashReporter.exe

set OLDVERSION=1.2.4546

wget -O "C:\WORK\Hyves-Desktop-setup-old.exe" http://desktop-builds.dev/kwekker2/testing/release/1.2/Hyves-Desktop-setup-1.2.exe
if errorlevel 1 exit %errorlevel%

"C:\WORK\Hyves-Desktop-setup-old.exe" --mode unattended
if errorlevel 1 exit %errorlevel%

echo Hyves Desktop version %OLDVERSION% should be installed now...
"C:\Program Files\Hyves Desktop\bin\HyvesDesktop.exe" --version | more

"C:\Program Files\Hyves Desktop\bin\HyvesDesktop.exe" --version | findstr /i %OLDVERSION% > nul
if not %errorlevel% equ 0 exit 2
