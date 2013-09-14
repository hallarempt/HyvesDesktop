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

#include <QByteArray>
#include <QList>
#include <QString>

#ifndef PLUGININFO_H
#define PLUGININFO_H

#include "PluginFile.h"
#include "Version.h"
#include "hyveslib_export.h"

namespace Plugger {

class PluginLoader;

/**
 * Structure containing meta-data about a plugin.
 */
struct HYVESLIB_EXPORT PluginInfo {
	
	/**
	 * Internal name of the plugin.
	 */
	QString name;
	
	/**
	 * Display name of the plugin.
	 */
	QString displayName;
	
	/**
	 * Vendor string.
	 */
	QString vendor;
	
	/**
	 * The version of the plugin.
	 */
	Version version;
	
	/**
	 * MIME type indicating the type of package.
	 */
	QString type;
	
	/**
	 * Full path where the plugin is installed.
	 */
	QString path;
	
	/**
	 * Defines entry point for launching the plugin.
	 */
	QString entry;
	
	/**
	 * Defines whether the entry point for the plugin should be registered
	 * globally. This will put an entry in the system tray icon, for instance.
	 */
	bool registerEntryPoint;
	
	/**
	 * List of all files in the package.
	 */
	QList<PluginFile> files;
	
	/**
	 * Boolean indicating whether the plugin is started automatically.
	 */
	bool autoStart;
	
	/**
	 * Plugin loader.
	 * 
	 * If the plugin is not currently running, this is a null pointer.
	 */
	PluginLoader *loader;
	
	/**
	 * Default constructor.
	 */
	PluginInfo();
	
	/**
	 * Copy constructor.
	 */
	PluginInfo(const PluginInfo &info);
	
	/**
	 * Reads plugin information from a manifest file.
	 *
	 * @param pluginPath Path to directory containing the manifest file.
	 * @return @c true on success, @c false on failure.
	 */
	bool loadManifest(const QString &pluginPath);
	
	/**
	 * Reads plugin information from a byte array.
	 *
	 * @param content The content of a manifest file.
	 * @return @c true on success, @c false on failure.
	 *
	 * @warning This method does not initialize the path property.
	 */
	bool loadManifestData(const QByteArray &content);
	
	/**
	 * Writes the current plugin information to the manifest file.
	 *
	 * @return @c true on success, @c false on failure.
	 */
	bool saveManifest() const;
	
	/**
	 * Assignment operator.
	 */
	PluginInfo &operator=(const PluginInfo &info);
	
	/**
	 * Equality operator.
	 *
	 * Only used for unit testing.
	 */
	bool operator==(const PluginInfo &info) const;
};

} // namespace Plugger

#endif // PLUGININFO_H
