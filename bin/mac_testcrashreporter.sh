#!/bin/bash

# check whether the crash reporter is started

if test -e /Applications/Hyves\ Desktop.app
then
        /Applications/Hyves\ Desktop.app/Contents/MacOS/HyvesDesktop --crash
elif test -e /Applications/HyvesDesktop.app
then
        /Applications/HyvesDesktop.app/Contents/MacOS/HyvesDesktop --crash
fi

if ps -x | grep -v grep | grep CrashReporter
then
    killall -9 HyvesDesktop
    killall -9 CrashReporter
    exit 0
fi

exit 1
