call "C:\Program Files\Microsoft Visual Studio 9.0\Common7\Tools\vsvars32.bat"

del /Q "C:\Program Files\BitRock InstallBuilder for Qt\output\*"

if exist build rm -Rf build
mkdir build
cp -R data\bundle build
find build\bundle -name .svn | xargs -n 1 rm -Rf

cd build
cmake .. -G "NMake Makefiles" -DZLIB_INCLUDE_DIR=C:\ZLib\include -DZLIB_LIBRARY=C:\ZLib\lib\zdll.lib -DCMAKE_PREFIX_PATH=C:\OpenSSL;C:\Qt\4.5.2 -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=install
if errorlevel 1 exit %errorlevel%

nmake install
if errorlevel 1 exit %errorlevel%

c:\Python25\python.exe ..\bin\symbolstore.py ..\bin\windows\dump_syms.exe symbols HyvesDesktop.exe HyvesDesktopLib.dll plugins\kwekker\bin\Kwekker.dll plugins\photouploader\bin\PhotoUploader.dll
if errorlevel 1 exit %errorlevel%

bash ..\bin\makesymbolstore.sh

cd ..

builder build distribution\bitrock\hyvesdesktop-win32.xml
if errorlevel 1 exit %errorlevel%

if not exist "C:\Program Files\BitRock InstallBuilder for Qt\output\Hyves-Desktop-setup-*.exe" exit 1
copy "C:\Program Files\BitRock InstallBuilder for Qt\output\Hyves-Desktop-setup-*.exe" build
