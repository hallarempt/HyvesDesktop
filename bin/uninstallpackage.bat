"C:\Program Files\Hyves Desktop\uninstall.exe" --mode unattended
if errorlevel 1 exit %errorlevel%

ping 127.0.0.1 -n 20 -w 1000 > nul

if exist "C:\Program Files\Hyves Desktop\bin" exit 1
if exist "C:\Program Files\Hyves Desktop\plugins" exit 2

ver | findstr /i "5\.1\." > nul
if %errorlevel% equ 0 goto winxp
goto vista

:winxp
if exist "C:\Documents and Settings\All Users\Desktop\Hyves Desktop Chat.lnk" exit 3
if exist "C:\Documents and Settings\All Users\Desktop\Hyves Desktop Photo Uploader.lnk" exit 4
goto end

:vista
if exist "C:\Users\Public\Desktop\Hyves Desktop Chat.lnk" exit 3
if exist "C:\Users\Public\Desktop\Hyves Desktop Photo Uploader.lnk" exit 4
exit 0
:end
