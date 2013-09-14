#!/bin/bash

export QTDIR=`cat kwekker-ubuntu.qtdir`
export PATH=$QTDIR/bin:$PATH
export LD_LIBRARY_PATH=$QTDIR/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$QTDIR/lib/pkgconfig:$PKG_CONFIG_PATH

mkdir build
rm -Rf build/*
cp -R data/bundle build
find build/bundle -name .svn | xargs -n 1 rm -Rf

cd build
cmake .. -DCMAKE_INSTALL_PREFIX=install
if [ $? != 0 ]; then exit 1; fi
make package-source-tgz
if [ $? != 0 ]; then exit 2; fi
cd ..
