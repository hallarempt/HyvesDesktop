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

extern "C" {

#include <CoreFoundation/CoreFoundation.h>
#include <Growl/Growl.h>

}
 
#include <QStringList>
#include <QPixmap>
#include <QBuffer>
#include <QTextDocument>

#include "config.h"
#include "GrowlNotifier.h"
#include "WindowManager.h"
#include "mediacache/MediaCache.h"

namespace WindowManager {

//------------------------------------------------------------------------------
/**
 * Utility functions to deal with Growl API
 */
//------------------------------------------------------------------------------

/**
 * Converts a QString to a CoreFoundation string, preserving Unicode.
 *
 * @param string The string to be converted.
 * @return A reference to a CoreFoundation string.
 */
static CFStringRef qString2CFString(const QString &string) {

	if (string.isNull()) {
		return 0;
	}
	
	ushort *buffer = new ushort[string.length()];
	for (int i = 0; i < string.length(); ++i) {
		buffer[i] = string[i].unicode();
	}
	CFStringRef result = CFStringCreateWithBytes(NULL, (UInt8 *) buffer,
						     string.length() * sizeof(ushort),
						     kCFStringEncodingUnicode, false);
	
	delete [] buffer;
	return result;
}

/**
 * Retrieves the values from the context.
 *
 * @param context the context
 * @param signaler the signaler object that sent the signal
 * @param receiver the receiving object which will be signaled when the
 *  notification is clicked. May be NULL.
 * @param clickedSlot the slot to be signaled when the notification is clicked.
 * @param timedOutSlot the slot to be signaled when the notification is timed out.
 * @param action the context which will be passed back to the slot
 *  May be NULL.
 */
void getContext(CFPropertyListRef context, 
                GrowlNotificationBridgeSignaler **signaler, 
                const QObject **receiver, 
                const char **clickedSlot,
		const char **timedOutSlot,
                NotificationAction **action) {
	
	CFDataRef data;
	
	if (signaler) {
		data = (CFDataRef) CFArrayGetValueAtIndex((CFArrayRef) context, 0);
		CFDataGetBytes(data, CFRangeMake(0,CFDataGetLength(data)), (UInt8 *) signaler);
	}
	
	if (receiver){
		data = (CFDataRef) CFArrayGetValueAtIndex((CFArrayRef) context, 1);
		CFDataGetBytes(data, CFRangeMake(0,CFDataGetLength(data)), (UInt8 *) receiver);
	}
	
	if (clickedSlot) {
		data = (CFDataRef) CFArrayGetValueAtIndex((CFArrayRef) context, 2);
		CFDataGetBytes(data, CFRangeMake(0,CFDataGetLength(data)), (UInt8 *) clickedSlot);
	}

	if (timedOutSlot) {
		data = (CFDataRef) CFArrayGetValueAtIndex((CFArrayRef) context, 3);
		CFDataGetBytes(data, CFRangeMake(0,CFDataGetLength(data)), (UInt8 *) timedOutSlot);
	}

	if (action) {
		data = (CFDataRef) CFArrayGetValueAtIndex((CFArrayRef) context, 4);
		CFDataGetBytes(data, CFRangeMake(0,CFDataGetLength(data)), (UInt8 *) action);
	}
}

//------------------------------------------------------------------------------
/**
 * The callback function, used by Growl to notify that a notification was
 * clicked.
 *
 * @param context The context of the notification.
 */
void growlNotificationClicked(CFPropertyListRef context) {
	
	GrowlNotificationBridgeSignaler *signaler;
	const QObject *receiver;
	const char *slot;
	NotificationAction *action;
	
	getContext(context, &signaler, &receiver, &slot, 0, &action);
	
	QObject::connect(signaler, SIGNAL(notificationClicked(NotificationAction *)), receiver, slot);
	signaler->emitNotificationClicked(action);
	QObject::disconnect(signaler, SIGNAL(notificationClicked(NotificationAction *)), receiver, slot);
}

 
/**
 * The callback function, used by Growl to notify that a notification was
 * timed out.
 *
 * @param context The context of the notification.
 */
void growlNotificationTimedOut(CFPropertyListRef context) {
	
	GrowlNotificationBridgeSignaler *signaler;
	const QObject *receiver;
	const char *slot;
	NotificationAction *action;
	
	getContext(context, &signaler, &receiver, 0, &slot, &action);
	
	QObject::connect(signaler, SIGNAL(notificationTimedOut(NotificationAction *)), receiver, slot);
	signaler->emitNotificationTimedOut(action);
	QObject::disconnect(signaler, SIGNAL(notificationTimedOut(NotificationAction *)), receiver, slot);
}

/**
 * Creates a context for a notification, which will be sent back by Growl
 * when a notification is clicked.
 *
 * @param receiver the receiving object which will be signaled when the
 *  notification is clicked. May be NULL.
 * @param clickedSlot the slot to be signaled when the notification is clicked.
 * @param timedOutSlot the slot to be signaled when the notification is timed out.
 * @param action the action which will be passed back to the slot
 *  May be NULL.
 * @return the context
 */
CFPropertyListRef createContext(GrowlNotificationBridgeSignaler *signaler, 
                                const QObject *receiver, 
                                const char *clickedSlot, 
                                const char *timedOutSlot, 
                                NotificationAction *action) {
	
	CFDataRef context[5];
	context[0] = CFDataCreate(kCFAllocatorDefault, (const UInt8 *) &signaler, sizeof(GrowlNotificationBridgeSignaler *));
	context[1] = CFDataCreate(kCFAllocatorDefault, (const UInt8 *) &receiver, sizeof(const QObject *));
	context[2] = CFDataCreate(kCFAllocatorDefault, (const UInt8 *) &clickedSlot, sizeof(const char *));
	context[3] = CFDataCreate(kCFAllocatorDefault, (const UInt8 *) &timedOutSlot, sizeof(const char *));
	context[4] = CFDataCreate(kCFAllocatorDefault, (const UInt8 *) &action, sizeof(NotificationAction *));
	
	CFArrayRef array = CFArrayCreate(kCFAllocatorDefault,
				    (const void **) context, 5, &kCFTypeArrayCallBacks);
	
	CFRelease(context[0]);
	CFRelease(context[1]);
	CFRelease(context[2]);
	CFRelease(context[3]);
	CFRelease(context[4]);
	
	return array;
}

//------------------------------------------------------------------------------
 
struct GrowlNotificationBridge::Private {
	
