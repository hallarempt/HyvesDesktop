#!/bin/bash

branch=`pwd | sed "s/\([0-9a-zA-Z ]*\/\)//g"`

if [ "$branch" == "trunk" ]; then
	scp build/distribution/HyvesDesktop-src-*.tar.gz www-data@desktop-builds.dev:desktop-builds
	if [ $? != 0 ]; then exit 1; fi
else
	scp build/distribution/HyvesDesktop-src-*.tar.gz www-data@desktop-builds.dev:desktop-builds/kwekker2/testing/$branch/versions/latest/HyvesDesktop-src.tar.gz
	if [ $? != 0 ]; then exit 1; fi
fi
