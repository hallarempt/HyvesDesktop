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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QByteArray>
#include <QMainWindow>
#include <QTemporaryFile>
#include <QUrl>
#include <QVariantMap>

#include "windowmanager/MainWindowBase.h"
#include "windowmanager/WindowBase.h"
#include "NetworkReply.h"

namespace Kwekker {

class MainWindow : public WindowManager::MainWindowBase {
	
	Q_OBJECT
	
	public:
		MainWindow();
		virtual ~MainWindow();
		
		QWebFrame *mainFrame() const;
		
	public slots:
		int windowId() const;
		
		/**
		 * Returns whether the news frame is visible to the user.
		 *
		 * This is used by JavaScript to determine whether frames should be
		 * reloaded.
		 */
		bool isNewsFrameVisible() const;
		
		/**
		 * Loads one of the buzz components into the latest news frame.
		 */
		void loadNewsComponent(QString component = QString::null);
		
		/**
		 * Sets a cursor that should be used for the window.
		 * 
		 * @param cursor "wait" or "default".
		 */
		void setCursor(const QString &cursor);
		
		void saveConfig() const;
		void loadConfig();
		
		void loggedIn();
		void loggedOut();
		void loggedInFromOtherLocation();
		
		void signOut();
		
		/**
		 * Resets the amount of seconds after which we're allowed to
		 * reconnect. Resetting should only be done upon a fully
		 * successful login.
		 */
		void resetReconnectSeconds();
		
		/**
		 * Returns the amount of seconds after which we may try to
		 * automatically reconnect.
		 *
		 * This method automatically increments the amount of seconds
		 * for subsequent reconnect attempts.
		 *
		 * @return Number of seconds after which to reconnect, or 0 if
		 *         no automatic reconnect is allowed.
		 */
		int reconnectSeconds();
		
		/**
		 * Reloads all contents in the window, including the roster list
		 * and the latest news section. Will also trigger a reconnect to
		 * the chat server.
		 */
		void reloadKwekker();
		
		void settings();
		
		/**
		 * Returns the number of notifications waiting for the user.
		 */
		int numNotifications() const;
		
		/**
		 * Sets the number of new notifications.
		 */
		void setNumNotifications(int numNotifications);
		
		/**
		 * Hides or displays the notification view.
		 */
		void toggleNotificationView();
		
		/**
		 * Launches the photo uploader plugin.
		 */
		void launchPhotoUploader();
		
		/**
		 * Opens the WWW dialog.
		 */
		void newWwwClicked();
		
		/**
		 * Launches the photo uploader plugin.
		 */
		void uploadPhotosClicked();
		
		/**
		 * Redirects to the website for adding other content.
		 */
		void newOtherClicked();
		
		/**
		 * Copies fake buzz HTML file from JavaScript. Used for testing
		 * only.
		 *
		 * @param dataFileName Origin file to set as buzz webpage (in
		 *                     the bundle/static/test directory).
		 */
		void setFakeBuzzHtml(const QString &dataFileName);
		
		/**
		 * Removes fake buzz HTML file. Used for testing
		 * only.
		 */
		void clearFakeBuzzHtml();
		
	protected:
		virtual void closeEvent(QCloseEvent *event);
		virtual void resizeEvent(QResizeEvent *event);
		virtual void showEvent(QShowEvent *event);
		void changeEvent(QEvent* event);
		bool eventFilter(QObject *obj, QEvent *event);
		
	private slots:
		void eventRaised(const QVariantMap &event);
		
		void setFrameDecorationGeometry();
		
		void webViewLoadStarted();
		void webViewLoadFinished(bool ok);
		
		void newsLinkClicked(const QUrl &url);
		
		void splitterDoubleClicked();
		
		void addNewsWebView();
		void removeNewsWebView();
			
		void testsFinished();
		
		/**
		 * Custom URL handler for testing the Buzz view.
		 *
		 * @param request Request that is going to be handled.
		 * @return network Reply after processing.
		 */
		NetworkReply *handleBuzzRequest(const QNetworkRequest &request);
		
	private:
		struct Private;
		Private *const m_d;
		
		void saveWindowState() const;
		void restoreWindowState();
};

} // namespace Kwekker

#endif // MAINWINDOW_H