	struct Growl_Delegate delegate;
	CFDataRef icon;
	GrowlNotificationBridgeSignaler *signaler;
	
	Private(const QStringList &notifications,
			const QStringList &default_notifications,
			const QString &iconPath,
			const QString &appName) {
		
		// Initialize signaler
		signaler = new GrowlNotificationBridgeSignaler();
		
		setupNotifications(notifications, default_notifications, appName);
		
		initApplicationIcon(iconPath);
		
		// Register with Growl
		Growl_SetDelegate(&delegate);
	}
	
	~Private() {
		
		if (icon) {
			CFRelease(icon);
		}
	}
	void setupNotifications(const QStringList &notifications, 
			  const QStringList &default_notifications, 
			  const QString &appName ) {
		
		// All Notifications
		QStringList::ConstIterator it;
		CFMutableArrayRef allNotifications = CFArrayCreateMutable(kCFAllocatorDefault, 0, 
																  &kCFTypeArrayCallBacks);
		
		for (it = notifications.begin(); it != notifications.end(); ++it) {
			CFArrayAppendValue(allNotifications, qString2CFString(*it));
		}
		
		// Default Notifications
		CFMutableArrayRef defaultNotifications = CFArrayCreateMutable(
				kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
		
		for (it = default_notifications.begin(); it != default_notifications.end(); ++it) {
			CFArrayAppendValue(defaultNotifications, qString2CFString(*it));
		}  
		
		InitGrowlDelegate(&delegate);
		if (!appName.isEmpty()) {
			delegate.applicationName = qString2CFString(appName);
		}
		
		CFTypeRef keys[] = { GROWL_NOTIFICATIONS_ALL, GROWL_NOTIFICATIONS_DEFAULT };
		CFTypeRef values[] = { allNotifications, defaultNotifications };
		delegate.registrationDictionary = CFDictionaryCreate(kCFAllocatorDefault, keys,
															 values, 2,
															 &kCFTypeDictionaryKeyCallBacks,
															 &kCFTypeDictionaryValueCallBacks);
		
		delegate.growlNotificationWasClicked = &growlNotificationClicked;
		delegate.growlNotificationTimedOut = &growlNotificationTimedOut;

	}
	
	void initApplicationIcon(const QString &iconPath) {
		
		icon = 0;
		
		QPixmap pixmap(iconPath);
		if (!pixmap.isNull()) {
			QByteArray imageData;
			QBuffer buffer(&imageData);
			buffer.open(QIODevice::WriteOnly);
			pixmap.save(&buffer, "PNG");
			icon = CFDataCreate(0, (UInt8 *) imageData.data(), imageData.size());
		}
		
		delegate.applicationIconData = icon;
	}
};

GrowlNotificationBridge::GrowlNotificationBridge(const QStringList &notifications,
						 const QStringList &defaultNotifications,
						 const QString &iconPath,
						 const QString &appName) :
	m_d(new Private(notifications, defaultNotifications, iconPath, appName)) {
}
  
GrowlNotificationBridge::~GrowlNotificationBridge() {
	
	delete m_d;
}

void GrowlNotificationBridge::notify(const QString &name, 
				     const QString &title,
				     const QString &description,
				     const QString &mediaId,
				     const QObject *receiver,
				     const char *clikedSlot,
				     const char *timedOutSlot,
				     NotificationAction *action) {

	// Convert strings
	CFStringRef cfTitle = qString2CFString(title);
	CFStringRef cfDescription = qString2CFString(description);
	CFStringRef cfName = qString2CFString(name);

	CFDataRef icon = 0;
	bool allocatedIcon = false;

	if (!mediaId.isEmpty()) { 
		if (MediaCache::MediaCache::instance()->isInCache(mediaId)) { 
			QPixmap pixmap = MediaCache::MediaCache::instance()->pixmap(mediaId, 128, 128);
			if (!pixmap.isNull()) {
				QByteArray imageData;
				QBuffer buffer(&imageData);
				buffer.open(QIODevice::WriteOnly);
				pixmap.save(&buffer, "PNG");
				icon = CFDataCreate(0, (UInt8 *) imageData.data(), imageData.size());
				allocatedIcon = true;
			}
		}
	}

	if (!icon) {
		icon = m_d->icon;
	}

	// Do notification
	CFPropertyListRef context = createContext(m_d->signaler, receiver, clikedSlot, timedOutSlot, action);

	Growl_NotifyWithTitleDescriptionNameIconPriorityStickyClickContext(
			cfTitle, cfDescription, cfName, icon, 0, false, context);

	CFRelease(context);

	if (cfTitle) {
		CFRelease(cfTitle);
	}
	if (cfDescription) {
		CFRelease(cfDescription);
	}
	if (cfName) {
		CFRelease(cfName);
	}
	if (allocatedIcon) {
		CFRelease(icon);
	}
}

bool GrowlNotificationBridge::isGrowlInstalled() {
	
	return Growl_IsInstalled();
}

bool GrowlNotificationBridge::isGrowlRunning() {
	
	return Growl_IsRunning();
}

//-----------------------------------------------------------------------------
/**
 * Structure holding information about callback action for Growl notifications
 */
struct NotificationAction {
	
