"C:\Program Files\Hyves Desktop\bin\HyvesDesktop.exe" --version | more

"C:\Program Files\Hyves Desktop\bin\HyvesDesktop.exe" --test 2>&1
if errorlevel 1 exit %errorlevel%

"C:\Program Files\Hyves Desktop\bin\HyvesDesktop.exe" --plugin photouploader --test 2>&1
if errorlevel 1 exit %errorlevel%

"C:\WORK\HyvesDesktop\trunk\bin\teststatsupload.py"
if errorlevel 1 exit %errorlevel%
