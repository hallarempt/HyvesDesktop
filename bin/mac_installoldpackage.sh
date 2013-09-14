#!/bin/bash

OLDVERSION="1.2.4546"
UPDATEURL="http://desktop-builds.dev/kwekker2/testing/release/1.2/Hyves-Desktop-setup-1.2.dmg"

# clean up any left overs from previous tests
[ -f ~/Hyves-Desktop-setup-old.dmg ] && echo "Removing existing dmg-file (possibly left from previous test-run)" && rm ~/Hyves-Desktop-setup-old.dmg

# fetch dmg image from server
curl ${UPDATEURL} -o ~/Hyves-Desktop-setup-old.dmg

if [ ! -f ~/Hyves-Desktop-setup-old.dmg ]
then
    echo "Failed to fetch the dmg-file from ${UPDATEURL}"
	echo "TEST FAILED!"
    exit 1
fi

# Install Hyves Desktop to /Applications folder (remove previous installation if it exists)
hdiutil attach ~/Hyves-Desktop-setup-old.dmg -noautoopen 

if test -e /Volumes/Hyves\ Desktop/Hyves\ Desktop.app
then
	hdpath="/Applications/Hyves Desktop.app"
	[ -e "${hdpath}" ] && echo "Removing existing installation in ${hdpath}" && rm -rf "${hdpath}"
	echo "Copying Hyves Desktop.app to /Applications"
	cp -rp /Volumes/Hyves\ Desktop/Hyves\ Desktop.app /Applications
elif test -e /Volumes/Hyves\ Desktop/HyvesDesktop.app
then
	hdpath="/Applications/HyvesDesktop.app"
	[ -e "${hdpath}" ] && echo "Removing existing installation in ${hdpath}" && rm -rf "${hdpath}"
	echo "Copying HyvesDesktop.app to /Applications"
	cp -rp /Volumes/Hyves\ Desktop/HyvesDesktop.app /Applications
else
	umount /Volumes/Hyves\ Desktop || echo "Failed to unmount dmg-file. Please clean up manually."
	echo "No app in the bundle"
	echo "TEST FAILED!"
	exit 2
fi

umount /Volumes/Hyves\ Desktop || echo "Failed to unmount dmg-file. Please clean up manually."

# check if we installed the correct version of Hyves Desktop
hdversion=""
if [ -f "${hdpath}/Contents/MacOS/HyvesDesktop" ]
then
	hdversion=$("${hdpath}/Contents/MacOS/HyvesDesktop" --version-number)
fi

if [ -z "${hdversion}" ]
then
	echo "Not able to determine version number of Hyves Desktop."
	echo "Either the installation failed or an very old version of Hyves Desktop was installed that doesn't know about the '--version-number' switch yet."
	echo "TEST FAILED"
	exit 3
fi

if [ "${hdversion}" != "${OLDVERSION}" ]
then
	echo "Installed Hyves Desktop ${hdversion} but expected version ${OLDVERSION}"
	echo "TEST FAILED!"
	exit 4
fi

echo "Test succeeded!"
exit 0

