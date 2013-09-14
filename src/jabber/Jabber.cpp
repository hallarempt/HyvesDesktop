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

#include <QTcpSocket>
#include <QTimer>
#include <QVariantMap>
#include <QtXml>

#include "extender/Extender.h"
#include "logger/Logger.h"
#include "roster/Roster.h"
#include "roster/RosterItem.h"
#include "tester/Tester.h"
#include "Jabber.h"

namespace Jabber {

Jabber *Jabber::s_instance = 0;

static const QString streamNS = "http://etherx.jabber.org/streams";
static const QString jabberNS = "jabber:client";
static const QString authNS = "jabber:iq:auth";
static const QString rosterNS = "jabber:iq:roster";
static const QString pingNS = "urn:xmpp:ping";
static const QString xmppSaslNS = "urn:ietf:params:xml:ns:xmpp-sasl";
static const QString hyvesXmppNS = "http://hyves.net/xmpp";
static const QString chatDomain = "hyves.nl";

struct Jabber::Private {
	Logger::Logger *log;
	
	ClientState state;
	
	QString username;
	QString digest;
	
	QString host;
	int port;
	
	QIODevice *inputDevice;
	QIODevice *outputDevice;
	QXmlStreamReader *reader;
	QXmlStreamWriter *writer;
	
	QByteArray streamBuffer;
	
	int errorCode;
	
	int numRosterUpdatesReceived;
	
	QTimer *pingTimer;
	QDateTime lastPong;
	quint64 pingCounter;
	
	Private() :
		log(0),
		state(DisconnectedState),
		port(0),
		inputDevice(0),
		outputDevice(0),
		reader(0),
		writer(0),
		numRosterUpdatesReceived(0) {
		
		pingTimer = 0;
		pingCounter = 0;
	}
	
