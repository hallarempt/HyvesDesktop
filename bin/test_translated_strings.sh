#!/bin/bash

#This script makes sure  all translated strings in all javascript files have been declared in Strings.cpp file.

echo "find all translated strings in all javascript files"
find . -name "*.js" | xargs egrep -h -o -E "tr\(\"[^\"]*\"\)" | sort | uniq > js_translation.txt 

echo "find all translated strings declared in all Strings.cpp"
find . -name "Strings.cpp" | xargs egrep -h -o -E "tr\(\"[^\"]*\"\)" | sort | uniq > cpp_translation.txt 

echo "differences between two files"
diff js_translation.txt cpp_translation.txt > diff_translation.txt 

echo "only grep strings that are in Javascript but not in Strings.cpp"
grep "< " diff_translation.txt

EXITCODE=$? 

rm -f js_translation.txt
rm -f cpp_translation.txt
rm -f diff_translation.txt

if (( ${EXITCODE} == 0 ))
then
	echo "[FAILED] Previous strings have not been declared in a Strings.cpp file. Please fix it and re-run this test"
	exit 1;
else
	echo "[PASS] All translatable strings in Javascript code have been written in a Strings.cpp file."
	exit 0;
fi