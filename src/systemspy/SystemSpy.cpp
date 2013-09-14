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

#include "extender/Extender.h"
#include "jabber/Jabber.h"
#include "logger/Logger.h"
#include "SystemSpy.h"

#if defined(Q_OS_MAC)
	#include "SystemSpyOsx.h"
#elif defined(Q_OS_WIN32)
	#include "SystemSpyWin.h"
#else
	#include "SystemSpyUnix.h"
#endif

namespace SystemSpy {

SystemSpy *SystemSpy::s_instance = 0;

struct SystemSpy::Private {
	Logger::Logger *log;
	NetworkState networkState;
	
	~Private() {
		delete log;
	}
};

SystemSpy *SystemSpy::instance() {
	
	if (s_instance == 0) {
#if defined(Q_OS_MAC)
		s_instance = new SystemSpyOSX();
#elif defined(Q_OS_WIN32)
		s_instance = new SystemSpyWin();
#else
		s_instance = new SystemSpyUnix();
#endif
	}
	
	return s_instance;
}

void SystemSpy::destroy() {
	
	delete s_instance;
	s_instance = 0;
}

NetworkState SystemSpy::networkState() {
	
	return m_d->networkState;
}

void SystemSpy::setNetworkError(int networkError) {
	
	m_d->log->debug(QString("Received network error %1, network has broken down").arg(networkError));
	setNetworkState(BrokenDown);
}

QString SystemSpy::platform() {
	
#if defined(Q_OS_MAC)
	return "OSX";
#elif defined(Q_OS_WIN32)
	return "WINDOWS";
#else
	return "LINUX";
#endif
}

void SystemSpy::startNetworkStateMonitoring() {
}

void SystemSpy::setNetworkState(NetworkState state) {
	
	m_d->log->debug(QString("Setting network state to %1").arg(state));
	
	switch (state) {
		case NetworkOk:
			emit networkRecovered();
			break;
		case BrokenDown:
			emit networkDown();
			break;
		default:
			Q_ASSERT_X(state != 1, "setNetworkState(NetworkState)", "Network state 'Glitchy' has been removed - please update your code!");
			m_d->log->warning(QString("Failed to set network state: %1 is not a valid state").arg(state));
			return;
	}
	m_d->networkState = state;
}

void SystemSpy::log(const QString &logMessage) {
	
	m_d->log->debug(logMessage);
}

void SystemSpy::connected() {
	
	if (networkState() != NetworkOk) {
		setNetworkState(NetworkOk);
	}
}


SystemSpy::SystemSpy(QObject *parent) :
	QObject(parent),
	m_d(new Private()) {
	
	m_d->networkState = NetworkOk;
	m_d->log = new Logger::Logger("SystemSpy");
	
	connect(Jabber::Jabber::instance(), SIGNAL(connectionError(int)), this, SLOT(setNetworkError(int)));
	connect(Jabber::Jabber::instance(), SIGNAL(connected()), this, SLOT(connected()));
	
	Extender::Extender::instance()->registerObject("systemSpy", this);
}

SystemSpy::~SystemSpy() {
	
	Extender::Extender::instance()->unregisterObject("systemSpy");
	delete m_d;
}

} // namespace SystemSpy
