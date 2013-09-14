#!/usr/bin/python
# -*- coding: iso-8859-1 -*-
""" Test script for usage and log file uploader
This script provides a test for the logfile uploading feature of Hyves Desktop.
It will start a new thread running a small webserver on localhost:8000

HyvesDesktop will be invoked with the --uploadtest <unique_key> argument and should
be able to upload it's current log to the webserver.

The test is successful if...
1. the '<unique_key>' is among the data received
2. the attributes required by the server are present and in the correct order
3. the submitted logfile has been truncated (should not be larger than 200k)
"""
import os
import sys
import platform
import subprocess
import string
import random
import socket
import time

from cgi import parse_qs
from threading import Thread
from wsgiref.simple_server import make_server

HD_VERSION_MIN = "1.3.4849"

html_response = """
	<html>
		<head>
			<title>stats submitted</title>
		</head>
		<body>
			<h2>stats submitted</h2>
			Thanks for uploading usage stats to the test server ;)<br><br>
			Note for Arend: We could do fancy stuff in this server response like
			sending the date for the next stats upload...
		</body>
	</html>
	"""


class UploadServer(Thread):
	def __init__ (self):
		Thread.__init__(self)
		self.receivedData = ""
		self.httpd = None
	
	def run(self):
		try:
			self.httpd = make_server('localhost', 8000, self.handle_request)
			#httpd.serve_forever()
			self.httpd.handle_request()
			return 0
		except socket.error, error:
			print "Failed to start server: %s" % error
			return 1

	def stop(self):
		if self.httpd is None:
			return 1
		# create fake connection to shut down server
		client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		client.connect(("localhost", 8000))
		client.close()
		return 0
	
	def handle_request(self, environ, start_response):
		try:
			request_body_size = int(environ.get('CONTENT_LENGTH', 0))
		except (ValueError):
			request_body_size = 0
		
		request_body = environ['wsgi.input'].read(request_body_size)
		d = parse_qs(request_body)
		self.receivedData = str(d.get(' name', []))
		
		response_body = html_response
		status = '200 OK'
		response_headers = [('Content-Type', 'text/html'), ('Content-Length', str(len(response_body)))]
		start_response(status, response_headers)
		return [response_body]



