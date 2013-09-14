#!/bin/bash


echo "sleep for 20 seconds to leave some time for Hyves Desktop to be started ..."
sleep 20

# check whether we're running
# matching HyvesDesktop.app OR Hyves Desktop.app
if ps -x | grep -v grep | grep -E "/Applications/Hyves +?Desktop.app/Contents/MacOS/HyvesDesktop "
then
    killall -9 HyvesDesktop
    exit 0

else
echo "Hyves Desktop process has not been found."
echo "Here is the list of running processes"
ps -x | grep -v grep


echo "ls /Library/StartupItems/"
ls /Library/StartupItems/
exit 1
fi

