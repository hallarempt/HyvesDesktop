import os
import random
import simplejson
import syslog
import time

METRIC_HYVESDESKTOP_CRASHES = 145
METRIC_HYVESDESKTOP_JAVASCRIPT_ERRORS = 146
METRIC_HYVESDESKTOP_SERVERSIDE_ERRORS = 147

lastPath = ""
root = "/var/socorro/toBeDeferred/date"

def findJsonFiles(dir, followSymLinks = True):
    global lastPath
    for entry in sorted(os.listdir(dir)):
        path = os.path.join(dir, entry)
	if len(path) == len(lastPath):
	    if path <= lastPath:
	        continue
	else:
            if path < lastPath[:len(path)]:
                continue
        if os.path.isdir(path):
            if followSymLinks:
                for x in findJsonFiles(path, not os.path.islink(path)):
                    yield x
            elif not os.path.islink(path):
                for x in findJsonFiles(path, False):
                    yield x
        elif path[-5:] == ".json":
            yield path

def processJsonFile(path):
    # check if there is a valid dump file, and report a crash if so...
    dumpPath = path[:-5] + ".dump"
    if os.path.exists(dumpPath) and os.path.getsize(dumpPath) > 0:
        logMetric(METRIC_HYVESDESKTOP_CRASHES)

    file = open(path)
    try:
        json = simplejson.load(file)
        if "logfile" in json:
            for line in json["logfile"].split("\n"):
	        processLogLine(line)
    finally:
        file.close()

def parseLogLine(line):
    if line.count(": ") < 2:
        return ("", "", "", "")

    if line[:2] == "20":
        dateTime = line[:19]
        (level, className, message) = line[21:].split(": ", 2)
    else:
        dateTime = None
        (className, level, message) = line.split(": ", 2)
    if className == "WebPage" and level == "NOTICE" and message.find(": ") > -1:
        (level, message) = message.split(": ", 1)
    return (dateTime, level, className, message)

def processLogLine(line):
    (dateTime, level, className, message) = parseLogLine(line)

    if level == "ERROR":
        if className == "WebView" or className == "HyvesAPI":
            logMetric(METRIC_HYVESDESKTOP_SERVERSIDE_ERRORS)
        elif className == "WebPage":
            logMetric(METRIC_HYVESDESKTOP_JAVASCRIPT_ERRORS)

def logMetric(metric, count = 1, memberId = 0, rand = 1):
    if random.randint(1, rand) == 1:
        syslog.openlog("metric", syslog.LOG_NDELAY, syslog.LOG_LOCAL7)
        syslog.syslog(syslog.LOG_NOTICE, "%s:%s:%s" % (metric, count, memberId))

def main():
    global lastPath
    
    try:
        file = open("lastpath.txt")
        lastPath = file.readline().strip()
        file.close()
    except:
        None

    pathChanged = False
    while True:
        for path in findJsonFiles(root):
            processJsonFile(path)
            lastPath = path
            pathChanged = True
        
        if pathChanged:
            file = open("lastpath.txt", "w")
	    file.write(lastPath)
	    file.close()
        
        time.sleep(1)

if __name__ == '__main__':
    main()
