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
 
#ifndef GROWLNOTIFIER_H
#define GROWLNOTIFIER_H
 
#include <QString>
#include <QStringList> 

namespace WindowManager {

struct NotificationAction;

/**
 * A class for emitting a clicked signal to Qt.
 *
 * "forwards" the signal received by Growl to Qt.
 */
class GrowlNotificationBridgeSignaler : public QObject {
	
	Q_OBJECT
	
	public:
		GrowlNotificationBridgeSignaler() { }
		
		void emitNotificationClicked(NotificationAction *action) {
			
			emit notificationClicked(action);
		}

		void emitNotificationTimedOut(NotificationAction *action) {
			
			emit notificationTimedOut(action);
		}
		
	signals:
		void notificationClicked(NotificationAction *action);
		void notificationTimedOut(NotificationAction *action);

};

/**
 * A simple interface to Growl API.
 */
class GrowlNotificationBridge {
	
	public:
		/**
		 * Constructs a GrowlNotificationBridge.
		 *
		 * @param notifications The list names of all notifications that
		 *                      can be sent by this notifier.
		 * @param defaultNotifications The list of names of the
		 *                             notifications that should be
		 *                             enabled by default.
		 * @param iconPath The path to the icon file that we want to
		 *                 display with the notifications.
		 * @param appName The name of the application under which the
		 *                notifier should register with growl.
		 */
		GrowlNotificationBridge(const QStringList &notifications,
					const QStringList &defaultNotifications,
					const QString &iconPath,
					const QString &appName = QString::null);
		~GrowlNotificationBridge();
		
		/**
		 * Sends a notification to Growl.
		 *
		 * @param name The registered name of the notification.
		 * @param title The title for the notification.
		 * @param description The description of the notification.
         * @param mediaId the media id for the image we want to display 
		 *                with the notification
		 * @param icon The icon of the notification.
		 * @param sticky Whether the notification should be sticky (i.e.
		 *               require a click to discard.
		 * @param receiver The receiving object which will be signaled
		 *                 when the notification is clicked.
		 * @param clickedSlot The slot to be signaled when the notification is
		 *             clicked.
		 * @param timedOutSlot The slot to be signaled when the notification is
		 *             timed out.
		 * @param action The action which will be passed back to the
		 *               slot.
		 */
		void notify(const QString &name, const QString &title, const QString &description,
					const QString &mediaId, const QObject *receiver = 0, 
					const char *clikedSlot = 0, const char* timedOutSlot = 0, 
					NotificationAction *action = 0);
		
		/**
		 * Is Growl installed on the user machine?
		 */
		static bool isGrowlInstalled();
		
		/**
		 * Is Growl currently running on the user machine?
		 */
		static bool isGrowlRunning();
		
	private:
		struct Private;
		Private *const m_d;
};

/** 
 * Interface between our application and Growl.
 *
 * This class takes care of presenting a simple interface for the window manager
 * to request notification showing. 
 */
class GrowlNotifier : public QObject {
	
	Q_OBJECT
	
	public:
		GrowlNotifier();
		~GrowlNotifier();
		
		/**
		 * Indicates if the growl notifier is ready for duty.
		 *
		 * @return A boolean indicating whether we should proceed to use
		 *         growl for posting the current message.
		 */
		bool isOperational();
		
		/**
		 * Requests Growl to show a notification to the user.
		 *
		 * @param htmlTextContent The text (in html or plain text
		 *                        format) to be displayed to the user.
		 * @param method The callback method to call if the notification
		 *               is clicked.
		 * @param parameter Argument to pass to the callback method if
		 *                  the notification is clicked.
		 * @param title The title of the message.
	  
		 * @param mediaId the media id for the image we want to display 
		 *                with the notification
         * @param mediaSecret the media secret for the image we want to display 
		 *                with the notification
		 */
		void showNotification(const QString &htmlTextContent, const QString &method,
							  const QString &parameter, const QString &title,
							  const QString& mediaId, const QString& mediaSecret);
		
 	private slots:
		/**
		 * Unpacks the callback action and proceeds to call method with
		 * parameters defined in the action.
		 *
		 * @param action contains the action to execute
		 */
		void triggerCallbackAction(NotificationAction *action);

		/**
		 * Cleanup once the notification has timed out.
		 *
		 * @param action we need to cleanup
		 */
		void cleanup(NotificationAction *action);
		
	private:
		GrowlNotificationBridge *m_notificationBridge;
		
		/**
		 * Initializes the notification system (for e.g, growl was
		 * started after the application was started).
		 */
		void start();
		
		/**
		 * Stops the notification system (for e.g, growl was stopped
		 * after the application was started).
		 */
		void stop();
};

} // namespace WindowManager

#endif // GROWLNOTIFIER_H
