taskkill /F /IM HyvesDesktop.exe /IM CrashReporter.exe

wget -O "C:\WORK\Hyves-Desktop-setup.exe" http://desktop-builds.dev/kwekker2/testing/release/1.1/Hyves-Desktop-setup-1.1.exe
if errorlevel 1 exit %errorlevel%

"C:\WORK\Hyves-Desktop-setup.exe" --mode unattended
if errorlevel 1 exit %errorlevel%

if not exist "C:\Program Files\Hyves Desktop\bin" exit 1
if not exist "C:\Program Files\Hyves Desktop\plugins" exit 2
if not exist "C:\Program Files\Hyves Desktop\uninstall.exe" exit 3

ver | findstr /i "5\.1\." > nul
if %errorlevel% equ 0 goto winxp
goto vista

:winxp
if not exist "C:\Documents and Settings\All Users\Desktop\Hyves Desktop Chat.lnk" exit 4
if not exist "C:\Documents and Settings\All Users\Desktop\Hyves Desktop Photo Uploader.lnk" exit 5
goto end

:vista
if not exist "C:\Users\Public\Desktop\Hyves Desktop Chat.lnk" exit 4
if not exist "C:\Users\Public\Desktop\Hyves Desktop Photo Uploader.lnk" exit 5

:end
