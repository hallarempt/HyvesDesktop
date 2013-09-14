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

#include <QBuffer>
#include <QStringList>
#include <QTimer>

#include "extender/Extender.h"
#include "logger/Logger.h"
#include "tester/Tester.h"
#include "DelayedJabberMessage.h"
#include "JabberMock.h"

namespace Jabber {

struct JabberMock::Private {
	Logger::Logger *log;
	
	qint64 outputBufferPos;
	
	QStringList sentMessages;
	bool pingReceived;
	
	QString mucMessageBody;
	int firstMucSessionId;
	
	Private() :
		log(0),
		outputBufferPos(0),
		pingReceived(false) {
	}
	
	~Private() {
		
		delete log;
	}
};

JabberMock::JabberMock() :
	Jabber(false),
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("JabberMock");
	
	Extender::Extender::instance()->unregisterObject("jabber"); // get rid of parent binding
	Extender::Extender::instance()->registerObject("jabber", this);
}

void JabberMock::signIn(const QString &username, const QString &realm,
                        const QString &password, const QString &resource) {
	
	QString jid = username + "@" + realm + "/" + resource;
	
	if (state() == ActiveState && jid == fullJid()) {
		emit connected();
		return;
	}
	
	reset();
	
	setUsername(username);
	setJid(jid);
	
	m_d->log->debug("Connecting to mock object.");
	
	setState(ConnectingState);
	
	Q_ASSERT(inputDevice());
	inputDevice()->open(QBuffer::ReadWrite);
	Q_ASSERT(outputDevice());
	outputDevice()->open(QBuffer::ReadWrite);
	
	connect(inputDevice(), SIGNAL(readyRead()), this, SLOT(incomingData()));
	connect(outputDevice(), SIGNAL(readyRead()), this, SLOT(outgoingData()));
	
	socketConnected();
}

void JabberMock::incomingXml(const QString &string) {
	
	if (!inputDevice() || !inputDevice()->isWritable()) {
		Tester::Tester::instance()->failTest("Trying to emulate incoming XML data while not connected.");
		return;
	}
	
	qint64 pos = inputDevice()->pos();
	inputDevice()->seek(inputDevice()->size());
	inputDevice()->write(string.toUtf8());
	inputDevice()->seek(pos);
}

QString JabberMock::sentMessage(int index) const {
	
	return m_d->sentMessages[m_d->sentMessages.size() - 1 - index];
}

bool JabberMock::pingReceived() const {
	
	return m_d->pingReceived;
}

void JabberMock::setMucMessageBody(const QString &bodyString) {
	
	m_d->mucMessageBody = bodyString;
}

void JabberMock::reset() {
	
	Jabber::reset();
	m_d->outputBufferPos = 0;
}

JabberMock::~JabberMock() {
	
	// no need to unregister "jabber", parent will do that
	delete m_d;
}

