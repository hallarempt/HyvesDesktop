"""Installer script.

"""
import platform
import os
import urllib
import logging
import subprocess
#user-defined modules
import clean


logging.basicConfig(level=logging.DEBUG,filemode='w')

class System:
	"""Facade pattern returning implementation objects of the right type (based on host system type).
	"""
	def cleaner(self):
		raise NotImplementedError
	def installer(self):
		raise NotImplementedError
	
class WindowsSystem(System):
	def cleaner(self):
		return clean.WindowsCleaner()
	def installer(self):
		return WindowsInstaller()

class MacSystem(System):
	def cleaner(self):
		return clean.MacCleaner()
	def installer(self):
		return MacInstaller()	

class LinuxSystem(System):
	def cleaner(self):
		return clean.LinuxCleaner()
	def installer(self):
		return LinuxInstaller()

class SystemFactory:
	def makeSystem(self):
		system = None
		if platform.system() == "Windows":
			system = WindowsSystem()
		
		elif platform.system() == "Darwin":
			system = MacSystem()

		elif platform.system() == "Linux":
			system = LinuxSystem()
		else:
			print "System not implemented : " + platform.system()
		return system


class Installer:
	"""Installs Hyves Desktop on the local machine.
	
	"""
	server_path = "http://desktop-builds.dev"
	local_installer_name = "Hyves-Desktop-setup"
	extension = None
	local_installer_path = None
	app_path = None
	
	def download(self):
		"""download installer from the development server onto the local machine
		"""
		branch_name = self.getBranchName()
		remote_installer_filename = None

		remote_installer_filename = "Hyves-Desktop-setup-" + branch_name + "." + self.extension
		
		self.local_installer_path =  os.path.join(self.local_tmp_path, self.local_installer_name + "." + self.extension)
		full_server_path = self.server_path + "/" + remote_installer_filename
		print "retrieving %s to %s" %(full_server_path, self.local_installer_path)	
		urllib.urlretrieve(full_server_path, self.local_installer_path)
			
	def install(self):
		"""system-dependent install
		"""
		raise NotImplementedError

	def systemCheck(self):	
		"""ssytem-dependent system check
		"""
		return
	
	def getBranchName(self):
		"""Get the actual branch, e.g. : '1.2' or 'trunk'
		"""
		directories = os.path.split(os.getcwd())
		
		branch_name = directories[1]
		return branch_name
	
class WindowsInstaller(Installer):
	extension="exe"
	local_tmp_path = r'C:\WORK'
	app_path = r'C:\Program Files\Hyves Desktop'
	
	def install(self):
		cmdLine = '"' + self.local_installer_path + '" ' + \
					'--prefix ' +  '"' + self.app_path + '" ' + \
					'--mode unattended'
		logging.debug("installer command line : " + cmdLine)
		process = subprocess.Popen(cmdLine, shell=False)
		return process.wait()

	def systemCheck(self):
		
		assert os.path.exists(os.path.join(self.app_path,"bin"))
		assert os.path.exists(os.path.join(self.app_path,"plugins"))
		assert os.path.exists(os.path.join(self.app_path,"uninstall.exe"))
		if platform.release() == "XP":
			assert os.path.exists(r"C:\Documents and Settings\All Users\Desktop\Hyves Desktop Chat.lnk")
			assert os.path.exists(r"C:\Documents and Settings\All Users\Desktop\Hyves Desktop Photo Uploader.lnk")
		else:#vista
			assert os.path.exists(r"C:\Users\Public\Desktop\Hyves Desktop Chat.lnk")
			assert os.path.exists(r"C:\Users\Public\Desktop\Hyves Desktop Photo Uploader.lnk")
			
	
class MacInstaller(Installer):
	extension="dmg"
	local_tmp_path =  os.path.expanduser('~')
	app_path = r'/Applications/HyvesDesktop.app'
	
	def install(self):
	
		os.system('hdiutil detach "/Volumes/Hyves Desktop"')
		os.system('hdiutil attach ' + '"' + self.local_installer_path  +  '"' + ' -noautoopen')
		os.system('ditto "/Volumes/Hyves Desktop/Hyves Desktop.app" ' +  '"' + self.app_path + '"')
		os.system('hdiutil detach "/Volumes/Hyves Desktop"')

	def systemCheck(self):
		
		assert os.path.exists(self.app_path)
		

class LinuxInstaller(Installer):
	extension="bin"
	local_tmp_path = os.path.expanduser('~')
	app_path = os.path.join(os.path.expanduser('~'),"Hyves Desktop")
	
	def install(self):

		subprocess.call(["chmod", "u+x", self.local_installer_path])
		cmdLine = '"' + self.local_installer_path + '" ' + \
					'--prefix ' +  '"' + self.app_path + '" ' + \
					'--mode unattended'
		logging.debug("installer command line : " + cmdLine)
		process = subprocess.Popen(cmdLine, shell=True)
		return process.wait()
	
	def systemCheck(self):
		
		assert os.path.exists(self.app_path)


if __name__=="__main__":
	systemFactory = SystemFactory()
	system = systemFactory.makeSystem()
	# 1- cleaning steps
	cleaner = system.cleaner()
	cleaner.kill()
	cleaner.findHyvesFiles()
	cleaner.removeInstall()
	# 2 - installing !
	installer = system.installer()
	installer.download()
	installer.install()
	# 3 - post-install checks
	installer.systemCheck()
