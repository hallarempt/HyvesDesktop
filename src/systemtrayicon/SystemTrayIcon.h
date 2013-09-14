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

#ifndef SYSTEMTRAYICON_H
#define SYSTEMTRAYICON_H

#include <QObject>
#include <QSystemTrayIcon>

#include "hyveslib_export.h"

namespace SystemTrayIcon {

/**
 * The SystemTrayIcon is a singleton that represents the system tray
 * icon of the Hyves Desktop application. The SystemTrayIcon can be
 * used to push notifications to the user, as well as offer right-click
 * (ordinary click on OS X) menu options.
 */
class HYVESLIB_EXPORT SystemTrayIcon : public QObject {
	
	Q_OBJECT
	
	public:
		static SystemTrayIcon *instance();
		static void destroy();
		
	signals:
		/**
		 * Emitted whenever a user chooses a menu option that was added from
		 * JavaScript.
		 */
		void menuOptionClicked(const QString &menuOption);

		/*
		 * Emitted whenever a message is clicked
		 *
		 * Warning: Do not connect to this signal. This is added temporarily for
		 * the usage by the photouploader.
		 */
		void messageClicked();
		
	public slots:
		/**
		 * Add the specified menu item to the submenu.
		 *  
		 * @param menu The main menu the item should be added to.
		 * @param menuItem The menu item text itself.
		 * @param key The key with which the the activated slot of the action
		 *            presents itself in JavaScript.
		 * 
		 * @note The slot is called without any arguments. All state is in
		 * JavaScript, this is just notification of a selection.
		 */
		void addAction(const QString &menu, const QString &key, const QString &menuItem, const QString &group = QString::null);
		
		/**
		 * Removes all the plugin actions.
		 */
		void removePluginActions();
		
		/**
		 * Removes the specified plugin action. If the action is the last item
		 * in the submenu, the submenu is removed, too.
		 */
		void removePluginAction(const QString &menu, const QString &key); 
		
		/**
		 * Sets an action to be selected.
		 */
		void setSelectedAction(const QString &menu, const QString &key);
		
		/**
		 * Returns the geometry of the tray icon. Use this to make the
		 * sliders popup in the right location.
		 */
		QString trayIconGeometryJSON();
		
		/**
		 * Returns the currently used icon in the system tray.
		 *
		 * @sa setIcon()
		 */
		const QString icon() const;
		
		/**
		 * Sets the system tray icon to the specified resource or file.
		 * 
		 * @param icon The icon to display in the system tray. Use
		 *             "active", "inactive", or "highlight" for
		 *             predefined (platform dependent) icons.
		 */
		void setIcon(const QString &icon);
		
		/**
		 * Show a message.
		 * 
		 * TODO: Integrate with the slider messages.
		 */
		void showMessage(const QString &title, const QString &message, 
					     QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information);
		
	private:
		static SystemTrayIcon *s_instance;
		
		struct Private;
		Private *const m_d;
		
		SystemTrayIcon();
		virtual ~SystemTrayIcon();
		
	private slots:
		void updateIcon();
		
		void onMenuClick();
		
		void onActivated(QSystemTrayIcon::ActivationReason reason);
		
		void onPluginEntryPointClicked();
		
		void registerPluginEntryPoint(const QString &pluginName, const QString &displayName);
};

} // namespace GUI

#endif // SYSTEMTRAYICON_H
