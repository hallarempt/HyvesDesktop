/*
 * Hyves Desktop, Copyright (C) 2008-2009 Hyves (Startphone Ltd.)
 * http://www.hyves.nl/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  US
 */

#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QProcess>

#include "hyveslib_export.h"

class QTemporaryFile;

namespace Updater {

/**
 * This exit code is returned when the application should install a new update
 */
const int InstallUpdateExitCode = 57;

/**
 * An updater object is created on startup. The updater checks every day
 * whether a new version of the HyvesDesktop has been created and downloads
 * updates in the background. Upon application startup, it checks for ready
 * to be installed updates, and installs them if available.
 */
class HYVESLIB_EXPORT Updater : public QObject {
	
	Q_OBJECT
	
	public:
		static Updater *instance();
		static void destroy();
		
		/**
		 * Returns whether updates should be forced upon the user. If
		 * this is the case, the user should not be asked for
		 * confirmation before updating.
		 *
		 * Updates are forced when HQ updates or silent updates are
		 * enabled.
		 */
		bool forceUpdates() const;
		
	public slots:
		/**
		 * Sets whether to perform silent updates.
		 *
		 * Right now this means there will be no popup asking whether to
		 * update, but the update will be started immediately.
		 * Installation of the update will also be performed in silent
		 * mode.
		 */
		void setUseSilentUpdate(bool useSilentUpdate);
		
		/**
		 * Overrides the path for update files in the server. 
		 * May be used to serve different updates (testing).
		 * @param path the relative path (from server root) to the update files.
		 */
		void setServerUpdatePath(const QString &path);
		
		/**
		 * Sets whether the HQ update should be downloaded. If this is
		 * true, an experimental build only meant for people at Hyves HQ
		 * will be downloaded and installed. Installation will be forced
		 * for those users.
		 *
		 * If HQ updates are enabled, setDownloadPreUpdate() will have
		 * no effect.
		 */
		void setDownloadHqUpdate(bool downloadHqUpdate);
		
		/**
		 * Sets whether the pre-update should be downloaded. The
		 * pre-update is an update which is available to a small
		 * percentage of users to test whether no big problems arise
		 * before pushing the update to all users.
		 *
		 * This method has no effect when HQ updates are enabled.
		 */
		void setDownloadPreUpdate(bool downloadPreUpdate);
		
		/**
		 * Starts checking for new updates.
		 *
		 * By default, on application will check for updates on startup.
		 */
		void checkForUpdates();
		
		/**
		 * Installs an available update.
		 */
		void installAvailableUpdate();
		
	signals:
		/**
		 * Emitted when the Updater starts checking for new updates.
		 */
		void checkingForUpdates();
		
		/**
		 * Emitted when the Updater starts downloading an update.
		 */
		void downloadingUpdate();
		
		/**
		 * Emitted when a new update is ready for installation.
		 */
		void updateAvailable();
		
		/**
		 * Emitted if no update is available.
		 */
		void noUpdateAvailable();
		
		/**
		 * Emitted when the update could not be downloaded.
		 */
		void errorDownloadingUpdate();
		
	private:
		static Updater *s_instance;
		
		struct Private;
		Private *const m_d;
		
		Updater();
		virtual ~Updater();
		
		friend class UpdaterTest;
		
		struct Update;
		
		void checkForUpdatesInternal();
		bool verifyReply(const QString &reply, Update *update);
		void fetchUpdate(const Update &update);
		
	private slots:
		void updateCheckResult(const QString &reply);
		void updateDownloaded(QTemporaryFile *temporaryFile);
		void processError(QProcess::ProcessError error);
		void errorDownloading();
};

} // namespace Updater

#endif // UPDATER_H
