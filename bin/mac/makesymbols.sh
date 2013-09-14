#!/bin/bash

export QTDIR=`cat kwekker-osx.qtdir`
export PATH=$QTDIR/bin:$PATH
export LD_LIBRARY_PATH=$QTDIR/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$QTDIR/lib/pkgconfig:$PKG_CONFIG_PATH

mkdir debugbuild
rm -Rf debugbuild/*

cd debugbuild
cmake .. -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=Debug
make

# dump everything to the symbols dir
python ../bin/symbolstore.py ../bin/mac/dump_syms symbols HyvesDesktop.app

# upload the whole symbols dir to the symbols server
#scp -r symbols/* XXX@symbols.hyves.org:~/symbols

