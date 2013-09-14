"""Hyves Desktop Updater test module.

Here are defined the various tests applied to the application when trying to
update in different scenarii.
"""
import logging
import os
import os.path
import platform
import time


class Tester:
	def check_autoUpdate_success(self, startApp, mysetup):
		"""Checks update to a newer version.

		Exit code of Hyves Desktop should match INSTALL_EXIT_CODE
		Version should be greater than original version.
		File contents should be changed.
		"""

		mysetup.selectApplication(startApp)
		mysetup.setUp()

		app = mysetup.application()
		assert app
		old_version = app.version()

		result = app.performRegularUpdate()
		assert result == app.INSTALL_EXIT_CODE
		# check the app exited with exit code = install update
		app.waitForUpdateFinished()

		# check the version was updated
		new_version = app.version()
		assert old_version < new_version

		assert mysetup.checkPlatformSpecificPostConditions()


class TesterBasic(Tester):
	# problem with display - connection to X server on Linux
	disabled = platform.system() == 'Linux'
	
	def test_corruptUpdate_doNotInstall(self, mysetup):
		"""Tries to install a corrupt update.

		Version should be unchanged.
		File contents should be unchanged.
		"""

		logging.info("test_corruptUpdate_doNotInstall")
		mysetup.selectApplication("new")
		mysetup.setUp()

		app = mysetup.application()
		assert app
		current_version = app.version()

		app.downloadCorruptInstall(current_version)

		new_version = app.version()
		assert current_version == new_version

	def test_UpdateToLatestInstaller_success(self, mysetup):
		"""Tries to install a regular update (from n-1 to n version)
		"""
		logging.info("test_UpdateToLatestInstaller_success")
		#select last version-1 installer (to be able to make a "real" update)
		self.check_autoUpdate_success("old", mysetup)


class TesterRelease(Tester):
	# linux updater was broken for release 1.0 and 1.1 so don't try to test it 
	disabled = platform.system() == 'Linux'	

	def test_releaseApp_1_0_autoUpdate_success(self,mysetup):
		"""Tries to update from release 1.0 to current version.
		"""
		logging.info("test_releaseApp_1.0_AutoUpdate_success");
		self.check_autoUpdate_success("release_1.0", mysetup)

	def test_releaseApp_1_1_autoUpdate_success(self,mysetup):
		"""Tries to update from release 1.1 to current version.
		"""
		logging.info("test_releaseApp_1.1_AutoUpdate_success");
		self.check_autoUpdate_success("release_1.1", mysetup)

	def test_releaseApp_1_2_autoUpdate_success(self,mysetup):
		"""Tries to update from release 1.2 to current version.
		"""
		logging.info("test_releaseApp_1.2_AutoUpdate_success");
		self.check_autoUpdate_success("release_1.2", mysetup)

# uncomment when we have a valid release on the server (in var/www/nginx-default/kwekker2/testing/release/1.3)
#	def test_releaseApp_1_3_autoUpdate_success(self,mysetup):
#		"""Tries to update from release 1.3 to current version.
#		"""
#		logging.info("test_releaseApp_1.3_AutoUpdate_success");
#		self.check_autoUpdate_success("release_1.3", mysetup)


class TesterWindows(Tester):
	disabled = platform.system() != 'Windows'

	def test_uninstall_allRemoved(self, mysetup):
		"""Simple uninstall of a current install

		Uninstall should be clean, i.e:

		- directory binaries and bundle directories should have been removed
		- on windows, shortcuts have been removed
		"""
		logging.info("test_uninstall_allRemoved")
		mysetup.selectApplication("new")
		mysetup.setUp()

		mysetup.uninstall()
		time.sleep(10)
		assert mysetup.is_uninstall_clean()

	def test_updateCustomLocation_correctLocation(self, mysetup):
		"""Checks that updates get installed in the same location as the previous version
		(not in a default directory).

		"""
		logging.info("test_updateCustomLocation_correctLocation")
		mysetup.selectApplication("old")
		mysetup.setUp(r"C:\WORK\testing")
		app = mysetup.application()
		assert app
		old_version = app.version()

		result = app.performRegularUpdate()
		assert result == app.INSTALL_EXIT_CODE
		app.waitForUpdateFinished()
		new_version = app.version()
		
		# this checks that the original app (in the test folder) got updated
		# and not a default directory (C:\Program Files\Hyves Desktop)
		# as in the earlier hyves desktop versions
		assert not os.path.exists(mysetup.defaultDirectory())
		# important step, to clean out registry so we can start clean again
		mysetup.uninstall()

class TesterMac(Tester):
	disabled = platform.system() != 'Darwin'

	def test_changeVolumeName_updateSuccessful(self, mysetup):
		"""This tests that no matter how the Volume is named, the updater will succeed.

		"""
		os.rename("/Volumes/Macintosh HD","/Volumes/\\")
		try:
			self.check_autoUpdate_success("old", mysetup)
		finally:
			"""in any case we need to rename the Volume as it was before"""
			os.rename("/Volumes/\\","/Volumes/Macintosh HD")
