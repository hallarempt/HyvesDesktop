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

#ifndef JABBER_H
#define JABBER_H

#include <QAbstractSocket>
#include <QDomDocument>
#include <QIODevice>
#include <QObject>
#include <QString>
#include <QUrl>

#include "hyveslib_export.h"

namespace Jabber {

typedef enum {
	NoError = 200,
	ConnectionError = 503,
	TimeoutError = 504,
	SecurityError = 601, // use custom error codes in the 6xx range
	AuthenticationError = 401,
	StreamError = 602
} ErrorCode;

typedef enum {
	DisconnectedState = 0,
	ConnectingState,
	AuthenticatingState,
	ActiveState
} ClientState;

/**
 * Main class for managing connections to a Jabber server.
 */
class HYVESLIB_EXPORT Jabber : public QObject {
	
	Q_OBJECT
	
	public:
		/**
		 * Returns the global Jabber instance.
		 */
		static Jabber *instance();
		
		/**
		 * Sets the global Jabber instance.
		 *
		 * This is used for testing purposes to insert a mock object instead of
		 * the real Jabber object. This method should be called before any real
		 * instance is instantiated.
		 */
		static void setInstance(Jabber *instance);
		
		/**
		 * Destroys the global Jabber instance.
		 */
		static void destroy();
		
		/**
		 * Returns the bare version of a given JID.
		 */
		static QString bare(const QString &jid);
		
		/**
		 * Returns the resource from a full JID.
		 */
		static QString resource(const QString &resource);
		
	public slots:
		/**
		 * Returns whether we're currently connected to the Jabber server.
		 */
		bool authenticated() const;
		
		/**
		 * Returns the client's state.
		 */
		int state() const;
		
		/**
		 * Returns the full JID of the logged in member. The full JID is the JID
		 * including the resource part.
		 * 
		 * Returns an empty string if we're not authenticated.
		 */
		QString fullJid() const;
		
		/**
		 * Returns the bare JID of the logged in member. The bare JID is the JID
		 * excluding the resource part.
		 * 
		 * Returns an empty string if we're not authenticated.
		 */
		QString bareJid() const;
		
		/**
		 * @deprecated
		 */
		virtual void setProtocolVersion(int version);
		
		/**
		 * Overrides the host and port to connect to. By default, the host will
		 * be determined using an SRV DNS lookup.
		 *
		 * @param host Host to connect to.
		 * @param port Port to connect on.
		 */
		virtual void setHostAndPort(const QString &host, int port);
		
		/**
		 * Overrides the authentication digest. By default, the digest will be
		 * based upon the user's password. By overriding the digest, the
		 * password given during signIn() will be ignored.
		 *
		 * @param digest The digest to use for authentication.
		 */
		virtual void setDigest(const QString &digest);
		
		/**
		 * Connects and signs in to a Jabber server.
		 *
		 * @param username Username for login.
		 * @param realm Realm in which to login.
		 * @param password Password for login.
		 * @param resource Resource to use.
		 */
		virtual void signIn(const QString &username, const QString &realm,
		                    const QString &password, const QString &resource = QString::null);
		
		/**
		 * Signs out and disconnects from the Jabber server.
		 */
		virtual void signOut();
		
		/**
		 * Sends a raw XML stream to the server.
		 *
		 * @param xmlString XML stream to send.
		 */
		virtual void sendXml(const QString &xmlString);
		
		/**
		 * Allows to manually set an error code.
		 *
		 * @param errorCode Error code to set.
		 */
		void setErrorCode(int errorCode);
		
	signals:
		/**
		 * Emitted when successfully connected to the server and authenticated.
		 */
		void connected();
		
		/**
		 * Emitted when an error occurs in the connection.
		 *
		 * @param errorCode Error code.
		 *
		 * @sa ErrorCode
		 */
		void connectionError(int errorCode);
		
		/**
		 * Emitted when we are disconnected because of a resource conflict.
		 */
		void resourceConflict();
		
		/**
		 * Emitted when disconnected from the server for an unknown reason.
		 */
		void disconnected();
		
		/**
		 * Emitted when the state changes.
		 *
		 * @param state The new state.
		 */
		void stateChanged(int state);
		
		/**
		 * Emitted when a new XML stanza is received.
		 *
		 * @param document DOM document containing the stanza.
		 */
		void incomingXml(QDomDocument document);
		
	protected:
		Jabber(bool useTcpSocket = true);
		virtual ~Jabber();
		
		void setJid(const QString &jid);
		
		void setState(ClientState state);
		
		void setUsername(const QString &username);
		
		virtual void reset();
		
		QIODevice* inputDevice();
		QIODevice* outputDevice();
		
		/**
		 * Check whether it is save to write to the output device
		 *
		 * @note: Writing to a disconnected socket turned out to cause crashes and endless-loops
		 * on Windows - so please make use of this function
		 */
		bool outputDeviceIsValid();

		/**
		 * Takes a look at the streamBuffer and returns an XML snippet that's
		 * guaranteed to be opened *and* closed. This way it will always return
		 * a complete iq, message or presence block. The XML declaration and
		 * <stream:stream> are returned immediately.
		 *
		 * The returned snippet is automatically cut off from the streamBuffer.
		 *
		 * @return The next available snippet, or an empty byte array if none is
		 *         available.
		 */
		static QByteArray nextBufferedSnippet(QByteArray &streamBuffer);
		
	protected slots:
		void socketConnected();
		void socketDisconnected();
		void socketError(QAbstractSocket::SocketError error);
		
	private:
		static Jabber *s_instance;
		
		struct Private;
		Private *const m_d;
		
	private slots:
		void incomingData();
		void incomingXml(const QString &string);
		void processSnippet(QByteArray snippet);
		void processStreamFeatures();
		void processSaslMechanisms();
		void processAuthResult1();
		void processIQ();
		void processPresence();
		
		void sendPing();
		
		void increaseNumRosterUpdatesReceived();
};

} // namespace Jabber

#endif // JABBER_H
