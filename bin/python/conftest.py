import sys
import platform

import py

def pytest_funcarg__mysetup(request):
	from hyvesdesktop import setup
	if platform.system() == "Windows":
		return setup.WindowsSetup(request)
	elif platform.system() == "Darwin":
		return 	setup.MacSetup(request)
	elif platform.system() == "Linux":
		return setup.LinuxSetup(request)
	else:
		sys.exit(1)

def pytest_addoption(parser):
	parser.addoption("--branch", action="store", default=None, help="specify wich branch to test like 'trunk' or 'release' (default: name of the current working directory)")