	~Private() {
		
		delete log;
	}
};

Jabber *Jabber::instance() {
	
	if (s_instance == 0) {
		s_instance = new Jabber();
	}
	
	return s_instance;
}

void Jabber::setInstance(Jabber *instance) {
	
	Q_ASSERT(s_instance == 0);
	s_instance = instance;
}

void Jabber::destroy() {
	
	delete s_instance;
	s_instance = 0;
}

QString Jabber::bare(const QString &jid) {
	
	QString bareJid(jid);
	int index = bareJid.indexOf('/');
	if (index != -1) {
		bareJid = bareJid.left(index);
	}
	return bareJid;
}

QString Jabber::resource(const QString &jid) {
	
	int index = jid.indexOf('/');
	if (index != -1) {
		return jid.mid(index + 1);
	}
	return QString::null;
}

bool Jabber::authenticated() const {
	
	return m_d->state == ActiveState;
}

int Jabber::state() const {
	
	return m_d->state;
}

QString Jabber::fullJid() const {
	
	return Roster::Roster::instance()->ownFullJid();
}

QString Jabber::bareJid() const {
	
	return Roster::Roster::instance()->ownBareJid();
}

void Jabber::setProtocolVersion(int version) {
	Q_UNUSED(version);
}

void Jabber::setHostAndPort(const QString &host, int port) {
	
	m_d->host = host;
	m_d->port = port;
}

void Jabber::setDigest(const QString &digest) {
	
	m_d->digest = digest;
}

void Jabber::signIn(const QString &username, const QString &realm,
                    const QString &password, const QString &resource) {

	if (m_d->state == AuthenticatingState || m_d->state == ConnectingState) {
		m_d->log->debug("Already connecting...");
		return;
	}
	
	QString jid = username + "@" + realm + "/" + resource;
	
	if (m_d->state == ActiveState && jid == fullJid()) {
		emit connected();
		return;
	}
	
	reset();
	
	setUsername(username);
	
	Roster::Roster::instance()->setOwnJid(jid);
	
	m_d->log->debug("Connecting to server.");
	
	setState(ConnectingState);
	
	QTcpSocket *socket = qobject_cast<QTcpSocket*>(m_d->outputDevice);
	if (socket) {
		socket->connectToHost(m_d->host, m_d->port);
	} else
	{
		m_d->log->error("Failed to open socket for jabber connection!");
	}
}

void Jabber::signOut() {
	
	reset();
}

void Jabber::sendXml(const QString &xmlString) {
	
	Q_ASSERT(m_d->outputDevice);
	
	if (m_d->state == ActiveState && m_d->outputDevice->isWritable()) {
		m_d->outputDevice->write(xmlString.toUtf8());
	}
}

void Jabber::setErrorCode(int error) {
	
	m_d->log->debug(QString("Setting error code to: %1").arg(error));
	
	m_d->errorCode = error;
	
	emit connectionError(m_d->errorCode);
	
	reset();
}

Jabber::Jabber(bool useTcpSocket) :
	QObject(),
	m_d(new Private()) {
	
	m_d->pingTimer = new QTimer(this);
	connect(m_d->pingTimer, SIGNAL(timeout()), this, SLOT(sendPing()));
	m_d->pingTimer->start(1000 * 30);
	m_d->lastPong = QDateTime::currentDateTime();
	
	m_d->log = new Logger::Logger("Jabber");
	
	if (useTcpSocket) {	// this is used by JabberMock to use a QBuffer instead of a real socket
		QTcpSocket *socket = new QTcpSocket();	// no testing -> create a real socket
		connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
		connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
		connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
		connect(socket, SIGNAL(readyRead()), this, SLOT(incomingData()));
		
		m_d->inputDevice = socket;
		m_d->outputDevice = socket;
	} else {
		m_d->inputDevice = new QBuffer();
		m_d->outputDevice = new QBuffer();
	}
	
	m_d->reader = new QXmlStreamReader();
	m_d->writer = new QXmlStreamWriter(m_d->outputDevice);
	
	Extender::Extender::instance()->registerObject("jabber", this);
}

Jabber::~Jabber() {
	
	reset();
	
	Extender::Extender::instance()->unregisterObject("jabber");
	
	delete m_d->reader;
	delete m_d->writer;
	
	if (m_d->inputDevice != m_d->outputDevice) {
		delete m_d->inputDevice;
		delete m_d->outputDevice;
	} else {
		m_d->outputDevice->disconnect();
		delete m_d->outputDevice;
	}
	
	delete m_d;
}

void Jabber::setJid(const QString &jid) {
	
	Roster::Roster::instance()->setOwnJid(jid);
}

void Jabber::setState(ClientState state) {
	
	switch (state) {
		case DisconnectedState:
			m_d->log->debug("Entering disconnected state.");
			break;
		
		case ConnectingState:
			m_d->log->debug("Entering connecting state.");
			break;
		
		case AuthenticatingState:
			m_d->log->debug("Entering authenticating state.");
			break;
		
		case ActiveState:
			m_d->log->debug("Entering active state.");
			break;
	}
	
	m_d->state = state;
	emit stateChanged(state);
}

void Jabber::setUsername(const QString &username) {
	
	m_d->username = username;
}

void Jabber::reset() {
	
	if (m_d->state == DisconnectedState) {
		m_d->log->debug("Already disconnected.");
		return;
	}
	
	m_d->log->notice(QString("Resetting Jabber (current state: %1)").arg(m_d->state));

	if (m_d->writer && outputDeviceIsValid()) {
		m_d->writer->writeEndDocument();
	}
	
	if (m_d->reader) {
		m_d->reader->clear();
	}
	
	if (m_d->outputDevice) {
		m_d->outputDevice->close();
	}
	
	if (m_d->inputDevice) {
		m_d->inputDevice->close();
	}
	
	setState(DisconnectedState);
	
	m_d->username = "";
	
	Roster::Roster::instance()->clear();
	m_d->numRosterUpdatesReceived = 0;

	emit disconnected();
}

QIODevice* Jabber::inputDevice() {
	
	return m_d->inputDevice;
}

QIODevice* Jabber::outputDevice() {
	
	return m_d->outputDevice;
}

QByteArray Jabber::nextBufferedSnippet(QByteArray &streamBuffer) {
	
	int openStartIndex = streamBuffer.indexOf('<');
	if (openStartIndex == -1) {
		return QByteArray();
	}
	
	int openEndIndex;
	for (openEndIndex = openStartIndex + 1; openEndIndex < streamBuffer.size(); openEndIndex++) {
		char character = streamBuffer[openEndIndex];
		if (character == ' ' || character == '>') {
			break;
		}
	}
	
	if (openEndIndex == streamBuffer.size()) {
		return QByteArray();
	}
	
	QByteArray tag = streamBuffer.mid(openStartIndex + 1, openEndIndex - (openStartIndex + 1));
	
	if (tag == "?xml") {
		int closeIndex = streamBuffer.indexOf("?>") + 2;
		if (closeIndex == 1) {
			return QByteArray();
		}
		
		QByteArray snippet = streamBuffer.mid(openStartIndex, closeIndex - openStartIndex);
		streamBuffer = streamBuffer.mid(closeIndex);
		return snippet;
	}
	
	if (tag == "stream:stream") {
		int closeIndex = streamBuffer.indexOf(">") + 1;
		if (closeIndex == 0) {
			return QByteArray();
		}
		
		QByteArray snippet = streamBuffer.mid(openStartIndex, closeIndex - openStartIndex);
		streamBuffer = streamBuffer.mid(closeIndex);
		return snippet;
	}
	if (tag == "/stream:stream") {
		QByteArray snippet = streamBuffer.mid(openStartIndex);
		streamBuffer = "";
		return snippet;
	}
	
	int closeIndex = streamBuffer.indexOf("</" + tag + ">");
	if (closeIndex == -1) {
		closeIndex = streamBuffer.indexOf("/>");
		if (closeIndex == -1 || streamBuffer.indexOf(">") < closeIndex) {
			return QByteArray();
		}
		
		closeIndex += 2;
		QByteArray snippet = streamBuffer.mid(openStartIndex, closeIndex - openStartIndex);
		streamBuffer = streamBuffer.mid(closeIndex);
		return snippet;
	}
	
	closeIndex += tag.size() + 3;
	QByteArray snippet = streamBuffer.mid(openStartIndex, closeIndex - openStartIndex);
	streamBuffer = streamBuffer.mid(closeIndex);
	return snippet;
}

void Jabber::socketConnected() {
	
	if (m_d->state != ConnectingState) {
		m_d->log->debug(QString("Got connected while in wrong state: %1").arg(m_d->state));
		return;
	}
	
	setState(AuthenticatingState);
	
	if (m_d->writer && outputDeviceIsValid()) {
		m_d->writer->writeStartDocument();
		m_d->writer->writeDefaultNamespace(jabberNS);
		m_d->writer->writeNamespace(streamNS, "stream");
		m_d->writer->writeStartElement(streamNS, "stream");
		m_d->writer->writeAttribute("to", chatDomain);
		m_d->writer->writeCharacters(" ");
	}
}

void Jabber::socketDisconnected() {
	
	if (m_d->state != DisconnectedState) {
		m_d->log->notice("Client disconnected.");
		
		reset();
	}
}

void Jabber::socketError(QAbstractSocket::SocketError error) {
	
	m_d->log->warning("Socket error: " + m_d->inputDevice->errorString());
	setErrorCode(ConnectionError);
}

void Jabber::incomingData() {
	
	if (m_d->inputDevice && m_d->inputDevice->isReadable()) {
		m_d->streamBuffer += m_d->inputDevice->readAll();
		
		QByteArray snippet = nextBufferedSnippet(m_d->streamBuffer);
		while (!snippet.isEmpty()) {
			processSnippet(snippet);
			snippet = nextBufferedSnippet(m_d->streamBuffer);
		}
	}
}

void Jabber::incomingXml(const QString &string) {
	
	if (m_d->state != ActiveState) {
		return;
	}
	
	// it actually doesn't matter whether we got a real pong or
	// something else, we're alive
	m_d->lastPong = QDateTime::currentDateTime();
	
	QDomDocument document;
	document.setContent(string);
	
	if (document.firstChild().nodeName() == "stream:error") {
		m_d->log->debug("Got a stream error.");
		if (document.firstChild().firstChild().nodeName() == "conflict") {
			m_d->log->debug("Resource conflict -- user logged in from another location.");
			
			emit resourceConflict();
			
			reset();
		}
		return;
	}
	
	emit incomingXml(document);
}

void Jabber::processSnippet(QByteArray snippet) {
	
	if (m_d->state == ActiveState) {
		incomingXml(QString::fromUtf8(snippet));
	}
	
	if (m_d->state == AuthenticatingState || m_d->state == ActiveState) {
		Q_ASSERT(m_d->reader);
		if (!m_d->reader) return;		

		m_d->reader->addData(snippet);
		
		while (m_d->reader && !m_d->reader->atEnd()) {
			QXmlStreamReader::TokenType tokenType = m_d->reader->readNext();
			
			if (tokenType == QXmlStreamReader::StartElement) {
				if (m_d->state == AuthenticatingState &&
				    m_d->reader->namespaceUri() == streamNS && m_d->reader->name() == "features") {
					processStreamFeatures();
				} else if (m_d->state == AuthenticatingState &&
				           m_d->reader->namespaceUri() == jabberNS && m_d->reader->name() == "iq") {
					if (m_d->reader->attributes().value("type") == "result" &&
					    m_d->reader->attributes().value("id") == "auth_1") {
						processAuthResult1();
					} else if (m_d->reader->attributes().value("type") == "result" &&
					           m_d->reader->attributes().value("id") == "auth_2") {
						setState(ActiveState);
						emit connected();
					} else if (m_d->reader->attributes().value("type") == "error") {
						m_d->log->warning("XMPP sent an error result during authentication.");
						setErrorCode(AuthenticationError);
					}
				} else if (m_d->state == ActiveState &&
				           m_d->reader->namespaceUri() == jabberNS && m_d->reader->name() == "iq") {
					if (m_d->reader->attributes().value("type") == "result" ||
					    m_d->reader->attributes().value("type") == "set") {
						processIQ();
					}
				} else if (m_d->state == ActiveState &&
				           m_d->reader->namespaceUri() == jabberNS && m_d->reader->name() == "presence") {
					processPresence();
				}
			}
		}
	} else {
		m_d->log->debug(QString("Got incoming data while in wrong state: %1").arg(m_d->state));
	}
}

void Jabber::processStreamFeatures() {
	
	while (m_d->reader && !m_d->reader->atEnd()) {
		QXmlStreamReader::TokenType tokenType = m_d->reader->readNext();
		
		if (tokenType == QXmlStreamReader::StartElement) {
			if (m_d->reader->name() == "mechanisms" && m_d->reader->namespaceUri() == xmppSaslNS) {
				processSaslMechanisms();
			}
		} else if (tokenType == QXmlStreamReader::EndElement) {
			return;
		}
	}
}

void Jabber::processSaslMechanisms() {
	
	bool supportsDigest = false;
	bool mechanismsEnded = false;

	Q_ASSERT(m_d->reader);
	if (!m_d->reader) return;
	
	while (!m_d->reader->atEnd() && !mechanismsEnded) {
		QXmlStreamReader::TokenType tokenType = m_d->reader->readNext();
		
		if (tokenType == QXmlStreamReader::StartElement) {
			if (m_d->reader->name() == "mechanism" && m_d->reader->namespaceUri() == xmppSaslNS) {
				if (m_d->reader->readElementText() == "DIGEST-MD5") {
					supportsDigest = true;
				}
			}
		} else if (tokenType == QXmlStreamReader::EndElement) {
			if (m_d->reader->name() == "mechanisms" && m_d->reader->namespaceUri() == xmppSaslNS) {
				mechanismsEnded = true;
			}
		}
	}
	
	if (supportsDigest) {
		Q_ASSERT(m_d->writer && outputDeviceIsValid());
		if (m_d->writer && outputDeviceIsValid()) {
			m_d->writer->writeStartElement(jabberNS, "iq");
			m_d->writer->writeAttribute("type", "get");
			m_d->writer->writeAttribute("to", chatDomain);
			m_d->writer->writeAttribute("id", "auth_1");
			m_d->writer->writeStartElement("query");
			m_d->writer->writeDefaultNamespace(authNS);
			m_d->writer->writeStartElement("username");
			m_d->writer->writeCharacters(m_d->username);
			m_d->writer->writeEndElement();
			m_d->writer->writeEndElement();
			m_d->writer->writeEndElement();
		}
	} else {
		m_d->log->warning("XMPP server does not support DIGEST-MD5 authentication.");
		setErrorCode(AuthenticationError);
	}
}

void Jabber::processAuthResult1() {
	
	bool isQuery = false;
	bool queryUsername = false;
	bool queryDigest = false;
	bool queryResource = false;
	bool resultEnded = false;
	
	Q_ASSERT(m_d->reader);
	if (!m_d->reader) return;
	
	while (!m_d->reader->atEnd() && !resultEnded) {
		QXmlStreamReader::TokenType tokenType = m_d->reader->readNext();
		
		if (tokenType == QXmlStreamReader::StartElement) {
			if (isQuery) {
				if (m_d->reader->name() == "username" && m_d->reader->namespaceUri() == authNS) {
					queryUsername = true;
				} else if (m_d->reader->name() == "digest" && m_d->reader->namespaceUri() == authNS) {
					queryDigest = true;
				} else if (m_d->reader->name() == "resource" && m_d->reader->namespaceUri() == authNS) {
					queryResource = true;
				} else {
					m_d->log->warning("Server authentication queries for unknown property: %1" + m_d->reader->name().toString());
				}
			} else {
				if (m_d->reader->name() == "query" && m_d->reader->namespaceUri() == authNS) {
					isQuery = true;
				}
			}
		} else if (tokenType == QXmlStreamReader::EndElement) {
			if (m_d->reader->name() == "iq" && m_d->reader->namespaceUri() == jabberNS) {
				resultEnded = true;
			} else if (m_d->reader->name() == "query" && m_d->reader->namespaceUri() == authNS) {
				isQuery = false;
			}
		}
	}
	
	if (queryUsername && queryDigest && queryResource) {
		
		Q_ASSERT(m_d->writer && outputDeviceIsValid());
		if (!m_d->writer || !outputDeviceIsValid()) return;
		
		m_d->writer->writeStartElement(jabberNS, "iq");
		m_d->writer->writeAttribute("type", "set");
		m_d->writer->writeAttribute("to", chatDomain);
		m_d->writer->writeAttribute("id", "auth_2");
		m_d->writer->writeStartElement("query");
		m_d->writer->writeDefaultNamespace(authNS);
		m_d->writer->writeStartElement("username");
		m_d->writer->writeCharacters(m_d->username);
		m_d->writer->writeEndElement();
		m_d->writer->writeStartElement("digest");
		m_d->writer->writeCharacters(m_d->digest);
		m_d->writer->writeEndElement();
		m_d->writer->writeStartElement("resource");
		m_d->writer->writeCharacters(Roster::Roster::instance()->ownResource());
		m_d->writer->writeEndElement();
		m_d->writer->writeEndElement();
		m_d->writer->writeEndElement();
	} else {
		m_d->log->warning("Server authentication didn't query for username, digest and resource.");
		setErrorCode(AuthenticationError);
	}
}

void Jabber::processIQ() {
	
	bool iqEnded = false;
	bool isQuery = false;
	Roster::RosterItem *item = 0;
		
	Q_ASSERT(m_d->reader);
	if (!m_d->reader) return;
	
	while (!m_d->reader->atEnd() && !iqEnded) {
		QXmlStreamReader::TokenType tokenType = m_d->reader->readNext();
		
		if (tokenType == QXmlStreamReader::StartElement) {
			if (item) {
				if (m_d->reader->name() == "group" && m_d->reader->namespaceUri() == rosterNS) {
					item->addGroup(m_d->reader->readElementText());
				}
			} else if (isQuery) {
				if (m_d->reader->name() == "item") {
					QString jid = m_d->reader->attributes().value("jid").toString();
					if (!jid.contains("@")) {
						m_d->log->debug("Item has no valid JID!");
						continue;
					}
					
					item = Roster::Roster::instance()->item(jid);
					item->unlock();
					
					if (m_d->reader->namespaceUri() == rosterNS) {
						item->setProperty("nick", m_d->reader->attributes().value("name").toString());
					} else if (m_d->reader->namespaceUri() == hyvesXmppNS) {
						foreach (QXmlStreamAttribute attribute, m_d->reader->attributes()) {
							if (attribute.name() != "jid") {
								item->setProperty(attribute.name().toString(), attribute.value().toString());
							}
						}
					} else {
						m_d->log->debug("Ignoring item attributes in unknown namespace: " + m_d->reader->namespaceUri().toString());
					}
				}
			} else {
				if (m_d->reader->name() == "query" &&
				    (m_d->reader->namespaceUri() == rosterNS || m_d->reader->namespaceUri() == hyvesXmppNS)) {
					isQuery = true;
				}
			}
		} else if (tokenType == QXmlStreamReader::EndElement) {
			if (m_d->reader->name() == "iq" && m_d->reader->namespaceUri() == jabberNS) {
				iqEnded = true;
			} else if (m_d->reader->name() == "query" &&
			           (m_d->reader->namespaceUri() == rosterNS || m_d->reader->namespaceUri() == hyvesXmppNS)) {
				isQuery = false;
			} else if (m_d->reader->name() == "item" &&
			           (m_d->reader->namespaceUri() == rosterNS || m_d->reader->namespaceUri() == hyvesXmppNS)) {
				Q_ASSERT(item != 0);
				item->lock();
				item = 0;
			}
		}
	}
	
	increaseNumRosterUpdatesReceived();
}

void Jabber::processPresence() {
	
	Q_ASSERT(m_d->reader);
	if (!m_d->reader) return;
	
	QString jid = m_d->reader->attributes().value("from").toString();
	
	Roster::Roster *roster = Roster::Roster::instance();
	Roster::RosterItem *item = roster->item(bare(jid));
	item->unlock();
	
	if (m_d->reader->attributes().value("type") == "unavailable") {
		item->removeResource(resource(jid));
		item->lock();
		return;
	}
	
	bool invisible = (m_d->reader->attributes().value("type") == "invisible");
	
	Roster::Resource itemResource = item->resource(resource(jid));
	bool presenceEnded = false;
	
	while (!m_d->reader->atEnd() && !presenceEnded) {
		QXmlStreamReader::TokenType tokenType = m_d->reader->readNext();
		
		if (tokenType == QXmlStreamReader::StartElement) {
			if (m_d->reader->name() == "priority" && m_d->reader->namespaceUri() == jabberNS) {
				itemResource.priority = m_d->reader->readElementText().toInt();
			} else if (m_d->reader->name() == "show") {
				itemResource.show = m_d->reader->readElementText();
			} else if (m_d->reader->name() == "status") {
				itemResource.status = m_d->reader->readElementText();
			}
		} else if (tokenType == QXmlStreamReader::EndElement) {
			if (m_d->reader->name() == "presence" && m_d->reader->namespaceUri() == jabberNS) {
				presenceEnded = true;
			}
		}
	}
	
	if (invisible) {
		itemResource.status = "offline";
	} else {
		if (!roster->statusExists(itemResource.status)) {
			// we'll guess based on the show property
			if (itemResource.show == "away" || itemResource.show == "xa") {
				itemResource.status = "away";
			} else if (itemResource.show == "dnd") {
				itemResource.status = "busy";
			} else {
				itemResource.status = "online";
			}
		}
	}
	
	itemResource.show = roster->statusShow(itemResource.status);
	
	item->addResource(itemResource);
	
	// keep status for our own resources synchronized
	if (item->jid() == bareJid()) {
		Roster::Resource ownResource = item->resource(resource(fullJid()));
		ownResource.status = itemResource.status;
		ownResource.show = itemResource.show;
		item->addResource(ownResource);
	}
	
	item->lock();
}

void Jabber::sendPing() {
	
	if (m_d->state != ActiveState) {
		return;
	}
	
	if (!Tester::Tester::isTesting() &&
	    m_d->lastPong.secsTo(QDateTime::currentDateTime()) > 60) {
		setErrorCode(TimeoutError);
		return;
	}
	
	m_d->pingCounter++;
	
	if (m_d->writer && outputDeviceIsValid()) {
		m_d->writer->writeStartElement(jabberNS, "iq");
		m_d->writer->writeAttribute("from", fullJid());
		m_d->writer->writeAttribute("id", "c2s" + QString::number(m_d->pingCounter));
		m_d->writer->writeAttribute("type", "get");
		m_d->writer->writeStartElement("ping");
		m_d->writer->writeDefaultNamespace(pingNS);
		m_d->writer->writeEndElement();
		m_d->writer->writeEndElement();
	}
}

void Jabber::increaseNumRosterUpdatesReceived() {
	
	m_d->numRosterUpdatesReceived++;
	
	if (m_d->numRosterUpdatesReceived == 4) {
		// 4 roster updates are necessary to initialize
		Roster::Roster::instance()->setInitialized();
	}
}

bool Jabber::outputDeviceIsValid() {
	if (!m_d->outputDevice || !m_d->outputDevice->isWritable()) {
		return false;
	}

	QTcpSocket *socket = qobject_cast<QTcpSocket*>(m_d->outputDevice);
	return (!socket || socket->isValid());	// if we have a tcp socket, make sure it's valid othwise it's just a QBuffer (for testing)
}

} // namespace Jabber
