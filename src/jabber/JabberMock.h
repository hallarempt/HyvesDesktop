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

#ifndef JABBERMOCK_H
#define JABBERMOCK_H

#include "Jabber.h"
#include "hyveslib_export.h"

namespace Jabber {

/**
 * Mock object simulating behavior of a real Jabber object.
 */
class HYVESLIB_EXPORT JabberMock : public Jabber {
	
	Q_OBJECT
	
	public:
		JabberMock();
		
		virtual void signIn(const QString &username, const QString &realm,
		                    const QString &password, const QString &resource = QString::null);
		
	public slots:
		/**
		 * Emulates incoming XML in the Jabber class.
		 *
		 * @param string XML data to be read by the Jabber class.
		 */
		void incomingXml(const QString &string);
		
		/**
		 * Returns a message that was sent to the server.
		 *
		 * @param index Index of the message to return. 0 is the index
		 *              of the last message, 1 is the previous message,
		 *              and so on.
		 */
		QString sentMessage(int index = 0) const;
		
		/**
		 * Returns whether the server has received a ping message from
		 * the client.
		 */
		bool pingReceived() const;
		
		/**
		 * Sets the message body used by MUC messages.
		 */
		void setMucMessageBody(const QString &bodyString);
		
	protected:
		virtual void reset();
		
	private:
		struct Private;
		Private *const m_d;
		
		virtual ~JabberMock();
		
		void processSnippet(QByteArray snippet);
		
	private slots:
		void outgoingData();
};

} // namespace Jabber

#endif // JABBERMOCK_H
