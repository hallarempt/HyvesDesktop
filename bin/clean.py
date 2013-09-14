"""Kills instances of Hyves Desktop and Crash Reporter and wipes out HyvesDesktop directory

Mac cleaner also removes application data files"
"""
import subprocess
import shutil
import os
import platform
import sys
import re
import time

class Cleaner:
	def removeInstall(self):
		if self.app_path:
			if os.path.exists(self.app_path):
				print "removing current install at %s" % (self.app_path)
				shutil.rmtree(self.app_path)
			assert not os.path.exists(self.app_path)
	
	def findHyvesFiles(self):
		return

	def restoreHostFile(self):
		backup_file = self.host_file + ".backup.cleaner"
		if os.path.exists(backup_file):
			print "restoring hosts file: copying backup (%s) into hosts file (%s)" % (backup_file,self.host_file)
			shutil.copyfile(backup_file, self.host_file)
		else:
			print "creating backup hosts file : " + backup_file
			shutil.copyfile(self.host_file, backup_file)
	
class WindowsCleaner(Cleaner):

	app_path = r"C:\Program Files\Hyves Desktop"
	host_file = r"C:\WINDOWS\system32\drivers\etc\hosts"
	def kill(self):
		proc = subprocess.Popen("taskkill /F /IM HyvesDesktop.exe /IM HYVESD~1.EXE /IM CrashReporter.exe /IM VCREDI~1.",
								stdout=subprocess.PIPE,stderr=subprocess.PIPE)
		output, errors = proc.communicate()
		if errors:
			print "in killProgram," + errors 
		time.sleep(2)

class MacCleaner(Cleaner):

	host_file = r"/etc/hosts"
	def __init__(self):
		homedir = os.path.expanduser('~')
		self.preferences_files = []   
		self.application_data_dirs = []
		#the app directory will be found by the application in findHyvesFiles()
		self.app_path=None

	def findHyvesFiles(self):
		print "\nHyves files discovery started.\n"

		homedir = os.path.expanduser('~')
		
		applications_dir = "/Applications"
		app_pattern = re.compile("Hyves[\s]?Desktop\.app")		
		for name in os.listdir(applications_dir):
			if app_pattern.search(name):
				print "Hyves Desktop application path found : " + os.path.join(applications_dir,name)
				self.app_path = (os.path.join(applications_dir,name))

		preferences_dir = os.path.join(homedir,r"Library/Preferences")
		plist_pattern = re.compile(".*?Hyves[\s]?Desktop.plist")
		for name in os.listdir(preferences_dir):
			if plist_pattern.search(name):
				print "plist file found : " + os.path.join(preferences_dir,name)
				self.preferences_files.append(os.path.join(preferences_dir,name))

		application_support_dir = os.path.join(homedir,r"Library/Application Support")
		application_data_pattern = re.compile("Hyves[\s]?Desktop")
		for name in os.listdir(application_support_dir):
			if application_data_pattern.search(name):
				print "Hyves Desktop application data dir found " + os.path.join(application_support_dir,name)
				self.application_data_dirs.append(os.path.join(application_support_dir,name))
		print "\nHyves files discovery finished. We can now proceed with removal ...\n"

	def kill(self):
		os.system("killall -9 HyvesDesktop")

	def cleanPreferences(self):
		for preferences_file in self.preferences_files:
			if os.path.exists(preferences_file):
				print "removing existing preferences at %s" % (preferences_file)
				os.remove(preferences_file)
			assert not os.path.exists(preferences_file)


	def cleanApplicationData(self):
		for application_data_dir in self.application_data_dirs:
			if os.path.exists(application_data_dir):
				print "removing existing application data at %s" % (application_data_dir)
				shutil.rmtree(application_data_dir)
			assert not os.path.exists(application_data_dir)


class LinuxCleaner(Cleaner):
	app_path = os.path.join(os.path.expanduser('~'),"Hyves Desktop")
	host_file = r"/etc/hosts"
	def kill(self):
		os.system("killall -9 HyvesDesktop")
		
class CleanerFactory:
	def makeCleaner(self):
		cleaner = None
		if platform.system() == "Windows":
			cleaner = WindowsCleaner()
		elif platform.system() == "Darwin":
			cleaner = MacCleaner()	
		elif platform.system() == "Linux":
			cleaner = LinuxCleaner()
		return cleaner

if __name__=="__main__":
	if platform.system() == "Windows":
		cleaner = WindowsCleaner()
		cleaner.kill()
		cleaner.removeInstall()
	elif platform.system() == "Darwin":
		cleaner = MacCleaner()
		cleaner.kill()
		cleaner.findHyvesFiles()
		cleaner.removeInstall()
		cleaner.cleanPreferences()
		cleaner.cleanApplicationData()
	elif platform.system() == "Linux":
		cleaner = LinuxCleaner()
		cleaner.kill()
		cleaner.removeInstall()
	cleaner.restoreHostFile()
