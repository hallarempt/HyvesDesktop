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

#include <QTimer>

#include "SystemSpyUnix.h"

namespace SystemSpy {

SystemSpyUnix::SystemSpyUnix() :
	SystemSpy() {
	
	m_lastTime = QDateTime::currentDateTime();
	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(checkSleep()));
	m_timer->start(1000 * 30); // every 30 seconds
	m_connectionChecker = new SimpleConnectionChecker(this);
	connect(m_connectionChecker, SIGNAL(networkUp()), this, SIGNAL(justAwoken()));
	connect(m_connectionChecker, SIGNAL(networkUp()), m_connectionChecker, SLOT(stop()));
}

SystemSpyUnix::~SystemSpyUnix() {
	m_timer->stop();
}

void SystemSpyUnix::checkSleep() {
	
	QDateTime curTime = QDateTime::currentDateTime();
	
	// if we lost more then 60 seconds we've slept long enough for all tcp connections to be timed out
	if (m_lastTime.secsTo(curTime) > ((m_timer->interval() / 1000) + 60))  {
		m_connectionChecker->start(); // wait for network to come up again
	}
	
	m_lastTime = curTime;
}

} // namespace SystemSpy
