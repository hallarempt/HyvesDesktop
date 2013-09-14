"""Provides services used by the test system.

"""
import os
import shutil
import tempfile
import sys
import re
import urllib
import subprocess
import platform
import shutil

import hyvesdesktop.util
import hyvesdesktop.application

import logging
#LOG_FILENAME = os.path.join(os.path.join(tempfile.gettempdir(),"hyves-desktop-test.log"))
#buildbot already captures stdout, so it is simpler and more useful to save it in a file
logging.basicConfig(level=logging.DEBUG,filemode='w')

	
class Setup:
	"""Installs virgin environment for test.
	
	This has to be implemented by platform-specific classes. 
	"""
	download_server_url= "download.hyves.org"
	download_test_server_ip = "172.31.0.51"
	
	installed = False
	
	def __init__(self, pyfuncitem):
		
		self.config = pyfuncitem.config
		branch = self.getBranchName()
		logging.info("branch : " + branch)
		path_to_old_setup_download = "kwekker2/testing/" + branch + "/versions/old"
		path_to_new_setup_download = "kwekker2/testing/" + branch + "/versions/latest"
		path_to_release_download = "kwekker2/testing/release"
		if branch == "trunk":
			postfix = "-trunk"
		else:
			postfix = ""
		
		# a map of available hyves desktop installers and their respective
		# download paths on the server
		self.application_download_path = {
					"old" : [path_to_old_setup_download, "Hyves-Desktop-setup" + postfix], 
					"new" : [path_to_new_setup_download, "Hyves-Desktop-setup" + postfix],
					"release_1.0" : [path_to_release_download + "/1.0", "Hyves-Desktop-setup-1.0"],
					"release_1.1" : [path_to_release_download + "/1.1", "Hyves-Desktop-setup-1.1"],
					"release_1.2" : [path_to_release_download + "/1.2", "Hyves-Desktop-setup-1.2"],
					"release_1.3" : [path_to_release_download + "/1.3", "Hyves-Desktop-setup-1.3"],
					}
					 
		#application that will be installed in a virgin environment			 
		self.selectedApplication = None
		
		modified_line = self.download_test_server_ip + " " + self.download_server_url		
		self.systemFileModifier = hyvesdesktop.util.SystemFileModifier(self.host_file,
									       	self.backup_host_file,
										modified_line)
		pyfuncitem.addfinalizer(self.tearDown)


	def getBranchName(self):
		"""Get the actual branch, e.g. : '1.2' or 'trunk'

		   Try to get branch name from cmdline option first
		   if not specfied use current name of working directory
		"""
		if self.config.option.branch is not None:
			return self.config.option.branch

		directories = os.path.split(os.getcwd())
		
		branch_name = directories[1]
		return branch_name
		
	def selectApplication(self, application_id):
		"""Selects which application we want to update 
		
		@param application_id could be :
				"new" : latest installer available
				"old" : (latest -1) installer
				"release_1.0" : release installer 1.0
				"release_1.1" : release installer 1.1
				"release_1.2" : release installer 1.2 (as currently accessible from Hyvers)
		"""
		if application_id in self.application_download_path:
			self.selectedApplication = application_id
			logging.debug("selected application : " + self.selectedApplication)
		else:
			logging.error("application not selected : " + application_id)
			
														
	def setUp(self,app_path=None):
		"""sets the environment for our test.
		"""
		assert self.selectedApplication
		if app_path:
			if not os.path.exists(app_path):
				os.mkdir(app_path)
			self.app_path = app_path
		#kills remaining instances of Hyves Desktop/Crash Reporter
		self.killProgram()

		#modify hosts file to ensure we use ip address of test download server
		self.systemFileModifier.modifyHostFile()
		
		#all the work needed before an install can take place
		installer_path = self._preInstall()
	
		logging.debug("installing application in " + self.app_path)
		return_code = self.install(installer_path)
		if return_code:
			logging.warning("install returned non-zero exit code : " + str(return_code))
		else:
			logging.debug("application installed")
			self.installed = True
	
	def _preInstall(self):
		""" Prepares the groundwork for the install to be executed.	
		"""
		#step 1 - remove current install and default install
		self.removeInstall(self.default_directory)
		self.removeInstall(self.app_path)
		os.mkdir(self.app_path)
			
		#step 2 - download the installer from the server
		installer_base_path = tempfile.gettempdir()
		installer_name = self.application_download_path[self.selectedApplication][1] + \
							self.installer_extension
		installer_path = os.path.join(installer_base_path,installer_name)
		
		full_server_path = "http://" + self.download_server_url + "/" + \
					self.application_download_path[self.selectedApplication][0] + \
					"/" + installer_name
	   	logging.debug("downloading installer from " + full_server_path)

		urllib.urlretrieve(full_server_path, installer_path)
		logging.debug("file saved in " + installer_path)
				   
		if platform.system() != "Windows":
			#adding executable permission
			os.system('chmod +x ' + '"' + installer_path + '"')
		return installer_path
		
	def removeInstall(self, app_path):
		if os.path.exists(app_path):
			logging.debug("cleaning install in " + app_path )
			shutil.rmtree(app_path, True)

	def tearDown(self):
		"""Performs all needed cleanup before destruction
		"""
		self.killProgram()
		self.systemFileModifier.revertHostFile()
		self.removeInstall(self.app_path)
		
	def application(self):
		"""Returns new instance of an Application object
		"""
		if not self.installed:
			logging.debug("application setup not properly initialized")
			return None
			
		return hyvesdesktop.application.Application(
				self.application_download_path["new"][0], 
				os.path.join(self.app_path,self.app_exe)
				)
		
	def fileTimestampManager(self):
		return hyvesdesktop.util.FileTimestampManager(self.app_path)
		
	def killProgram(self):
		raise NotImplementedError
	
	def install(self):
		raise NotImplementedError
		
	def uninstall(self):
		raise NotImplementedError
	
	def is_uninstall_clean(self):
		raise NotImplementedError
		
	def checkPlatformSpecificPostConditions(self):
		"""checks post-conditions specific to the tested platform. 
		
		This can be checked at the end of a successful update.
		"""
		return True
		
