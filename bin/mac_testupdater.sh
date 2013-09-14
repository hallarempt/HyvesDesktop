#!/bin/bash

killall -9 HyvesDesktop

OLDVERSION="1.2.4546"
HOSTFILE="/etc/hosts"
UPDATE_EXITCODE="57"


# locate Hyves Desktop installation
if [ -d "/Applications/Hyves Desktop.app" ]
then
	hdapp="/Applications/Hyves Desktop.app"
elif [ -d "/Applications/HyvesDesktop.app" ]
then
	hdapp="/Applications/HyvesDesktop.app"
else
	echo "Could not find Hyves Desktop installation"
	echo "TEST FAILED!"
	exit 1
fi

# check whether the correct version is installed
hdexec="${hdapp}/Contents/MacOS/HyvesDesktop"
hdversion=$("$hdexec" --version-number)
if [ -z "${hdversion}" ]
then
	echo "Found Hyves Desktop installed to ${hdapp} but failed to determine version number"
	echo "You probably have an very old version of Hyves Desktop installed that does not know about the '--version-number' switch yet"
	echo "TEST FAILED!"
	exit 1
fi

if [ "${hdversion}" != "${OLDVERSION}" ]
then
	echo "Found Hyves Desktop ${hdversion} but expected version ${OLDVERSION}"
	echo "TEST FAILED!"
	exit 2
fi

echo "Found Hyves Desktop ${hdversion} installed to ${hdapp}"


# run update
sudo cp ${HOSTFILE} ~/hosts.bak
sudo echo "172.31.0.51 download.hyves.org" >> $HOSTFILE
if [ $? -ne 0 ]
then
	echo "Failed to modify ${HOSTFILE}"
	echo "TEST FAILED!"
	exit 1
fi

"${hdexec}" --silent-update
exitcode=$?
sudo cp ~/hosts.bak $HOSTFILE || echo "Could not restore ${HOSTFILE} - you better have a look at it, sorry :("


# after restarting it somehow doesn't belong to us anymore? <-- needs to be reparented to init otherwise it would die with the updater-process?
# Anyways, we don't know the pid of the new process so...
# matching HyvesDesktop.app OR Hyves Desktop.app
if ps -x | grep -v grep | grep -E "/Applications/Hyves +?Desktop.app/Contents/MacOS/HyvesDesktop "
then 
    sudo killall -9 HyvesDesktop
    if [ $? -ne 0 ]
    then
        echo "Could not stop running HyvesDesktop"
	echo "TEST FAILED!"
        exit 3
    fi
fi

if [ $exitcode -ne ${UPDATE_EXITCODE} ]
then
	echo "HyvesDesktop exited with ${exitcode} but should have exited with ${UPDATE_EXITCODE}"
	echo "TEST FAILED!"
	exit 4
fi


# locate new version
if [ -d "/Applications/Hyves Desktop.app" ]
then
	newapp="/Applications/Hyves Desktop.app"
elif [ -d "/Applications/HyvesDesktop.app" ]
then
	newapp="/Applications/HyvesDesktop.app"
else
	echo "Can not find Hyves Desktop anymore."
	echo "TEST FAILED!"
	exit 5
fi

newexec="${newapp}/Contents/MacOS/HyvesDesktop"
newversion=$("$newexec" --version-number)
if [ -z "${newversion}" ]
then
	echo "Failed to determine HyvesDesktop version number"
	echo "We probably downgraded to an old version of Hyves Desktop that does not know about the '--version-number' switch yet"
	echo "TEST FAILED!"
	exit 6
fi

if [ ! "${newversion}" \> "${hdversion}" ]
then
	echo "Found Hyves Desktop ${newversion} but expected version greater than ${hdversion}"
	echo "TEST FAILED!"
	exit 6
fi

echo "Found Hyves Desktop ${newversion}"
echo "Test succeeded."
exit 0
