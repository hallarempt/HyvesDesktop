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

#ifndef EXTENDER_H
#define EXTENDER_H

#include <QObject>
#include <QString>
#include <QVariant>

#include "hyveslib_export.h"
#include "MetaTypes.h"

class QWebFrame;

namespace Extender {

/**
 * The Extender is responsible for extending JavaScript environments with native
 * C++ functionality, and allowing calling of JavaScript functions from C++.
 */
class HYVESLIB_EXPORT Extender : public QObject {
	
	Q_OBJECT
	
	public:
		/**
		 * Returns the global Extender instance.
		 */
		static Extender *instance();
		
		/**
		 * Destroys the global Extender instance.
		 */
		static void destroy();
		
		/**
		 * Registers a QWebFrame environment.
		 *
		 * @param frame Frame to register.
		 */
		void registerEnvironment(QWebFrame *frame);
		
		/**
		 * Unregisters a registered QWebFrame environment.
		 *
		 * @param frame Frame to unregister.
		 */
		void unregisterEnvironment(QWebFrame *frame);
		
		/**
		 * Registers an object to expose to the JavaScript environments.
		 *
		 * @param name The name to register the object under.
		 * @param object The object to register.
		 */
		void registerObject(const QString &name, QObject *object);
		
		/**
		 * Unregisters a registered object.
		 *
		 * @param name Name of the registered object.
		 * 
		 * @warning It is generally a bad idea to unregister objects as long
		 *          as there are still environments available, because the
		 *          connection from the environment to the object is not
		 *          explicitly broken.
		 * @note Registered objects which get deleted are automatically
		 *       unregistered.
		 */
		void unregisterObject(const QString &name);
		
		/**
		 * Returns a registered object.
		 *
		 * @param name Name of the registered object.
		 * @return The registered object, or 0 if the object is not found.
		 */
		QObject *registeredObject(const QString &name);
		
		/**
		 * Calls a JavaScript function.
		 *
		 * @param environmentId (Internal) ID of the environment in which to
		 *                      execute the JavaScript call.
		 * @param function Name of the function to call.
		 * @param argumentList Optional list of arguments.
		 */
		QVariant jsCall(int environmentId, const QString &function,
		                QVariantList argumentList = QVariantList());
		
		/**
		 * Returns the statement that is currently being executed in the
		 * JavaScript environment, if any.
		 * 
		 * @return The currently executing statement, or QString::null if not
		 *         currently executing any statement.
		 */
		QString currentExecutingStatement() const;
		
	private:
		static Extender *s_instance;
		
		struct Private;
		Private *const m_d;
		
		Extender();
		virtual ~Extender();
		
	private slots:
		void javaScriptWindowObjectCleared();
		
		void objectDestroyed(QObject *object);
};

} // namespace Extender

#endif // EXTENDER_H
