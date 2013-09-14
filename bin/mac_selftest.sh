#!/bin/bash

# we do not want the hyvesdeskopt to be running
if ps -x | grep -v grep | grep HyvesDesktop$
then
	echo "Killing currently running instance of HyvesDesktop..."
	sudo killall -9 HyvesDesktop
	exitcode=$?
	if ((exitcode != 0))
	then
		echo "Could not stop running HyvesDesktop"
		exit 1
	fi
fi

echo "Looking for HyvesDesktop installation..."
if [ -d "/Applications/Hyves Desktop.app" ]
then
	hdapp="/Applications/Hyves Desktop.app" 
elif [ -d "/Applications/HyvesDesktop.app" ]
then
	hdapp="/Applications/HyvesDesktop.app"
else
	echo "Could not find HyvesDesktop"
	exit 1
fi

hdexec="${hdapp}/Contents/MacOS/HyvesDesktop"
hdversion=$("${hdexec}" --version-number)
echo "Found Hyves Desktop version ${hdversion} installed in ${hdapp}"

echo "Running Kwekker selftest..."
"${hdexec}" --test 2>&1
kwekker_exitcode="$?"
if [ ${kwekker_exitcode} -eq 0 ]
then
	echo "Kwekker selftest successful"
else
	echo "Kwekker selftest failed!"
fi

echo "Running PhotoUploader selftest..."
"${hdexec}" --plugin photouploader --test 2>&1
photo_exitcode="$?"
if [ ${photo_exitcode} -eq 0 ]
then
	echo "PhotoUploader selftest successful"
else
	echo "PhotoUploader selftest failed!"
fi


echo "Running usagestats upload test..."
if [ -f "/Users/mactester/HyvesDesktop/trunk/bin/teststatsupload.py" ]
then
	/Users/mactester/HyvesDesktop/trunk/bin/teststatsupload.py 2>&1
	stats_exitcode=$?
else
	echo "Could not find teststatsupload.py"
	exit 1 
fi

if ((${stats_exitcode} != 0))
then
	echo "StatsUploader test failed"
fi


exit $(expr ${kwekker_exitcode} + ${photo_exitcode} + ${stats_exitcode})

