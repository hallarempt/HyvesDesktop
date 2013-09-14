"""version module : provides a representation of a Hyves Desktop application version.
"""

class Version:
	"""
	Class comparing and building version objects for Hyves Desktop
	"""
	def __init__(self,major,minor,subminor):
		self.major = int(major)
		self.minor = int(minor)
		self.subminor = int(subminor)

	def __eq__(self, other):
		if self.major != other.major:
			return False
		if self.minor != other.minor:
			return False
		if self.subminor != other.subminor:
			return False
		return True
	
	def __lt__(self, other):
	
		if self.major != other.major:
			return self.major < other.major
		if self.minor != other.minor:
			return self.minor < other.minor
		if self.subminor != other.subminor:
			return self.subminor < other.subminor
		return False
			
	def __str__(self):
		return str(self.major) + "." + str(self.minor) + "." + str(self.subminor)

