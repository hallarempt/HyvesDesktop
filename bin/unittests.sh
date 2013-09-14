#!/bin/bash

export DISPLAY=:0

export QTDIR=`cat kwekker-ubuntu.qtdir`
export PATH=$QTDIR/bin:$PATH
export LD_LIBRARY_PATH=$QTDIR/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$QTDIR/lib/pkgconfig:$PKG_CONFIG_PATH

cd build
UNITTESTING=1 make test
if [ $? != 0 ]; then exit 2; fi
cd ..