class WindowsSetup(Setup):
	"""Windows-specific implementation for setup.
	"""
	uninstaller_name = "uninstall.exe"
	installer_extension = ".exe"
	default_directory = r"C:\Program Files\Hyves Desktop"
	app_path = r"C:\Program Files\Hyves Desktop"
	app_exe = r"bin\HyvesDesktop.exe"
	
	host_file = r"C:\WINDOWS\system32\drivers\etc\hosts"
	
	shortcuts = [r"C:\Documents and Settings\All Users\Desktop\Hyves Desktop Chat.lnk",
				r"C:\Documents and Settings\All Users\Desktop\Hyves Desktop Photo Uploader.lnk"]
	homedir = os.path.expanduser('~')
	backup_host_file = os.path.join(homedir,"hosts.bak")
	
	
	def __init__(self, pyfuncitem):	
		Setup.__init__(self, pyfuncitem)

		self.original_file_count_at_root = 0
	
	def setUp(self, app_path = None):
	
		Setup.setUp(self,app_path)
		root = os.path.splitdrive(self.app_path)[0] + "/" 
		files = os.listdir(root)
		for name in files:
			if re.match("eula..*.txt",name):
				os.remove(root + name)
				logging.debug("removing file : " + root + name)
		self.original_file_count_at_root = self.getFileCountAtRoot()
			
	def getFileCountAtRoot(self):
		return len(os.listdir(os.path.splitdrive(self.app_path)[0] + "/"))
	
	def killProgram(self):
		proc = subprocess.Popen("taskkill /F /IM HyvesDesktop.exe /IM HYVESD~1.EXE /IM CrashReporter.exe",
					stdout=subprocess.PIPE,stderr=subprocess.PIPE)
		output, errors = proc.communicate()
		if errors:
			not_found = re.findall("process (\S*?) not found",errors)
			if not_found:
				for program in not_found:
					logging.debug("%s process not found" % program)
			else:
				logging.debug("killProgram: " + errors)
		
	def install(self, installer_path):
		
		#return_code = subprocess.call(installer_path, '--prefix',self.app_path, '--mode','unattended'])
		cmdLine = '"' + installer_path + '" ' + \
					'--prefix ' +  '"' + self.app_path + '" ' + \
					'--mode unattended'
		logging.debug("installer command line : " + cmdLine)
		process = subprocess.Popen(cmdLine,	shell=False)
		return process.wait()
		
	def uninstall(self):
	
		uninstaller_path = os.path.join(self.app_path,self.uninstaller_name)
		subprocess.call([uninstaller_path, '--mode','unattended'])
	
	def is_uninstall_clean(self):
		
		binExists = os.path.exists(os.path.join(self.app_path,"bin"))
		pluginExists = os.path.exists(os.path.join(self.app_path,"plugins"))
		shortcutExists = False
		for shortcut in self.shortcuts:
			if os.path.exists(shortcut):
				shortcutExists = True
		if binExists or pluginExists or shortcutExists:
				msg =""
				if binExists: 
					msg += "binary directory still present\n"
				if pluginExists:
					msg += "plugins directory still present\n"
				if shortcutExists:
					msg += "shortcuts still exist\n"
				logging.warning("uninstall not clean : %s" % (msg))
				return False
		return True
	
	def defaultDirectory(self):
		return self.default_directory
	
	def checkPlatformSpecificPostConditions(self):
		sameFileCount = (self.original_file_count_at_root == self.getFileCountAtRoot())
		if not sameFileCount:
			logging.warning("the file count at the roor has increased.")
		return sameFileCount
		