void JabberMock::processSnippet(QByteArray _snippet) {
	
	// make the snippet a bit easier to process
	QString snippet = _snippet;
	snippet.replace(QRegExp(" id=\"m_\\d+\" "), " id=\"m_1\" ");
	
	if (snippet.startsWith("<iq from=\"" + fullJid() + "\" id=\"c2s") &&
	    snippet.endsWith("\" type=\"get\"><ping xmlns=\"urn:xmpp:ping\"/></iq>")) {
		m_d->pingReceived = true;
		return;
	}
	
	m_d->sentMessages.append(snippet);
	
	if (snippet == "<?xml version=\"1.0\" encoding=\"UTF-8\"?>") {
		new DelayedJabberMessage(
			"<?xml version='1.0' encoding='UTF-8'?>\n"
			"<stream:stream xmlns:stream=\"http://etherx.jabber.org/streams\" xmlns=\"jabber:client\" id=\"1279\" xml:lang=\"en\" version=\"0.0\" from=\"hyves.nl\">"
		, 30);
		return;
	}
	
	if (snippet == "<stream:stream xmlns=\"jabber:client\" xmlns:stream=\"http://etherx.jabber.org/streams\" to=\"hyves.nl\">") {
		new DelayedJabberMessage(
			"<stream:features>\n"
				"<mechanisms xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\">\n"
					"<mechanism>DIGEST-MD5</mechanism>\n"
				"</mechanisms>\n"
				"<auth xmlns=\"http://jabber.org/features/iq-auth\"/>\n"
			"</stream:features>"
		, 60);
		return;
	}
	
	if (snippet == "<iq type=\"get\" to=\"hyves.nl\" id=\"auth_1\"><query xmlns=\"jabber:iq:auth\"><username>arendtest</username></query></iq>") {
		new DelayedJabberMessage(
			"<iq type=\"result\" id=\"auth_1\" >\n"
				"<query xmlns=\"jabber:iq:auth\">\n"
					"<username/>\n"
					"<digest/>\n"
					"<resource/>\n"
				"</query>\n"
			"</iq>"
		, 30);
		return;
	}
	
	if (snippet.startsWith("<iq type=\"set\" to=\"hyves.nl\" id=\"auth_2\"><query xmlns=\"jabber:iq:auth\"><username>arendtest</username><digest>") &&
	    snippet.endsWith("</resource></query></iq>")) {
		new DelayedJabberMessage(
			"<iq type=\"result\" id=\"auth_2\" />"
		, 30);
		return;
	}
	
	if (snippet == "<iq id=\"roster_5\" type=\"get\"><query xmlns=\"jabber:iq:roster\" /></iq>") {
		// default response to roster request
		new DelayedJabberMessage(
			"<iq type=\"result\" to=\"" + fullJid() + "\" id=\"roster_5\" >\n" +
				"<query xmlns=\"jabber:iq:roster\">\n" +
					"<item subscription=\"both\" name=\":kilian: Kilian\" jid=\"kilian@hyves.nl\" >\n" +
						"<group>Friends</group>\n" +
					"</item>\n" +
					"<item subscription=\"both\" name=\"Youri\" jid=\"optroodt@hyves.nl\" >\n" +
						"<group>Friends</group>\n" +
					"</item>\n" +
					"<item subscription=\"both\" name=\"blaatpuk &lt;script&gt;alert('TypeError: Gotcha!');&lt;/script&gt;\" jid=\"toymachine@hyves.nl\" >\n" +
						"<group>Friends</group>\n" +
					"</item>\n" +
				"</query>\n" +
			"</iq>"
		, 30);
		return;
	}
	
	if (snippet == "<iq id=\"Hyves_Roster_Extension\" type=\"get\"><query xmlns=\"http://hyves.net/xmpp\" /></iq>") {
		// default response to extended roster request
		new DelayedJabberMessage(
			"<iq type=\"result\" to=\"" + fullJid() + "\" id=\"Hyves_Roster_Extension\" >\n" +
				"<query xmlns=\"http://hyves.net/xmpp\">\n" +
					"<item whatsOnYourMind=\"\" memberId=\"84\" wwwEmotion=\"&lt;blink&gt;JOEHOE&lt;/blink&gt;&quot;&gt;&quot; \" " +
					      "wwwTime=\"20081016113724\" mediaId=\"12742281\" mediaSecret=\"HJW2\" jid=\"kilian@hyves.nl\" " +
					      "wwwLocation=\"&lt;blink&gt;XSSland&lt;/blink&gt;\" blocked=\"False\" />\n" +
					"<item whatsOnYourMind=\"\" memberId=\"7211\" wwwEmotion=\"wandelen met [hyver=downicola]Nic[/hyver]\" " +
					      "wwwTime=\"20081016113724\" mediaId=\"12742281\" mediaSecret=\"HJW2\" jid=\"optroodt@hyves.nl\" " +
					      "wwwLocation=\"strand\" blocked=\"False\" />\n" +
					"<item whatsOnYourMind=\"\" memberId=\"123\" wwwEmotion=\"\" " +
					      "wwwTime=\"\" mediaId=\"12742281\" mediaSecret=\"HJW2\" jid=\"toymachine@hyves.nl\" " +
					      "wwwLocation=\"\" blocked=\"False\" />\n" +
				"</query>\n" +
			"</iq>"
		, 30);
		new DelayedJabberMessage(
			"<iq type=\"set\" to=\"" + fullJid() + "\" id=\"1\" >\n" +
				"<query xmlns=\"http://hyves.net/xmpp\">\n" +
					"<item whatsOnYourMind=\"\" memberId=\"96566\" wwwEmotion=\"test\" wwwTime=\"20081014101139\" " +
					       "mediaId=\"1487042\" mediaSecret=\"38c5\" jid=\"" + bareJid() + "\" " +
					       "wwwLocation=\"hyves hq\" blocked=\"False\" />\n" +
				"</query>\n" +
			"</iq>"
		, 60);
		new DelayedJabberMessage(
			"<iq type=\"set\" to=\"" + fullJid() + "\" id=\"1\" >\n" +
				"<query xmlns=\"http://hyves.net/xmpp\">\n" +
					"<item visibility=\"2\" nick=\"Arie\" chatability=\"2\" jid=\"" + bareJid() + "\" />\n" +
				"</query>\n" +
			"</iq>"
		, 90);
		return;
	}
	
	if (snippet == "<presence><show>normal</show><status>online</status><priority>8</priority></presence>") {
		// one contact online
		new DelayedJabberMessage(
			"<presence from=\"kilian@hyves.nl/siteClient\" to=\"" + bareJid() + "\" >\n" +
				"<priority>4</priority>\n" +
			"</presence>"
			"<presence from=\"optroodt@hyves.nl/desktopClient\" to=\"" + bareJid() + "\" >\n" +
				"<priority>4</priority>\n" +
			"</presence>"
			"<presence from=\"toymachine@hyves.nl/desktopClient_1234\" to=\"" + bareJid() + "\" >\n" +
				"<priority>4</priority>\n" +
			"</presence>"
		, 30);
		return;
	}
	
	if (snippet == "<message id=\"m_1\" to=\"kilian@hyves.nl/siteClient\" type='chat'><body>testing full JID</body><active xmlns='http://jabber.org/protocol/chatstates'/></message>") {
		// fake a reply from Kilian
		new DelayedJabberMessage(
			"<message from=\"kilian@hyves.nl/siteClient\" to=\"" + bareJid() + "\" type='chat' id='m_2'>"
				"<body>Ack</body>"
				"<active xmlns='http://jabber.org/protocol/chatstates'/>"
			"</message>"
		, 30);
		return;
	}
	
	if (snippet.startsWith("<message id=\"m_1\" to=\"optroodt@hyves.nl/desktopClient\" type=\"chat\"><invitestart xmlns=\"http://hyves.net/xmpp\" invitee=\"toymachine@hyves.nl\" fromSessionId=\"")) {
		// filter out the session ID we will be using
		int fromSessionIdIndex = snippet.indexOf("fromSessionId=\"") + 15;
		m_d->firstMucSessionId = snippet.mid(fromSessionIdIndex, snippet.indexOf("\"", fromSessionIdIndex) - fromSessionIdIndex).toInt();
		// reply to invitestart message
		new DelayedJabberMessage(
			"<message id=\"m_1\" from=\"optroodt@hyves.nl/desktopClient\" to=\"" + fullJid() + "\" type=\"chat\">"
				"<inviteack xmlns=\"http://hyves.net/xmpp\" invitee=\"toymachine@hyves.nl\" toSessionId=\"" + QString::number(m_d->firstMucSessionId) + "\" fromSessionId=\"87654321\" />"
				+ m_d->mucMessageBody +
			"</message>"
		, 30);
		return;
	}
	
	if (snippet == "<message id=\"m_1\" to=\"toymachine@hyves.nl\" type=\"chat\"><sessioninvite xmlns=\"http://hyves.net/xmpp\" fromSessionId=\"" + QString::number(m_d->firstMucSessionId) + "\"><participants><participant jid=\"optroodt@hyves.nl/desktopClient\" sessionId=\"87654321\"/></participants></sessioninvite>" + m_d->mucMessageBody + "</message>") {
		// reply to sessioninvite message
		new DelayedJabberMessage(
			"<message id=\"m_1\" from=\"toymachine@hyves.nl/desktopClient_1234\" to=\"" + fullJid() + "\" type=\"chat\">"
				"<sessionjoined xmlns=\"http://hyves.net/xmpp\" toSessionId=\"" + QString::number(m_d->firstMucSessionId) + "\" fromSessionId=\"12345678\" />"
				+ m_d->mucMessageBody +
			"</message>"
		, 30);
		return;
	}
	
	if (snippet == "<message id=\"m_1\" to=\"optroodt@hyves.nl/desktopClient\" type=\"chat\"><invitestart xmlns=\"http://hyves.net/xmpp\" invitee=\"kilian@hyves.nl\" fromSessionId=\"" + QString::number(m_d->firstMucSessionId) + "\" />" + m_d->mucMessageBody + "</message>") {
		// reply to invitestart message
		new DelayedJabberMessage(
			"<message id=\"m_2\" from=\"optroodt@hyves.nl/desktopClient\" to=\"" + fullJid() + "\" type=\"chat\">"
				"<inviteack xmlns=\"http://hyves.net/xmpp\" invitee=\"kilian@hyves.nl\" toSessionId=\"" + QString::number(m_d->firstMucSessionId) + "\" fromSessionId=\"87654321\" />"
				+ m_d->mucMessageBody +
			"</message>"
		, 30);
		return;
	}
	if (snippet == "<message id=\"m_1\" to=\"toymachine@hyves.nl/desktopClient_1234\" type=\"chat\"><invitestart xmlns=\"http://hyves.net/xmpp\" invitee=\"kilian@hyves.nl\" fromSessionId=\"" + QString::number(m_d->firstMucSessionId) + "\" />" + m_d->mucMessageBody + "</message>") {
		// reply to invitestart message
		new DelayedJabberMessage(
			"<message id=\"m_2\" from=\"toymachine@hyves.nl/desktopClient_1234\" to=\"" + fullJid() + "\" type=\"chat\">"
				"<inviteack xmlns=\"http://hyves.net/xmpp\" invitee=\"kilian@hyves.nl\" toSessionId=\"" + QString::number(m_d->firstMucSessionId) + "\" fromSessionId=\"12345678\" />"
				+ m_d->mucMessageBody +
			"</message>"
		, 30);
		return;
	}
	
	m_d->log->debug("Unrecognized message: " + snippet);	
}

void JabberMock::outgoingData() {
	
	if (!outputDeviceIsValid()) {
		m_d->log->warning("Received outgoing data while not connected.");
		return;
	}
	
	outputDevice()->seek(m_d->outputBufferPos);
	QByteArray data = outputDevice()->readAll();
	m_d->outputBufferPos = outputDevice()->pos();
	
	QByteArray snippet = nextBufferedSnippet(data);
	while (!snippet.isEmpty()) {
		processSnippet(snippet);
		snippet = nextBufferedSnippet(data);
	}
}

} // namespace Jabber
