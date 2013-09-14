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

#include <QDebug>
#include <QTimer>
#include "logger/Logger.h"

#include "settingsmanager/SettingsManager.h"
#include "SimpleConnectionChecker.h"

/*
 NOTE: please leave the commented out debug messages in here for testing purposes.
 */
namespace SystemSpy {

	struct SimpleConnectionChecker::Private {
	QTcpSocket socket;
	QHostAddress host;
	Logger::Logger *log;
	QTimer *timer;
	bool busy;
	int countFailed;
};
	
SimpleConnectionChecker::SimpleConnectionChecker( QObject *parent) :
	QObject(parent), m_d(new Private()) {
	
	QHostInfo::lookupHost(SettingsManager::SettingsManager::instance()->baseServer(), this, SLOT(hostResult(QHostInfo)));
	
	m_d->timer = new QTimer(this);
	m_d->log = new Logger::Logger("ConnectionChecker");
	m_d->busy = false;
	m_d->countFailed = 0;
	
	connect(m_d->timer, SIGNAL(timeout()), SLOT(tryConnect()));
	connect(&m_d->socket, SIGNAL(connected()), SLOT(connected()));
	connect(&m_d->socket, SIGNAL(disconnected()), SLOT(disconnected()));
	connect(&m_d->socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(error(QAbstractSocket::SocketError)));
}

SimpleConnectionChecker::~SimpleConnectionChecker() {
	
	delete m_d->timer;
	delete m_d->log;
	delete m_d;
}
	
void SimpleConnectionChecker::tryConnect() {
	
	//qDebug() << "tryConnect. busy: " << m_d->busy << ", countFailed: " << m_d->countFailed;
	
	if (m_d->host.isNull()) {
		QHostInfo::lookupHost(SettingsManager::SettingsManager::instance()->baseServer(), this, SLOT(hostResult(QHostInfo)));
		return;
	}
	
	if (m_d->busy) {
		if (m_d->countFailed ==  FAIL_COUNT_DOWN) {
			m_d->log->debug(QString("Network is down: %1 successive attempts to ping %2 have timed out.").arg(FAIL_COUNT_DOWN).arg(m_d->host.toString()));
			emit networkDown();
		}
		m_d->socket.abort();
		m_d->busy = false;
	} else {
		m_d->busy = true;
		m_d->socket.connectToHost(m_d->host, 80);
	}
}

void SimpleConnectionChecker::connected() {
	//qDebug() << "connected. busy: " << m_d->busy << ", countFailed: " << m_d->countFailed;
	
	if (m_d->countFailed > 0) {
		m_d->countFailed = 0;
		m_d->log->debug("Network is up");
		emit networkUp();
	}
	m_d->socket.abort();
}

void SimpleConnectionChecker::disconnected() {
	//qDebug() << "disconnected. busy: " << m_d->busy << ", countFailed: " << m_d->countFailed;
	
	m_d->busy = false;
}

void SimpleConnectionChecker::error(QAbstractSocket::SocketError) {
	
	m_d->countFailed++;
	//qDebug() << "error. busy: " << m_d->busy << ", countFailed: " << m_d->countFailed;
	if (m_d->countFailed == FAIL_COUNT_DOWN) {
		m_d->log->debug(QString("Network is down: %1 successive attempts to ping %2 have failed.").arg(FAIL_COUNT_DOWN).arg(m_d->host.toString()));
		emit networkDown();
	}
	m_d->socket.abort();
	m_d->busy = false;
}

void SimpleConnectionChecker::hostResult(QHostInfo info) {
	
	QList<QHostAddress> addresses = info.addresses();
	if (info.error() == QHostInfo::NoError && addresses.size() > 0) {
		m_d->host = addresses[0];
		m_d->log->debug(QString("Dns lookup for %1 succeeded: %2").arg(info.hostName()).arg(m_d->host.toString()));
		
		if (m_d->timer->isActive()) {
			m_d->busy = true;
			m_d->socket.connectToHost(m_d->host, 80);
		}
		return;
	}
	// on first dns-lookup failure print log message and emit networkDown(), then silently increase error counter only
	if (m_d->countFailed == 0) {
		m_d->log->debug(QString("Failed to resolve %1: %2").arg(info.hostName()).arg((info.error() == QHostInfo::HostNotFound) ? "host not found" : "unknown error"));
		emit networkDown();
	}
	m_d->countFailed++;
}

void SimpleConnectionChecker::start(NetworkState currentState) {
	
	switch (currentState) {
		case NetworkOk:
			m_d->countFailed = 0;
			break;
		case BrokenDown:
			m_d->countFailed = FAIL_COUNT_DOWN;
			break;
		default:
			m_d->log->warning(QString("Failed to start watching connection: unknown network state (%1)").arg(currentState));
			return;
	}
	m_d->log->debug(QString("Start watching connection. Current network state: %1").arg(currentState));
	m_d->timer->start(1000 * 10);
}

void SimpleConnectionChecker::stop() {
	
	m_d->busy = false;
	m_d->socket.abort();
	m_d->timer->stop();
	m_d->log->debug("Stop watching connection");
}

bool SimpleConnectionChecker::isRunning() const {
	return m_d->timer->isActive();
}

NetworkState SimpleConnectionChecker::currentNetworkState() const {
	
	if (!m_d->timer->isActive()) {
		m_d->log->debug("Currently not watching connection. Returning last known network state.");
	}
	
	if (m_d->countFailed > FAIL_COUNT_DOWN || m_d->host.isNull()) {
		return BrokenDown;
	} else {
		return NetworkOk;
	}
}

} // namespace SystemSpy
