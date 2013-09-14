"""Allows active sleeping while waiting for the desired tester to be up again.

This serves as a synchronization step between test part 1 and test part 2 of the several
Hyves Desktop tester slaves.
Takes the name of the builder (defined in buildbot config) as parameter. This specifies 
the tester it will wait for availability.
"""
import subprocess
import time
import platform
import sys

host=None

print sys.argv

if len(sys.argv) > 1:
	if sys.argv[1] == "desktop-tester-vista":
		host='desktop-tester-vista.dev'
	elif sys.argv[1] == "desktop-tester-xp":
		host='desktop-tester-xp.dev'
	elif sys.argv[1] == "desktop-tester-mac":
		#desktop-tester-mac does not have a registered name - so use the ip for now
		host='192.168.13.49'
	else:
		print "tester %s is not recognized" % sys.argv[1]
		sys.exit(1)
else:
	print "please specify which tester you want to wait for"
	sys.exit(1)
	

if platform.system() == 'Windows':
	ping_count_option='-n'
else:
	ping_count_option='-c'

MAX_WAIT=300
start_time = time.time()
time_elapsed = 0
ret = 1

#print "if this script starts too early, we might try to ping host just before it reboots"
#print "so first sleep a little bit"
time.sleep(60)

#loops until the tester is reachable or the time allowed is elapsed
while ret and not time_elapsed > MAX_WAIT:
	ret = subprocess.call("ping %s 1 %s" % (ping_count_option, host),
				shell = True,
				stdout=subprocess.PIPE,
				stderr=subprocess.PIPE)
	time_elapsed = time.time() - start_time 
	time.sleep(10)
	print "trying to ping host - %i seconds elapsed so far " % time_elapsed
				
if not ret:
	print "%s is alive " % host
	sys.exit(0)
else:
	print "%s is down " % host	
	sys.exit(1)
