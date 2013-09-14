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

#ifndef SYSTEM_SPY_H
#define SYSTEM_SPY_H

#include <QAbstractSocket>
#include <QObject>

#include "hyveslib_export.h"

namespace SystemSpy {

typedef enum {
	NetworkOk = 0, // Everything is just peachy
	// 1 was once used by 'Glitchy' but has been removed
	BrokenDown = 2 // We've got actual errors from the sockets
} NetworkState;

/**
 * The SystemSpy singleton spies on the system for events like sleeping,
 * networking glitchs and other hardware events. Plugins can subscribe to the
 * signals or read the current state.
 * 
 * Note: this class is inspired by Remco Troncon's SystemWatch class in Psi.
 *
 * Note: this class papers over several system-dependent capabilities.
 * Not all systems might implement everything.
 *
 * Note: this class is dependent on the Jabber singleton.
 *
 * TODO: integrate also with QSessionManager
 */
class HYVESLIB_EXPORT SystemSpy : public QObject {
	
	Q_OBJECT
	
	public:
		/**
		 * Returns the systemspy instance.
		 */
		static SystemSpy *instance();
		
		/**
		 * Destroys the global systemspy instance.
		 */
		static void destroy();
		
	public slots:
		/**
		 * @return the current state of the network.
		 */
		NetworkState networkState();
		
		/**
		 * set the network state to the specified state. Will
		 * emit signals as appropriate.
		 */
		void setNetworkState(NetworkState state);
		
		/**
		 * Set the specified network error. A network error always
		 * means the network state is BrokenDown until reset.
		 */
		void setNetworkError(int networkError);
		
		/**
		 * return the os we are running on.
		 */
		QString platform();
		
	signals:
		/**
		 * Emitted whenever the system is about to suspend, hibernate or sleep.
		 */
		void willSleep();
		
		/**
		 * Emitted whenever the system has just awoken from sleep.
		 */
		void justAwoken();
		
		/**
		 * Emitted whenever there is a clearly non-transient network error. Use
		 * this to disable anything that needs the network completely until the
		 * network has recovered.
		 */
		void networkDown();
		
		/**
		 * Emitted whenever the network ahs recovered completely.
		 */
		void networkRecovered();
		
	private:
		static SystemSpy *s_instance;
		
		struct Private;
		Private *const m_d;
		
	private slots:
	
		void connected();
		
	protected:
		/**
		 * The default way of monitoring the network for glitches is through 
		 * pinging at intervals. If an OS has a better way of monitoring its
		 * interfaces, reimplement this method.
		 */
		virtual void startNetworkStateMonitoring();
		
		/**
		 * Log using the parent log instance.
		 */
		void log(const QString &message);
		
		SystemSpy(QObject *parent = 0);
		virtual ~SystemSpy();
};

} // namespace SystemSpy

#endif
