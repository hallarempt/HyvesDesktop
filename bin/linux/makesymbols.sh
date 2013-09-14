#!/bin/bash

export QTDIR=`cat kwekker-ubuntu.qtdir`
export PATH=$QTDIR/bin:$PATH
export LD_LIBRARY_PATH=$QTDIR/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$QTDIR/lib/pkgconfig:$PKG_CONFIG_PATH

mkdir debugbuild
rm -Rf debugbuild/*

cd debugbuild
cmake .. -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS=-gstabs
make

python ../bin/symbolstore.py ../bin/linux/dump_syms symbols HyvesDesktop

scp -r symbols/* XXX@symbols.hyves.org:~/symbols
