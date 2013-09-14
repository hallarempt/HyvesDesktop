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

#ifndef SYSTEM_SPY_UNIX_H
#define SYSTEM_SPY_UNIX_H

#include <QDateTime>

#include "SystemSpy.h"
#include "SimpleConnectionChecker.h"

class QTimer;

namespace SystemSpy {

class SystemSpyUnix : public SystemSpy {
	
	Q_OBJECT
	
	public:
		SystemSpyUnix();
		~SystemSpyUnix();
		
	public slots:
		void checkSleep();
		
	private:
		QTimer *m_timer;
		QDateTime m_lastTime;
		SimpleConnectionChecker *m_connectionChecker;
};

} // namespace SystemSpy

#endif
