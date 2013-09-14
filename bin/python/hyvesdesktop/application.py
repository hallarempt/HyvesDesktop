"""Encapsulates Hyves Desktop application actions.
"""

#windows
import ctypes

import re
import subprocess
import platform
import time
import os
import logging

import signal

import version



class Application:
	"""Performs different actions on a Hyves Desktop application.
	"""
	CORRUPT_UPDATE_WAIT_TIME = 20
	INSTALL_EXIT_CODE = 57
	INSTALL_UPDATE_WAIT_TIME = 40
	
	def __init__(self,path_to_download, app_exe):
		logging.debug("application exe path : " + app_exe)
		self.app_exe = app_exe
		self.path_to_download = path_to_download
		
	def version(self):
		#necessary to escape white space in exe path if any		
		prog = [ self.app_exe, '--version']
		proc = subprocess.Popen(prog, 
                        stdin=subprocess.PIPE,
                        stdout=subprocess.PIPE,
                        ) 
		outputText = proc.communicate()[0]

		pattern = re.compile ("Hyves Desktop (\d)\.(\d)\.(\d{1,4})")
		result = pattern.match(outputText)
		if result:
			current_version = version.Version(result.groups()[0],
							result.groups()[1],  
							result.groups()[2])
		else:
			current_version = version.Version(0,0,0)
		logging.debug("current version : " + str(current_version))
		return current_version
				
	def downloadCorruptInstall(self, old_version):
		
		if old_version < version.Version(1,0,2933):
			logging.debug("test not executed. Current version " + \
							str(old_version) + \
							" not able to select different server paths")
			return
		logging.debug("now starting corrupt install")
		#this doesn't seem to be working on MAC
		#process = subprocess.Popen([self.app_exe, 
		#'--silent-update --server-update-path kwekker2/testing/corrupt'])		

		process = subprocess.Popen( '"' + self.app_exe + '"' + 
				' --silent-update --server-update-path kwekker2/testing/corrupt',
				shell=True)
		
		time.sleep(self.CORRUPT_UPDATE_WAIT_TIME)
		
		if platform.system() == "Windows":
			ctypes.windll.kernel32.TerminateProcess(int(process._handle), -1)
		else:
			os.kill(process.pid,signal.SIGTERM)
			
	def performRegularUpdate(self):
	
		cmd = '"' + self.app_exe + '"' + \
			' --clear-settings --silent-update --server-update-path ' + self.path_to_download
		logging.debug("starting " + cmd)
		process = subprocess.Popen( cmd, shell=True)
		timeout = 50
		elapsedTime = 0
		retCode = None
		#loops until application finishes or timeout occurs
		while not retCode and elapsedTime < timeout:
			elapsedTime = elapsedTime +5
			retCode = process.poll()
			time.sleep(5)
			
		if not retCode:
			logging.debug("the process is not finished, kill it!")
			if platform.system() == "Windows":
				ctypes.windll.kernel32.TerminateProcess(int(process._handle), -1)
			else:
				os.kill(process.pid,signal.SIGTERM)
		return retCode
		
	def waitForUpdateFinished(self):

		logging.debug("now allowing installer " + 
					str(self.INSTALL_UPDATE_WAIT_TIME) + 
					" seconds to terminate.")
		time.sleep(self.INSTALL_UPDATE_WAIT_TIME)
	
