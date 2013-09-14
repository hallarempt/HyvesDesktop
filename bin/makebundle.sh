#!/bin/bash

export QTDIR=`cat kwekker-osx.qtdir`
export PATH=$QTDIR/bin:$PATH
export LD_LIBRARY_PATH=$QTDIR/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$QTDIR/lib/pkgconfig:$PKG_CONFIG_PATH

rm -Rf build
mkdir build
cp -R data/bundle build
find build/bundle -name .svn | xargs -n 1 rm -Rf

cd build

cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
if [ $? != 0 ]; then exit 1; fi

make -j2
if [ $? != 0 ]; then exit 1; fi


python ../bin/symbolstore.py ../bin/mac/dump_syms symbols HyvesDesktop.app

tar -czf symbolstore-osx-r`svn info .. | grep Revision | cut -b11-`.tgz symbols

find HyvesDesktop.app -type d | xargs chmod a+rx
if [ $? != 0 ]; then exit 1; fi
chmod a+x distribution/osx/bundle/bundle.sh
distribution/osx/bundle/bundle.sh
if [ $? != 0 ]; then exit 2; fi
cd ..
