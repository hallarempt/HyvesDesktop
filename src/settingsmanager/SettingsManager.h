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

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QVariant>

#include "hyveslib_export.h"

class QSettings;

namespace SettingsManager {

class HYVESLIB_EXPORT SettingsManager : public QObject {
	
	Q_OBJECT
	
	public:
		/**
		 * Returns the global SettingsManager instance.
		 */
		static SettingsManager *instance();
		
		/**
		 * Destroys the global SettingsManager instance.
		 */
		static void destroy();
		
		/**
		 * Removes all settings and reverts to defaults. Also empties
		 * the cookiejar.
		 */
		void clearSettings();
		
		/**
		 * @return a pointer to the settings object managed by this class.
		 *
		 * Note: preferably use the methods below, but for some complicated
		 * cases, like reading an array, access to the QSettings instance is
		 * the only option.
		 */
		QSettings *settings();
		
		QVariant variantValue(const QString &key, const QVariant &defaultValue = QVariant());
		void setVariantValue(const QString &key, const QVariant &value, const QVariant &defaultValue = QVariant());
		
	public slots:
		bool boolValue(const QString &key, bool defaultValue = false);
		int intValue(const QString &key, int defaultValue = 0);
		QString stringValue(const QString &key, const QString &defaultValue = "");
		
		void setValue(const QString &key, bool value, bool defaultValue = false);
		void setValue(const QString &key, int value, int defaultValue = 0);
		void setValue(const QString &key, const QString &value);
		void setValue(const QString &key, const QByteArray &value);
		
		/**
		 * Returns the host name of the server to connect to Hyves.
		 */
		QString baseServer() const;
		
	signals:
		/**
		 * This signal is emitted every time a setting changes value.
		 *
		 * @param key The key of the setting that has changed.
		 */
		void settingChanged(const QString &key);
		
	private:
		static SettingsManager *s_instance;
		
		struct Private;
		Private *const m_d;
		
		SettingsManager();
		virtual ~SettingsManager();
};

} // namespace SettingsManager

#endif // SETTINGSMANAGER_H
