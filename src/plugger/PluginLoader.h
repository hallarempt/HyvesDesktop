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

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <QtCore>
#include "hyveslib_export.h"

namespace Plugger {

struct PluginInfo;

/**
 * Interface for all plugin loaders.
 */
class HYVESLIB_EXPORT PluginLoader : public QObject {
	
	Q_OBJECT
	
	public:
		/**
		 * Constructor.
		 */
		PluginLoader(QObject *parent = 0);
		
		/**
		 * Destructor.
		 */
		virtual ~PluginLoader();
		
		/**
		 * Loads the plugin specified by @p info.
		 * 
		 * @param info PluginInfo structure with description of the plugin.
		 * @param args Parameters for the plugin.
		 * @return @c true on success, @c false on error.
		 */
		virtual bool loadPlugin(const PluginInfo &info, const QStringList &args) = 0;
		
		/**
		 * (Re-)activates an already loaded plugin.
		 *
		 * @return @c true on success, @c false on error.
		 */
		virtual bool activatePlugin();
		
		/**
		 * Sends the message specified by @p args.
		 *
		 * @param args The message.
		 */
		virtual bool sendMessage(const QStringList &args);

		/**
		 * Asks the plugin to stop. The return value will indicate whether the
		 * plugin accepts to stop.
		 * 
		 * If the plugin accepts to be stopped, unloadPlugin() will be called to
		 * do the actual destruction.
		 * 
		 * @return @c true if the plugin agrees to stop, @c false if it refuses.
		 */
		virtual bool stopPlugin();
		
		/**
		 * Unloads the previously loaded plugin.
		 *
		 * @return @c true on success, @c false on error.
		 */
		virtual bool unloadPlugin() = 0;
		
	signals:
		/**
		 * This signal should be emitted whenever the plugin terminates on its
		 * own, rather than being unloaded using unloadPlugin().
		 * 
		 * @param pluginName Name of the terminated plugin.
		 */
		void terminated(const QString &pluginName);
};

} // namespace Plugger

#endif // PLUGINLOADER_H
