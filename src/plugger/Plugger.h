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

#include <QObject>
#include <QStringList>

#ifndef PLUGGER_H
#define PLUGGER_H

#include "hyveslib_export.h"
#include "PluginInfo.h"
#include "PluginMap.h"

namespace Plugger {

/**
 * Enumeration of all error codes.
 */
typedef enum {
	NoError = 0,
	FileNotFound,
	UnknownPlugin,
	CouldNotLinkPlugin,
	CouldNotInstantiatePlugin,
	CannotOpenForWriting,
	ErrorUnpackingArchive,
	PluginAlreadyInstalled
} ErrorCode;

/**
 * Plugger is the main class for the Hyves Desktop plugin system.
 */
class HYVESLIB_EXPORT Plugger : public QObject {
	
	Q_OBJECT
	
	public:
		/**
		 * Returns the global Plugger instance.
		 */
		static Plugger *instance();
		
		/**
		 * Destroys the global Plugger instance.
		 */
		static void destroy();
		
		/**
		 * Initializes the Plugger. This will trigger the loading of all
		 * plugins.
		 */
		void initialize();
		
		/**
		 * Sets a custom directory where plugins are located. If not set,
		 * a suitable location is created.
		 *
		 * Besides this directory, there is also an application wide plugin
		 * directory for plugins that ship with the application itself. This
		 * directory cannot be changed.
		 *
		 * @param pluginDir The directory to look for plugins.
		 *
		 * @note You should call this function before initialize() to have any
		 *       effect.
		 */
		void setPluginDir(QString pluginDir);
		
		/**
		 * Installs a new plugin.
		 *
		 * @param filePath Full path to the plugin package file.
		 * @return @c true on success, @c false on failure.
		 */
		bool installPlugin(QString filePath);
		
		/**
		 * Uninstalls an installed plugin.
		 *
		 * @param pluginName Name of the plugin to uninstall.
		 * @return @c true on success, @c false on failure.
		 */
		bool uninstallPlugin(QString pluginName);
		
		/**
		 * Returns a list with the names of all installed plugins.
		 *
		 * @return A list of strings containing the names of all plugins.
		 */
		QStringList installedPlugins() const;
		
		/**
		 * Returns plugin information about a plugin.
		 *
		 * @param pluginName Name of the plugin to get information about.
		 * @return Structure containing information about the plugin.
		 */
		PluginInfo pluginInfo(QString pluginName);
		
		/**
		 * Sets whether a plugin should be started automatically when Hyves
		 * Desktop is started.
		 *
		 * @param pluginName Name of the plugin to set the auto-start property
		 *                   for.
		 * @param autoStart @c true if the plugin should be started
		 *                  automatically, @c false otherwise.
		 * @return @c true on success, @c false on failure.
		 *
		 * @sa runPlugin()
		 */
		bool setAutoStart(QString pluginName, bool autoStart);
		
		/**
		 * Returns the error code of the last error. This code is updated
		 * every time a method returns @c false to indicate an error.
		 *
		 * @return The error code of the last error.
		 */
		ErrorCode lastError() const;
		
		/**
		 * Returns a text message describing an error code.
		 *
		 * @param errorCode The error code to return the message for.
		 * @return Localized error message for the error the code.
		 */
		static QString errorMessage(ErrorCode errorCode);
		
	public slots:
		/**
		 * Runs a plugin.
		 *
		 * This is the same as enablePlugin(), except that it will not mark the
		 * plugin as enabled, so that it won't be automatically loaded on the
		 * next start.
		 *
		 * @param pluginName Name of the plugin to run.
		 * @param args Arguments for the plugin.
		 * @return @c true on success, @c false on failure.
		 */
		bool runPlugin(QString pluginName, QStringList args = QStringList());
		
		/**
		 * Stops execution of a running plugin.
		 * 
		 * This is the same as disablePlugin(), except that it will not change
		 * the enabled status of the plugin.
		 * 
		 * @param pluginName Name of the plugin to stop.
		 * @return @c true on success, @c false on failure.
		 */
		bool stopPlugin(QString pluginName);
		
		/**
		 * Checks whether a plugin is currently running.
		 * 
		 * @param pluginName Name of the plugin to check whether it's running.
		 * @return @c true if the specified plugin is currently running,
		 *         @c false otherwise. 
		 */
		bool isRunning(QString pluginName) const;
		
	signals:
		/**
		 * Emitted when a plugin is discovered that wants its entry point to be
		 * registered.
		 * 
		 * @param pluginName Name of the plugin.
		 * @param displayName Display name of the plugin.
		 */
		void registerEntryPoint(const QString &pluginName, const QString &displayName);
		
	private:
		static Plugger *s_instance;
		
		struct Private;
		Private *const m_d;
		
		Plugger();
		virtual ~Plugger();
		
		void loadPlugins();
		void loadPluginDirectory(QString path);
		bool loadPlugin(QString name, QStringList args = QStringList());
		
		void unloadPlugins();
		bool unloadPlugin(QString name);
		
	private slots:
		void pluginTerminated(const QString &pluginName);
};

} // namespace Plugger

#endif // PLUGGER_H
