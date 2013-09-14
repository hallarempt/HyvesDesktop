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

#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QObject>
#include <QVariant>
#include <QWebFrame>

#include "WindowInfo.h"
#include "hyveslib_export.h"

class QWebPage;
class QWidget;

namespace WindowManager {

class WindowBase;

/**
 * The WindowManager class registers all windows and allows scripts to easily
 * access them.
 */
class HYVESLIB_EXPORT WindowManager : public QObject {
	
	Q_OBJECT
	
	public:
		/**
		 * Returns the global WindowManager instance.
		 */
		static WindowManager *instance();
		
		/**
		 * Destroys the global WindowManager instance.
		 */
		static void destroy();
		
		/**
		 * Sets the window ID of the window that should be referenced under
		 * window.opener for newly created windows.
		 */
		void setOpenerWindowId(int windowId);
		
		/**
		 * Informs the WindowManager of the size of the frame decoration.
		 *
		 * @param titlebarHeight Height of the titlebar.
		 * @param framewidth Width of the frame.
		 * @param frameHeight Height of the frame.
		 */
		void setFrameDecorationGeometry(int titleBarHeight, int frameWidth, int frameHeight);
		
		/**
		 * Registers a window to the WindowManager.
		 *
		 * @param window The window to register.
		 * @return The ID that has been assigned to the WindowManager.
		 */
		int registerWindow(WindowBase *window);
		
		/**
		 * Unregisters a registered window from the WindowManager.
		 *
		 * @param windowId ID of the window to unregister.
		 * @return @c true on success, @c false on failure.
		 */
		bool unregisterWindow(int windowId);
		
		/**
		 * Returns a registered window.
		 *
		 * @param windowId ID of the window to return.
		 * @return The Window object. May be a null pointer if the window is not
		 *         registered.
		 */
		QWidget *window(int windowId) const;
		
		/**
		 * Returns a list of all active windows
		 */
		QList<QWidget*> windows() const;
		
		/**
		 * Returns window information about a window.
		 *
		 * @param windowId ID of the window to get information about.
		 * @return The WindowInfo object. If the window with the specified ID
		 *         was not found, an invalid object with ID 0 is returned.
		 */
		WindowInfo windowInfo(int windowId) const;
		
		/**
		 * Raises an event which can be caught by all windows.
		 *
		 * @param event Information about the event. The event map is supposed
		 *              to have at least a name property which identifies the
		 *              type of event.
		 */
		void raiseEvent(const QVariantMap &event);
		
		/**
		 * Returns the frame associated with windowId
		 *
		 * @param windowId ID of the window associated with the frame.
		 * @return frame
		 */
		QWebFrame *frame(int windowId);
	
		/**
		 * Displays a confirmation dialog, asking the user whether they
		 * really want to perform some action. The dialog will be
		 * populated with default Yes/No buttons.
		 *
		 * @param parent Parent window that pops up the question, use 0
		 *               for a parentless dialog.
		 * @param title Title of the dialog. If this is an empty string,
		 *              the application name will be used as title.
		 * @param question The question which should be prompted to the
		 *                 user.
		 * @return Boolean indicating whether the user confirmed the
		 *         the question.
		 *
		 * @note During test runs, all dialogs all confirmed by default.
		 */
		bool askConfirmation(QWidget *parent, const QString &title, const QString &question);
		
		/**
		 * Displays an information dialog. The dialog will be populated
		 * with a default Ok button.
		 *
		 * @param parent Parent window that pops up the message, use 0
		 *               for a parentless dialog.
		 * @param title Title of the dialog. If this is an empty string,
		 *              the application name will be used as title.
		 * @param message The message that should be displayed to the
		 *                user.
		 *
		 * @note During test runs, no dialog is shown.
		 */
		void showMessage(QWidget *parent, const QString &title, const QString &message);
		
	public slots:
		/**
		 * Returns the height of the title bar on top of decorated windows.
		 */
		int titleBarHeight() const;
		
		/**
		 * Returns the width of the frame around decorated windows.
		 */
		int frameWidth() const;
		
		/**
		 * Returns the height of the frame on the bottom of decorated windows.
		 */
		int frameHeight() const;
		
		/**
		 * Returns the window information about a window in a JSON string.
		 *
		 * @sa windowInfo();
		 */
		QString windowInfoJSON(int windowId) const;
		
		/**
		 * Creates a new window.
		 *
		 * @param windowClass The type of window to create.
		 * @param arguments Comma-seperated list of arguments.
		 * @return ID of the newly created window. Can be 0 if the window was
		 *         not successfully created.
		 */
		int createWindow(QString windowClass, QString arguments);
		
