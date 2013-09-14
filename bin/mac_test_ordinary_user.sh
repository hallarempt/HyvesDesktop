#!/bin/bash

# start as an extremely disenfranchised user
if test -e /Applications/Hyves\ Desktop.app
then
    if ls -l /Applications/Hyves\ Desktop.app/Contents/MacOS/HyvesDesktop | grep rwxr-xr-x
    then
         exit 0
    fi
elif test -e /Applications/HyvesDesktop.app
then
    if ls -l /Applications/HyvesDesktop.app/Contents/MacOS/HyvesDesktop | grep rwxr-xr-x
    then
         exit 0
    fi
fi

echo "HyvesDesktop is not world executable"
exit 1