class Tester:
	def startTestServer(self):
		print "\nStarting the test server..."
		srv = UploadServer()
		srv.start()
		time.sleep(1)
		if not srv.isAlive():
			print "Failed to start test server!"
			sys.exit(1)
		return srv
	
	def generateTestKey(self):
		print "\nGenerating test key..."
		chars = string.ascii_letters + string.digits
		key = "$"
		for i in range(15):
			key += random.choice(chars)
		print "test key is: %s" % key
		return key
	
	def invokeHyvesDesktop(self, key, retry):
		print "\nRunning Hyves Desktop..."

		path = ""
		if platform.system() == "Windows":
			path = r"C:\Program Files\Hyves Desktop\bin\HyvesDesktop.exe"
		elif platform.system() == "Darwin":
			# it should be "Hyves Desktop.app" but for testing we sometimes install to "HyvesDesktop.app" (without space)
			# hopefully this will be cleaned up in the furture...
			if os.path.exists(r"/Applications/HyvesDesktop.app/Contents/MacOS/HyvesDesktop"):
				path = r"/Applications/HyvesDesktop.app/Contents/MacOS/HyvesDesktop"
			else:
				path = r"/Applications/Hyves Desktop.app/Contents/MacOS/HyvesDesktop"
		elif platform.system() == "Linux":
			path = os.path.join(os.path.expanduser('~'),"Hyves Desktop/bin/HyvesDesktop.sh")
		else:
			print "System not implemented: " + platform.system()
			return
		
		if not os.path.exists(path):
			print "Could not find executable for Hyves Desktop! It should be here: %s ...but it isn't!" % path
			return
		
		# check for Hyves Desktop version
		cmdline = "\"" + path + "\" --version-number"
		hd = subprocess.Popen(cmdline, stdout=subprocess.PIPE, shell=(platform.system() != "Windows"))
		hdversion = hd.communicate()[0]
		if hdversion < HD_VERSION_MIN:
			print "Found Hyves Desktop %s which is too old!" % hdversion
			print "This test requires version %s or greater" % HD_VERSION_MIN
			return
		print "Found Hyves Desktop version %s" % hdversion
		
		# run upload test
		cmdline = "\"" + path + "\" --uploadtest '" + key + "' --kill"
		hd = subprocess.Popen(cmdline, stderr=subprocess.PIPE, shell=(platform.system() != "Windows"))
		hderr = hd.communicate()[1]
		print "Output from HyvesDesktop:\n%s" % hderr
		hd.wait()
		if hderr.find("sent to previous instance") > 0 and retry > 0:
			print "It seems HyvesDesktop was already running. Trying again for %i more time(s)..." %retry
			time.sleep(3)
			self.invokeHyvesDesktop(key, retry - 1)
	
	def stopTestServer(self, srv):
		print "\nStopping the test server..."
		srv.join(10)
		if srv.isAlive() and srv.stop():
			srv.join()
		print "Received %i bytes." % len(srv.receivedData)
	
	def evaluateResult(self, receivedData, key):
		errorsFound = 0
		print "\nChecking for required server attributes..."
		attributes = ["BuildID", "ProductName", "Version", "Vendor", "timestamp", "upload_file_minidump", "logfile"]
		prevPos = 0
		prevAttribute = ""
		for attribute in attributes:
			pos = receivedData.find("\"" + attribute + "\"")
			if pos < 0:
				print "Attribute \"%s\" not found!" % attribute
				errorsFound += 1
			elif pos < prevPos:
				print "Attribute \"%s\" found but on wrong position (should be behind \"%s\")" % (attribute, prevAttribute)
				errorsFound += 1
				prevPos = pos
			else:
				print "Attribute \"%s\" found" % attribute
				prevPos = pos
			prevAttribute = attribute
		
		print "\nChecking logfile size..."
		if receivedData.find("If_you_can_read_this_on_the_server_the_logfile_has_not_been_truncated_before_uploading!!!") < 0:
			print "Log file has been truncated."
		else:
			print "Log file was not truncated and is most probably too large!"
			errorsFound += 1

		print "\nLooking for settings dump..."
		settingsStartPos = receivedData.find("<= start settings dump =>\\n")
		settingsEndPos = receivedData.rfind("<= end settings dump =>")
		if settingsStartPos < 0 or settingsEndPos < 0:
			print "Settings dump is missing or corrupted"
			errorsFound += 1
		else:
			settings = receivedData[(settingsStartPos + 27):settingsEndPos]
			print "Found the following settings:"
			for setting in settings.split("\\n"):
				print setting
		
		print "\nLooking for test key..."
		if receivedData.find(key) < 0:
			print "Could not find test key \"%s\" among the received data:" % key
			print receivedData
			errorsFound += 1
		else:
			print "Test key found."

		return errorsFound
	
	def run(self):
		srv = self.startTestServer()
		key = self.generateTestKey()
		self.invokeHyvesDesktop(key, 1)
		self.stopTestServer(srv)
#		with open('teststatsupload.py.out', 'w+') as outfile:
#			outfile.write(srv.receivedData)
		if len(srv.receivedData) == 0:
			print "\nNo data received from Hyves Desktop"
			print "\nTEST FAILED!!!"
			time.sleep(1) # give http server thread some time to shut down properly
			return 3
		if self.evaluateResult(srv.receivedData, key) > 0:
			print "\nTEST FAILED!!!"
			return 2
		else:
			print "\nTEST SUCCESSFULL"
		return 0


if __name__=="__main__":
	print sys.argv
	test = Tester()
	sys.exit(test.run())
