call "C:\Program Files\Microsoft Visual Studio 9.0\Common7\Tools\vsvars32.bat"

cd build
set PATH=%CD%;%PATH%
set UNITTESTING=1
nmake test
if errorlevel 1 exit %errorlevel%
cd ..
