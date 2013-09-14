#!/bin/bash

svn_base="https://trac.startpda.net/svn/desktop"

if [ "$1" == "" ] || [ "$2" == "" ]; then
	echo "Usage: bin/createreleasebranch.sh <major> <minor> [<patch>]"
	exit 1
fi

modified_files=`svn status | grep "^[^?]"`
if [ "$modified_files" != "" ]; then
	echo "Please make sure you have no local SVN changes before running this script."
	exit 1
fi

if [ "$3" == "" ]; then
	version_string="$1.$2"
else
	version_string="$1.$2.$3"
fi

cp CMakeLists.txt CMakeLists.txt.orig
cat CMakeLists.txt.orig | sed -e "s/HD_MAJOR_VERSION [0-9]\+/HD_MAJOR_VERSION $1/" -e "s/HD_MINOR_VERSION [0-9]\+/HD_MINOR_VERSION $2/" > CMakeLists.txt

pushd build
	cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
popd

for f in hyvesdesktop.xml hyvesdesktop-win32.xml; do
	p="distribution/bitrock/$f"
	cp $p $p.orig
	cat $p.orig | sed -e "s/<version>.*<\/version>/<version>$version_string<\/version>/" > $p
done

svn commit -m "- Update version number to $version_string."

svn copy -m "- Create branch for version $version_string." "$svn_base/trunk" "$svn_base/branches/$version_string"

echo "Please give the root password on desktop-builds.dev to configure the builds."
echo "branches/$version_string" > release-branch.txt
scp release-branch.txt root@desktop-builds.dev:/var/www/desktop-builds
rm release-branch.txt

echo "Please give the www-data password on desktop-builds.dev twice to configure the build locations."
scp -r www-data@desktop-builds.dev:desktop-builds/kwekker2/testing/trunk .
find trunk -name Hyves-Desktop-setup-trunk.* | sed "s/\([0-9a-zA-Z./]*\/Hyves-Desktop-setup\)\(.*\)\.\([0-9a-zA-Z]\{3\}\)/mv \1\2.\3 \1.\3/g" | sh
find trunk/versions -name updates.xml | sed "s/\(.*\)/cp \1 \1.tmp; cat \1.tmp | sed 's:-trunk::g' > \1; rm \1.tmp/g" | sh
scp -r trunk www-data@desktop-builds.dev:desktop-builds/kwekker2/testing/$version_string
rm -Rf trunk
