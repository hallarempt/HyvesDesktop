"""Utility module for Hyves Desktop testing.

"""
import os
import time
import shutil

import logging

class FileTimestampHierarchy:
	"""Keeps relationshipe between file names and timestamps
	"""
	def __init__(self):
		#dictionary containing pairs [name-timestamp] of manipulated files
		self.filesAndTimestamps = {}  
	
	def mapping(self):
		return self.filesAndTimestamps	
		
	def insert(self, path, time):
		"""Inserts a new item in the hierarchy
		"""
		self.filesAndTimestamps[path] = time


class FileTimestampManager:
	"""Performs file timestamps manipulations (retrieval, modification ) on file hierarchies.
	"""
	def __init__(self,path):
		
		#base path from which we want to manipulate timestamps
		self.path = path
		#self.fileTimestampHierarchy = FileTimestampHierarchy()
		
	def retrieve(self):
		"""Retrieves file timestamps from the file system.
		"""
		#first erase the previous hierarchy
		fileTimestampHierarchy = FileTimestampHierarchy()
		for root, dirs, files in os.walk(self.path):
			for f in files:
				epoch = os.path.getmtime(os.path.join(root,f))
				mtime = time.gmtime(epoch)
				fileTimestampHierarchy.insert(os.path.join(root,f), time.strftime('%d %m %Y', mtime))
		return fileTimestampHierarchy

	def replace(self,timestamp):
		"""Replace all timestamps in the hierarchy with one timestamp.
		
		timestamp: The timestamp going to be assigned to all files.
		"""
		#the modified date (with utime) is set to (time requested - 1 hour) by Windows !
		#set the time after midnight so we don't get pushed back one day before ...
		fileTimestampHierarchy = self.retrieve()
		timeofday= '8'
		old_time = time.strptime(timestamp + " " + timeofday, '%d %m %Y %H')
		mapping = fileTimestampHierarchy.mapping()
		for filename, timestamp in mapping.iteritems():
			os.utime(filename,(time.mktime(old_time),time.mktime(old_time)))
	
	def clearedContents(self,old_timestamped_hierarchy, new_timestamped_hierarchy):

		old_timestamps = old_timestamped_hierarchy.mapping()
		new_timestamps = new_timestamped_hierarchy.mapping()
		for filename, timestamp in old_timestamps.iteritems():
			if filename in new_timestamps:
				if new_timestamps[filename] == timestamp:
					logging.info("clearedContents : timestamp didn't change for file : " + 
						filename)
					return False
		return True

class SystemFileModifier:
	"""Modifies the host system for testing (and reverts it to original state)
	"""
	def __init__(self,hostfile_name, hostfile_backup_name, download_server):
		self.hostfile_name = hostfile_name
		self.hostfile_backup_name = hostfile_backup_name
		self.download_server = download_server
		self.modified = False
		
	def modifyHostFile(self):
		"""Modify the host file to redirect download server to the test server
		"""
		#save original hosts file
		logging.info("save original host file")
		shutil.copyfile(self.hostfile_name, self.hostfile_backup_name)
		#modify host file to access our custom download server
		hostfile = open(self.hostfile_name, 'a')
		hostfile.write(self.download_server + "\n")
		hostfile.close()
		self.modified = True
	
	def revertHostFile(self):
		"""Reverts host file to the original one.
		"""
		if self.modified:
			logging.info("revert original host file")
			shutil.copyfile( self.hostfile_backup_name, self.hostfile_name)
			self.modified = False
