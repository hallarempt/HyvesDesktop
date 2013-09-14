"C:\Program Files\Hyves Desktop\bin\HyvesDesktop.exe" --crash

taskkill /F /IM CrashReporter.exe
if errorlevel 1 exit %errorlevel%