		/**
		 * Sends a command to execute to a window.
		 *
		 * @param windowId ID of the window to execute the command in.
		 * @param command The command to execute.
		 * @param arguments Arguments for the command.
		 *
		 * A list of commands and accepted arguments follows:
		 *
		 * <dl>
		 *   <dt>bringToFront</dt>
		 *   <dd>Brings the window to the front and gives it focus.</dd>
		 *   <dt>close</dt>
		 *   <dd>Closes the window.</dd>
		 *   <dt>flash(time)</dt>
		 *   <dd>Flashes the taskbar entry of the window for @p time seconds.</dd>
		 *   <dt>navigate(url)</dt>
		 *   <dd>Directs the window to navigate to the given @p url, which may be
		 *     absolute relative to the Hyves root.</dd>
		 *   <dt>update(caption,height,left,right,visible,width)</dt>
		 *   <dd>Updates any one of the given properties of the window.
		 *     Set caption to change the window title. Left, right, width and
		 *     height control the geometry of the window. Set visible to either
		 *     "true" or "false" to show or hide the window, respectively.</dd>
		 * </dl>
		 *
		 * @return @c true if the command was successfully sent to the window,
		 *         @c false otherwise. Note that a return value of @c true does
		 *         not indicate whether the window actually did anything with
		 *         the command.
		 */
		bool windowExecute(int windowId, QString command, QString arguments = QString::null);
		
		/**
		 * Closes the window with the given window ID.
		 */
		void closeWindow(int windowId);
		
		/**
		 * Closes all windows, except the main window.
		 */
		void closeAllWindows();
		
		/**
		 * Raises an event which can be caught by all windows.
		 *
		 * @param eventJSON Information about the event, encoded in JSON. The
		 *                  event is supposed to have at least a name property
		 *                  which identifies the type of event.
		 */
		void raiseEvent(const QString &eventJSON);
		
		/**
		 * Opens a URL in the user's preferred browser.
		 *
		 * @param url The URL to open.
		 */
		void openUrl(const QString &url);
		
		/**
		 * Opens a notification slider.
		 * 
		 * @param htmlContent HTML content to display in the slider.
		 * @param textContent Alternative text content to display in the
		 *                    slider (for text-only systems).
		 * @param title Title of the notification.
		 * @param method Callback method we want to execute on cliking
		 *               the notification.
		 * @param paramater Parameter to give to the callback method.
		 * @param mediaId media id for the picture we want to display
		 * @param mediaSecret media secret for the picture we want to display
		 */
		void showSlider(const QString &htmlContent, const QString &textContent, const QString &title,
		                const QString &method, const QString &parameter,
				const QString& mediaId, const QString& mediaSecret);
		
		/**
		 * Closes the slider with the given index.
		 * 
		 * @param index Index of the slider to close.
		 */
		void closeSlider(int index);
		
		/**
		 * Displays a confirmation dialog, asking the user whether they
		 * really want to perform some action. The dialog will be
		 * populated with default Yes/No buttons.
		 *
		 * @param windowId ID of the window that pops up the question,
		 *                 use 0 for a parentless dialog.
		 * @param title Title of the dialog. If this is an empty string,
		 *              the application name will be used as title.
		 * @param question The question which should be prompted to the
		 *                 user.
		 * @return Boolean indicating whether the user confirmed the
		 *         the question.
		 *
		 * @note During test runs, all dialogs all confirmed by default.
		 */
		bool askConfirmation(int windowId, const QString &title, const QString &question);
		
		/**
		 * Displays an information dialog. The dialog will be populated
		 * with a default Ok button.
		 *
		 * @param windowId ID of the window that pops up the message,
		 *                 use 0 for a parentless dialog.
		 * @param title Title of the dialog. If this is an empty string,
		 *              the application name will be used as title.
		 * @param message The message that should be displayed to the
		 *                user.
		 *
		 * @note During test runs, no dialog is shown.
		 */
		void showMessage(int windowId, const QString &title, const QString &message);
		
	signals:
		/**
		 * Emitted when a new event is raised through raiseEvent().
		 * 
		 * @param event Information about the event. The event map is supposed
		 *              to have at least a name property which identifies the
		 *              type of event.
		 */
		void eventRaised(const QVariantMap &event);
		
		/**
		 * Emitted whenever a window that was created using createWindow() has
		 * finished loading its web contents.
		 */
		void windowLoadFinished(int windowId);
		
		/**
		 * Emitted whenever a window is created.
		 */
		void windowCreated(int windowId);
		
		/**
		 * Emitted whenever a window is closed. Note that the window may have been
		 * deleted when you receive this signal.
		 */
		void windowClosed(int windowId);
		
	private:
		static WindowManager *s_instance;
		
		struct Private;
		Private *const m_d;
		 /**
		 * Opens a notification slider with html content.
		 * 
		 * @param htmlContent HTML content to display in the slider.
		 */
		void showHtmlSlider(const QString &htmlContent);
	

		WindowManager();
		virtual ~WindowManager();
};

} // namespace WindowManager

#endif // WINDOWMANAGER_H
