"""Multi-platform uploader for Hyves Desktop installers.

This is valid for all platforms and source control branches. 
"""
import subprocess
import sys
import filecmp
import os
import re
import platform
import md5
from xml.dom import minidom


def executeScpCommand(scp, arguments):
	full_cmd = scp + " " + arguments
	print full_cmd
	proc = subprocess.Popen(full_cmd, shell=True)
	returncode = proc.wait()
	if returncode:
		sys.exit(returncode)

class Uploader:
	"""Uploads the Hyves Desktop installer to the server.
	
	As a new installer was built, it is pushed onto the server. Also, previous
	installer is moved to a different directory, as we should always have : 
	version(new_installer) > version (old_installer).
	That means that we shouldn't upload a installer if it has already been 
	uploaded. (this situation arises if we run this script two times with 
	the same installer version)
	"""

	def __init__(self):

		self.extra_scp_arguments = ""
		self.local_path = os.path.join(os.getcwd(),"build")
		self.user = "www-data"
		self.server = "desktop-builds.dev"
		self.server_kwekker_installer =  None
		self.server_base_installer = None
		self.new_installer_path_server = None
		self.old_installer_path_server = None
		self.new_installer_path_local = None
		self.old_installer_path_local = None
	
	def findSetupFile(self):

		hyves_desktop_version_regex = re.compile("Hyves-Desktop-setup-\d\.\d(?:\.\d)?\." + \
								self.extension)
		count = 0
		setup_file = None	
		names = os.listdir(self.local_path)
		for name in names:
			if hyves_desktop_version_regex.match(name):
				setup_file = os.path.join(self.local_path, name)
				count += 1
		
		if count > 1:
			print "More than one installer files in the directory. Aborting ..."
			sys.exit(1)
		if count == 0:
			print "No installer files in the directory. Aborting ..."
			sys.exit(1)
		return setup_file


	def initializePaths(self):
		
		branch = self.getBranchName()
		if branch == "trunk":
			installer_filename = "Hyves-Desktop-setup-trunk." + self.extension
			original_name = installer_filename
		else:
			installer_filename = "Hyves-Desktop-setup." + self.extension
			original_name = "Hyves-Desktop-setup-" + branch + "." + self.extension
		
		print "original installer name : " + original_name
		self.server_kwekker_installer =  "desktop-builds/kwekker2/" + original_name
		self.server_base_installer = "desktop-builds/" + original_name
		
		self.new_installer_path_local = os.path.join(self.local_path, \
							     installer_filename)
									
		self.new_installer_path_server = "desktop-builds/kwekker2/testing/" + \
									branch + \
									"/versions/latest/" + \
									installer_filename
									
		self.old_installer_path_local = os.path.join(self.local_path, \
									"Hyves-Desktop-old-setup." + self.extension)
									
		self.old_installer_path_server = "desktop-builds/kwekker2/testing/" + \
									branch + \
									"/versions/old/" + \
									installer_filename
		base_path = os.path.split(self.new_installer_path_server)[0]
		
		self.update_xml_server = base_path + "/updates.xml"
		self.update_xml_local = os.path.join(self.local_path, "updates.xml")
								
	def getBranchName(self):
		"""Get the actual branch, e.g. : '1.2' or 'trunk'
		"""
		directories = os.path.split(os.getcwd())
		
		branch_name = directories[1]
		return branch_name
	
	def renameInstaller(self):
		"""Rename file with no version to simplicy further processing.
		
		e.g Hyves-Desktop-setup-1.1.exe -> Hyves-Desktop-setup.exe
		"""
		setup_file = self.findSetupFile()
		os.rename(setup_file, self.new_installer_path_local)

	def uploadToBaseDirectory(self):
		"""Uploads file directly to the home directory 
		
		(old behavior : other scripts may depend on that)
		"""
		full_cmd = "%s %s %s@%s:%s" % (self.extra_scp_arguments,\
									 	self.new_installer_path_local, self.user,\
									 	self.server, self.server_base_installer) 
		
		executeScpCommand(self.cmd, full_cmd)
		full_cmd = "%s %s %s@%s:desktop-builds/symboldumps/" % (self.extra_scp_arguments, os.path.join(self.local_path, "symbolstore*.tgz"), self.user, self.server)
		executeScpCommand(self.cmd, full_cmd)

	
	def downloadPreviousInstaller(self):
		"""Downloads previous installer from the build server.
		
		"""	
		full_cmd = "%s %s@%s:%s %s " % (self.extra_scp_arguments,\
										self.user, self.server,\
										self.new_installer_path_server,\
										self.old_installer_path_local)
		
		executeScpCommand(self.cmd, full_cmd)
	
	def uploadToLatestDirectory(self):
		"""Uploads latest installer to the 'latest' directory on the build server.
		
		"""	
		full_cmd = "%s %s %s@%s:%s" % (self.extra_scp_arguments, \
										self.new_installer_path_local, self.user,\
										self.server, self.new_installer_path_server) 
		executeScpCommand(self.cmd, full_cmd)

	
				
	def uploadToKwekkerDirectory(self):
		"""Uploads file directly to the kwekker2 directory 
		
		(old behavior : other scripts may depend on that)
		"""
		full_cmd = "%s %s %s@%s:%s" % (self.extra_scp_arguments,\
									 	self.new_installer_path_local, self.user,\
									 	self.server, self.server_kwekker_installer) 
		executeScpCommand(self.cmd, full_cmd)
	
	def uploadToOldDirectory(self):
		"""Uploads old installer to the 'old' directory on the build server.
		
		"""	
		full_cmd = "%s %s %s@%s:%s" %(self.extra_scp_arguments,\
										self.old_installer_path_local, self.user,\
										self.server, self.old_installer_path_server)
		
		executeScpCommand(self.cmd, full_cmd)
	
	def downloadXmlUpdateFile(self):

		full_cmd = "%s %s@%s:%s %s " % ( self.extra_scp_arguments,\
										self.user, self.server,\
										self.update_xml_server,\
										self.update_xml_local)
		executeScpCommand(self.cmd, full_cmd)
		
	def revisionFromXml(self):
	
		updates = minidom.parse(self.update_xml_local)
		files = updates.getElementsByTagName("file")
				
		pat = re.compile("Hyves.*?.%s" % self.extension)
		
		for f in files:
			if pat.match(f.childNodes[0].data):
				if f.attributes.has_key("version"):
					return int(f.attributes["version"].value)
		return 0
		
	def updateXml(self,revision):
		"""Updates xml file with attributes of the new installer.

		This sets the md5 and version attributes of the selected entry in the xml file.
		"""
		updates = minidom.parse(self.update_xml_local)
		files = updates.getElementsByTagName("file")		
		pat = re.compile("Hyves.*?.%s" % self.extension)
		#find the selected entry
		for f in files:
			#matches the 'file' tag with the installer file of the specified extension
			if pat.match(f.childNodes[0].data):
				f.attributes["version"] = str(revision)
				new_installer = open(self.new_installer_path_local,'rb')
				f.attributes["md5"] = md5.new(new_installer.read()).hexdigest()
				new_installer.close()

		writer = open(self.update_xml_local,'w')
		updates.writexml(writer)
		writer.close()
		
	def uploadXmlUpdateFile(self):
	
		full_cmd = "%s %s %s@%s:%s" % (self.extra_scp_arguments,\
										self.update_xml_local,\
										self.user, self.server,\
										self.update_xml_server,\
										)
		executeScpCommand(self.cmd, full_cmd)
		
	def previousRevision(self):

		previous_revision = self.revisionFromXml()
		return previous_revision

	def newRevision(self):
		previous_revision = self.previousRevision()		
		print "revision on the server : " + str(previous_revision)
		revision = 0
		output = subprocess.Popen(['svn','info'],stdout=subprocess.PIPE).communicate()[0]
		revision_match = re.search("Revision: (\d+)",output)
		if revision_match:
			revision = int(revision_match.groups()[0])
		
		print "local revision : " + str(revision) 

		if revision > previous_revision:
			return revision
		else:
			return 0

	def upload(self):
		"""Main upload method
		
		"""
		self.initializePaths()
		self.renameInstaller()
		self.uploadToKwekkerDirectory()
		self.uploadToBaseDirectory()
		self.downloadXmlUpdateFile()
		revision = self.newRevision()
		previous_revision = self.previousRevision()
		if revision:
			print "newly built installer (%i) is more recent than the current "\
				"installer(%i), proceeding to move previous installer to "\
				"old directory" % (revision, previous_revision)
			self.updateXml(revision)
			self.uploadXmlUpdateFile()
			
			self.downloadPreviousInstaller()
			self.uploadToOldDirectory()
			self.uploadToLatestDirectory()
		else:
			print "newly built installer (%i) is not more recent than the current "\
				"installer(%i), keeping previous installer" % (revision, previous_revision)


####################################
# Implementations of Uploader interface
####################################
class WindowsUploader(Uploader):
	def __init__(self):
	
		Uploader.__init__(self)
		self.extension = "exe"
		self.cmd = "pscp"
		self.extra_scp_arguments = "-i C:\WORK\.ssh\id_rsa.ppk"
	
class MacUploader(Uploader):
	def __init__(self):
	
		Uploader.__init__(self)
		self.extension = "dmg"
		self.cmd = "scp"

class LinuxUploader(Uploader):
	def __init__(self):
	
		Uploader.__init__(self)	
		self.extension = "bin"
		self.cmd = "scp"


####################################
# main block
####################################
if __name__== "__main__":
	uploader = None
	if platform.system() == 'Windows':
		uploader = WindowsUploader()
	elif  platform.system()== 'Darwin':
		uploader = MacUploader()
	elif platform.system()== 'Linux':
		uploader = LinuxUploader()
	else:
		print  platform.system() + " is not supported for now"
		sys.exit(1)
	uploader.upload()