class MacSetup(Setup):
	"""Mac-specific implementation for setup.
	"""
	
	installer_extension = ".dmg"
	default_directory = r"/Applications/Hyves Desktop.app"
	app_path = r"/Applications/Hyves Desktop.app"
	app_exe = r"Contents/MacOS/HyvesDesktop"
	
	host_file = r"/etc/hosts"
	homedir = os.path.expanduser('~')
	backup_host_file = os.path.join(homedir,"hosts.bak")
	
			
	def killProgram(self):
		os.system("killall -9 HyvesDesktop")

	def install(self, installer_path):
	
		os.system('hdiutil detach "/Volumes/Hyves Desktop"')
		os.system('hdiutil attach ' + '"' + installer_path  +  '"' + ' -noautoopen')
		if os.path.exists("/Volumes/Hyves Desktop/Hyves Desktop.app"):
			os.system('ditto "/Volumes/Hyves Desktop/Hyves Desktop.app" ' +  '"' + self.app_path + '"')
		else:
			os.system('ditto "/Volumes/Hyves Desktop/HyvesDesktop.app" ' + '"' + self.app_path + '"')
		os.system('hdiutil detach "/Volumes/Hyves Desktop"')
		return 0

class LinuxSetup(Setup):

	installer_extension = ".bin"
	default_directory = os.path.join(os.path.expanduser('~'),"Hyves Desktop")
	app_path = os.path.join(os.path.join(tempfile.gettempdir(), "hyves-desktop-testing"))
	app_exe = "bin/HyvesDesktop.sh"
	
	host_file = r"/etc/hosts"
	
	homedir = os.path.expanduser('~')
	backup_host_file = os.path.join(homedir,"hosts.bak")

	def __init__(self, pyfuncitem):
		Setup.__init__(self,pyfuncitem)
		#os.putenv("DISPLAY",":0.0")

	def killProgram(self):
		os.system("killall -9 HyvesDesktop")

	def install(self, installer_path):
	
		prog = subprocess.Popen(installer_path + ' --mode unattended --prefix ' + self.app_path , shell=True)
		return prog.wait()
		