	NotificationAction(const QString &method, const QString &parameter) :
		method(method),
		parameter(parameter) {
	}
	
	QString method;
	QString parameter;
};

//-----------------------------------------------------------------------------

//path to the icon we want to display alongside the notifications
static QString iconPath = QString(":/data/appicon_64.png");
static QString growlMessageType = QString("Hyves Notification");

GrowlNotifier::GrowlNotifier() {
	
	m_notificationBridge = 0;
}

GrowlNotifier::~GrowlNotifier() {
	
	delete m_notificationBridge;
}

bool GrowlNotifier::isOperational() {
	
	// lazy loading - doesn't start initialization of the Growl API before
	// being ready to send a first notification
	if (!m_notificationBridge) {
		if (GrowlNotificationBridge::isGrowlRunning()) {
			// we're able to start Hyves growl subsystem
			start();
		}
	} else {
		if (!GrowlNotificationBridge::isGrowlRunning()) {
			// user stopped Growl in the meantime - we can't use it anymore !
			stop();
		}
	}
	return (m_notificationBridge);
}

void GrowlNotifier::start() {
	
	QStringList notifications;
	notifications.append(growlMessageType);
	
	m_notificationBridge = new GrowlNotificationBridge(notifications, notifications,
							 iconPath, HD_PRODUCT_NAME);
}

void GrowlNotifier::stop() {
	
	if (m_notificationBridge) {
		delete m_notificationBridge;
		m_notificationBridge = 0;
	}
}



void GrowlNotifier::triggerCallbackAction(NotificationAction *notificationAction) {
	
	// here we can extend the number of methods we want to take care of. 
	// for now, only two different actions have been identified
	if (notificationAction->method == "windowManager.raiseEvent") {
		WindowManager::WindowManager::instance()->raiseEvent(notificationAction->parameter);
	} else if (notificationAction->method == "windowManager.openUrl") {
		WindowManager::WindowManager::instance()->openUrl(notificationAction->parameter);
	} else {
		// don't do anything for now in case the method is not understood
	}
	// notificationAction was allocated by showNotification()
	delete notificationAction;
}

void GrowlNotifier::cleanup(NotificationAction *notificationAction) {
	
	// Cleanup happens after notification is timed out
	// (no callback has been called since there was no NotificationClicked event).
	// So at this point we need to clean allocated resources.

	// notificationAction was allocated by showNotification()
	delete notificationAction;
}

void GrowlNotifier::showNotification(const QString &textContent, const QString &method,
									 const QString &parameter, const QString &title,
									 const QString& mediaId, const QString& mediaSecret) {

	if (!m_notificationBridge) {
		return;
	}
	// process rich html text --> we want to display plain text via growl
	QTextDocument document;
	document.setHtml(textContent);
	QString plainTextContent = document.toPlainText();
	QString fullMediaId;
	if (!mediaId.isEmpty() && !mediaSecret.isEmpty()) {
	  fullMediaId = mediaId + "/" + mediaSecret;
	}

	// will be deleted by triggerCallbackAction() or cleanup()
	NotificationAction *action = new NotificationAction(method, parameter);

	// send a notification request to growl
	m_notificationBridge->notify(growlMessageType,
								 title,
								 plainTextContent,
								 fullMediaId,
								 this,
								 SLOT(triggerCallbackAction(NotificationAction *)),
								 SLOT(cleanup(NotificationAction *)),
								 (NotificationAction *) action);
}

} // namespace WindowManager
