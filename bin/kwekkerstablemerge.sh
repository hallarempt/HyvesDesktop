#!/bin/bash

let rev1=$1-1
let rev2=$1

pushd $STABLE_DIR
svn up
svn merge --ignore-ancestry -r $rev1:$rev2 https://trac.startpda.net/svn/desktop/trunk .
svn commit -m "- Merge r$rev2 to stable."
popd
