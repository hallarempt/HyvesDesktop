call "C:\Program Files\Microsoft Visual Studio 9.0\Common7\Tools\vsvars32.bat"

if exist debugbuild rm -Rf debugbuild
mkdir debugbuild

cd debugbuild

cmake .. -G "NMake Makefiles" -DZLIB_INCLUDE_DIR=C:\ZLib\include -DZLIB_LIBRARY=C:\ZLib\lib\zdll.lib -DCMAKE_PREFIX_PATH=C:\OpenSSL;C:\Qt\4.5.2 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=install
if errorlevel 1 exit %errorlevel%

nmake
if errorlevel 1 exit %errorlevel%

c:\Python25\python.exe ..\bin\symbolstore.py ..\bin\windows\dump_syms.exe symbols HyvesDesktop.exe HyvesDesktopLib.dll plugins\kwekker\bin\Kwekker.dll plugins\photouploader\bin\PhotoUploader.dll
if errorlevel 1 exit %errorlevel%

pscp -r -i -batch C:\WORK\.ssh\id_rsa.ppk "symbols\*.*" USERNAME@symbols.hyves.org:~/symbols/
if errorlevel 1 exit %errorlevel%


