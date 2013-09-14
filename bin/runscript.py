import os
import sys
import urllib
from optparse import OptionParser
from subprocess import call

def main():
    parser = OptionParser(usage="usage: %prog [options] <script> <arguments>")
    parser.add_option("-b", "--branch",
                      action="store", dest="branch", default="trunk",
                      help="The branch in which the script is going to be executed.")
    (options, args) = parser.parse_args()
    
    if len(args) < 1:
        sys.exit(1)
    
    branch = options.branch
    if branch == "": 
        branch = "trunk"
    elif branch == "release":
        r = urllib.urlopen("http://desktop-builds.dev/release-branch.txt")
        branch = r.readline().strip()
	if branch == "":
	    sys.exit(0)
    
    if branch != "trunk":
        path = "../" + branch
        if os.path.exists(path):
            os.chdir(path)
	else:
            (head, tail) = os.path.split(path)
	    os.chdir(head)
	    result = call(["svn", "up", tail])
	    if result != 0:
	        sys.exit(2)
	    os.chdir(tail)
    
    if args == ["svn", "up"]:
        result = call(["svn", "up"])
	sys.exit(result)
    
    if args[0].endswith(".py"):
        result = call(["python"] + args)
    else:
        result = call(args)
    sys.exit(result)

if __name__ == "__main__":
    main()
