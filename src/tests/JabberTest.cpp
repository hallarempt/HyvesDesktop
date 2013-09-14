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

#include <QString>
#include <QTest>

#include <QtCrypto>

#include "jabber/Jabber.h"

#include "JabberTest.h"

void JabberTest::initTestCase() {
	
	m_qcaInitializer = new QCA::Initializer();
}

void JabberTest::init() {
	
	m_authenticated = false;
}

void JabberTest::testConnection() {
	
	QString resource = "desktopClient";
	QString host = "85.17.225.209";
	int port = 5222;
	
	Jabber::Jabber *jabber = Jabber::Jabber::instance();
	jabber->setProtocolVersion(Jabber::xmpp09);
	jabber->setHostAndPort(host, port);
	jabber->setDigest("4768630eb3dd69d7101c5c7fc96cda02");
	jabber->signIn("arendjr", "hyves.nl", "", resource);
	
	connect(jabber, SIGNAL(connected()), SLOT(connected()));
	
	QTest::qWait(2000);
	
	Q_ASSERT(m_authenticated);
	
	Jabber::Jabber::destroy();
}

void JabberTest::cleanup() {
}

void JabberTest::cleanupTestCase() {
	
	delete m_qcaInitializer;
}

void JabberTest::connected() {
	
	m_authenticated = true;
}

QTEST_MAIN(JabberTest)
