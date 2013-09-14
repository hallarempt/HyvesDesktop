#!/bin/bash

rm /home/administrator/InstallBuilder/output/*

export QTDIR=`cat kwekker-ubuntu.qtdir`
export PATH=$QTDIR/bin:$PATH
export LD_LIBRARY_PATH=$QTDIR/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$QTDIR/lib/pkgconfig:$PKG_CONFIG_PATH

mkdir build
rm -Rf build/*
cp -R data/bundle build
find build/bundle -name .svn | xargs -n 1 rm -Rf

cd build
cmake .. -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS_RELWITHDEBINFO=-gstabs
if [ $? != 0 ]; then exit 1; fi

make install
if [ $? != 0 ]; then exit 2; fi

python ../bin/symbolstore.py ../bin/linux/dump_syms symbols HyvesDesktop libHyvesDesktopLib.so plugins/kwekker/bin/libKwekker.so plugins/photouploader/bin/libPhotoUploader.so

tar -czf symbolstore-linux-r`svn info .. | grep Revision | cut -b11-`.tgz symbols

cd ..

/home/administrator/InstallBuilder/bin/builder build distribution/bitrock/hyvesdesktop.xml
if [ $? != 0 ]; then exit 3; fi

cp /home/administrator/InstallBuilder/output/Hyves-Desktop-setup-*.bin build
if [ $? != 0 ]; then exit 4; fi
