#!/usr/bin/env python

import sys
import os
from glob import glob

# Generate a .pro file for lupdate lrelease to work on
projects = ['3rdparty/qtimagefilters/src/', 'crashreporter', 'src', 'plugins/kwekker/src', 'plugins/photouploader/src', 'data/bundle']

def listFiles(dir, headers, sources, forms):
	headers += glob( dir + "/*.h")
	headers += glob( dir + "/*.js")
	sources += glob( dir + "/*.cpp")
	forms += glob(dir + "/*.ui")
	for item in os.listdir(dir):
		item = os.path.join(dir, item)
		if os.path.isdir(item):
			if item != ".svn":
				listFiles(item, headers, sources, forms)

def main(argv):
	print "Creating a .pro file..."
	f = open("translatable.pro", "w+")
	f.write("TRANSLATIONS = HyvesDesktop_nl.ts\n\n")
	headers = []
	sources = []
	forms = []
	for project in projects:
		listFiles(project, headers, sources, forms)
	f.write("HEADERS = 	" + headers[0] + " \\\n");
	for header in headers[1:]:
		f.write("		" + header + " \\\n");

	f.write("SOURCES = 	" + sources[0] + " \\\n");
	for source in sources[1:]:
		f.write("		" + source + " \\\n");

	f.write("FORMS = 	" + forms[0] + " \\\n");
	for form in forms[1:]:
		f.write("		" + form + " \\\n");
	
	f.flush()

if __name__ == "__main__":
	main(sys.argv)
