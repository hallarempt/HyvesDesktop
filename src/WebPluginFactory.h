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

#ifndef WEBPLUGINFACTORY_H
#define WEBPLUGINFACTORY_H

#include <QWebPluginFactory>

#include "hyveslib_export.h"

class HYVESLIB_EXPORT WebPluginFactory : public QWebPluginFactory {
	
	public:
		static WebPluginFactory *instance();
		static void destroy();
		
		QObject *create(const QString &mimeType, const QUrl &url,
		                const QStringList &argumentNames, const QStringList &argumentValues) const;
		
		QList<Plugin> plugins() const;
		
		/**
		 * Registers a plugin that can be created by the plugin factory.
		 * 
		 * @param mimeType MIME type handled by the plugin.
		 * @param object Object that provides the plugin functionality.
		 */
		void registerPlugin(const QString &mimeType, QObject *object);
		
		/**
		 * Unregisters a previously registered plugin.
		 * 
		 * @param mimeType MIME type handled by the plugin.
		 */
		void unregisterPlugin(const QString &mimeType);
		
	private:
		static WebPluginFactory *s_instance;
		
		WebPluginFactory(QObject *parent = 0);
		virtual ~WebPluginFactory();
		
		struct Private;
		Private *const m_d;
};

#endif // WEBPLUGINFACTORY_H
