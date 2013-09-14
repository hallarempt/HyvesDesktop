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

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <QObject>
#include <QString>

#include "hyveslib_export.h"

class QWebFrame;

namespace Extender {

class HYVESLIB_EXPORT Environment : public QObject {
	
	Q_OBJECT
	
	Q_PROPERTY(int id READ id)
	
	public:
		QWebFrame *webFrame;
		
		Environment();
		virtual ~Environment();
		
		int id();
		
		void disconnectAll();
		
	public slots:
		void connect(const QString &sender, const QString &signal, const QString &slot);
		
	private:
		struct Private;
		Private *const m_d;
};

} // namespace Extender

#endif // ENVIRONMENT_H
