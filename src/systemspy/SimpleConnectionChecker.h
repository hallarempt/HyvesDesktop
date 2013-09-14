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

#ifndef SIMPLE_CONNECTION_CHECKER_H
#define SIMPLE_CONNECTION_CHECKER_H

#include <QHostAddress>
#include <QHostInfo>
#include <QObject>
#include <QTcpSocket>

#include "SystemSpy.h"

const int FAIL_COUNT_DOWN = 6; // try to establish a connection for 6 times then emit neworkDown()

class QTimer;

namespace SystemSpy {

/**
 * The simple connection checker uses a timer and an udp 
 * socket to ping a host to determine whether the network 
 * interface is still doing fine.
 * 
 * We ping every 10 seconds; after six failed pings, we emit
 * networkDown(), after one succeeded ping, we emit networkUp().
 */ 
class SimpleConnectionChecker : public QObject {
	
	Q_OBJECT
	
	public:
		SimpleConnectionChecker( QObject* parent );
		~SimpleConnectionChecker();
		
		/**
		 * @returns the current network state
		 * @note: if SimpleConnectionChecker is not running the last known state is returned
		 */
		NetworkState currentNetworkState() const;
		
		/**
		 * @returns true if SimpleConnectionChecker is currently watching the network connection
		 */
		bool isRunning() const;

	public slots:
		/**
		 * start watching the network connection
		 * @param currentState give a hint about the current network state
		 */
		void start(NetworkState currentState = BrokenDown);

		/**
		 * stop watching the network connection
		 */
		void stop();
		
	signals:
		void networkUp();
		void networkDown();
		
	private slots:
		void tryConnect();
		void hostResult(QHostInfo info);
		void connected();
		void disconnected();
		void error(QAbstractSocket::SocketError);
		
	private:
		struct Private;
		Private *const m_d;
};

} // namespace SystemSpy

#endif
