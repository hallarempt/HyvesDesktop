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

#ifndef CXXPLUGININTERFACE_H
#define CXXPLUGININTERFACE_H

#include <QtPlugin>

#include "hyveslib_export.h"

namespace Plugger {

class HYVESLIB_EXPORT CXXPluginInterface {
	
	public:
		virtual ~CXXPluginInterface() {};
		
		virtual bool run() = 0;
		virtual bool activate() = 0;
		virtual bool handleMessage(const QStringList &args) = 0;
		
		/**
		 * Called when the plugin is asked to stop. The stop action can be
		 * cancelled by the plugin by returning false.
		 * 
		 * @return @c true if the plugin wants to stop, @c false if the plugin
		 *         refuses to stop.
		 */
		virtual bool acceptStopEvent() = 0;
};

} // namespace Plugger

Q_DECLARE_INTERFACE(Plugger::CXXPluginInterface,
                    "nl.hyves.Plugger.CXXPluginInterface/1.0")

#endif // CXXPLUGININTERFACE_H
