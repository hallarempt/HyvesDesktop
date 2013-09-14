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

#include "roster/Roster.h"
#include "roster/RosterItem.h"

#include "RosterTest.h"

struct RosterTest::Private {
	
	Roster::Roster *roster;
	bool initialized;
	
	QVariantMap item;
	QVariantMap changedProperties;
};

RosterTest::RosterTest() :
	QObject(),
	m_d(new Private()) {
}

RosterTest::~RosterTest() {
	
	delete m_d;
}

void RosterTest::init() {
	
	m_d->roster = Roster::Roster::instance();
	m_d->initialized = false;
	
	m_d->item = QVariantMap();
	m_d->changedProperties = QVariantMap();
	
	connect(m_d->roster, SIGNAL(initialized()), SLOT(initialized()));
	connect(m_d->roster, SIGNAL(itemChanged(QVariantMap, QVariantMap)), SLOT(itemChanged(QVariantMap, QVariantMap)));
}

void RosterTest::testCreateItem() {
	
	QVERIFY(!m_d->roster->hasItem("arendjr@hyves.nl"));
	
	Roster::RosterItem *item = m_d->roster->item("arendjr@hyves.nl");
	QVERIFY(item != 0);
	QCOMPARE(item->jid(), QString("arendjr@hyves.nl"));
	
	QVERIFY(m_d->roster->hasItem("arendjr@hyves.nl"));
	
	item->unlock();
	item->setProperty("nick", "Arend");
	item->setProperty("memberId", "96566");
	item->lock();
	
	// test the item is still empty, the signal should not have been emitted
	QCOMPARE(m_d->item["jid"], QVariant());
	
	QString expectedJSON = "{ \"available\": false, \"availableClients\": 0, \"jid\": \"arendjr@hyves.nl\", \"memberId\": \"96566\", \"nick\": \"Arend\", \"show\": \"\", \"status\": \"\", \"statusText\": \"" + tr("Online") + "\", \"userhome\": \"arendjr.hyves.nl\", \"username\": \"arendjr\" }";
	QCOMPARE(m_d->roster->itemJSON("arendjr@hyves.nl"), expectedJSON);
	
	expectedJSON = "{ \"resources\": [  ] }";
	QCOMPARE(m_d->roster->itemResourcesJSON("arendjr@hyves.nl"), expectedJSON);
	
	QVERIFY(!m_d->initialized);
	
	item->unlock();
	item->addResource(Roster::Resource("desktopClient_1234", "normal", "online", 8));
	item->lock();
	
	QCOMPARE(m_d->item["jid"], QVariant());
	
	QCOMPARE(item->hasAvailableResource(), true);
	QCOMPARE(item->resources().size(), 1);
	QCOMPARE(item->resource("desktopClient_1234").id, QString("desktopClient_1234"));
	QCOMPARE(item->resource("desktopClient_1234").show, QString("normal"));
	QCOMPARE(item->resource("desktopClient_1234").status, QString("online"));
	QCOMPARE(item->resource("desktopClient_1234").priority, 8);
	
	expectedJSON = "{ \"available\": true, \"availableClients\": 4, \"jid\": \"arendjr@hyves.nl\", \"memberId\": \"96566\", \"nick\": \"Arend\", \"show\": \"normal\", \"status\": \"online\", \"statusText\": \"" + tr("Online") + "\", \"userhome\": \"arendjr.hyves.nl\", \"username\": \"arendjr\" }";
	QCOMPARE(m_d->roster->itemJSON("arendjr@hyves.nl"), expectedJSON);
	
	expectedJSON = "{ \"resources\": [ { \"id\": \"desktopClient_1234\", \"priority\": 8, \"show\": \"normal\", \"status\": \"online\" } ] }";
	QCOMPARE(m_d->roster->itemResourcesJSON("arendjr@hyves.nl"), expectedJSON);
	
	item->removeResource("desktopClient_1234"); // should fail because the item is locked
	
	QCOMPARE(m_d->roster->itemResourcesJSON("arendjr@hyves.nl"), expectedJSON);
	
	item->unlock();
	item->removeResource("siteClient");
	item->removeResource("desktopClient_1234");
	item->lock();
	
	expectedJSON = "{ \"resources\": [  ] }";
	QCOMPARE(m_d->roster->itemResourcesJSON("arendjr@hyves.nl"), expectedJSON);
	
	m_d->roster->removeItem("arendjr@hyves.nl");
	
	QVERIFY(!m_d->roster->hasItem("arendjr@hyves.nl"));
}

void RosterTest::testUpdateItem() {
	
	m_d->roster->setInitialized();
	QVERIFY(m_d->initialized);
	
	Roster::RosterItem *item = m_d->roster->item("arendjr@hyves.nl");
	QVERIFY(item != 0);
	QCOMPARE(item->jid(), QString("arendjr@hyves.nl"));
	
	item->unlock();
	item->setProperty("nick", "Arend");
	item->setProperty("memberId", "96566");
	item->lock();
	
	QCOMPARE(m_d->item["jid"].toString(), QString("arendjr@hyves.nl"));
	QCOMPARE(m_d->item["memberId"].toString(), QString("96566"));
	QCOMPARE(m_d->item["nick"].toString(), QString("Arend"));
	
	QCOMPARE(m_d->changedProperties["memberId"].toString(), QString(""));
	QCOMPARE(m_d->changedProperties["nick"].toString(), QString(""));
	
	m_d->roster->setItemProperties("arendjr@hyves.nl", "{ \"nick\": \"Arend :kilian:\" }");
	
	QCOMPARE(m_d->item["jid"].toString(), QString("arendjr@hyves.nl"));
	QCOMPARE(m_d->item["memberId"].toString(), QString("96566"));
	QCOMPARE(m_d->item["nick"].toString(), QString("Arend :kilian:"));
	
	QVERIFY(!m_d->changedProperties.contains("memberId"));
	QCOMPARE(m_d->changedProperties["nick"].toString(), QString("Arend"));
	
	QString expectedJSON = "{ \"available\": false, \"availableClients\": 0, \"jid\": \"arendjr@hyves.nl\", \"memberId\": \"96566\", \"nick\": \"Arend :kilian:\", \"show\": \"\", \"status\": \"\", \"statusText\": \"" + tr("Online") + "\", \"userhome\": \"arendjr.hyves.nl\", \"username\": \"arendjr\" }";
	QCOMPARE(m_d->roster->itemJSON("arendjr@hyves.nl"), expectedJSON);
	
	item->unlock();
	item->addResource(Roster::Resource("desktopClient_1234", "normal", "online", 8));
	item->lock();
	
	QVERIFY(m_d->changedProperties.contains("available"));
	QVERIFY(m_d->changedProperties.contains("status"));
	QCOMPARE(m_d->changedProperties["available"].toBool(), false);
	QCOMPARE(m_d->changedProperties["status"].toString(), QString(""));
	
	QCOMPARE(m_d->item["available"].toBool(), true);
	QCOMPARE(m_d->item["status"].toString(), QString("online"));
	
	item->unlock();
	item->addResource(Roster::Resource("desktopClient_1234", "away", "away", 8));
	item->lock();
	
	QVERIFY(!m_d->changedProperties.contains("available"));
	QVERIFY(m_d->changedProperties.contains("status"));
	QCOMPARE(m_d->changedProperties["status"].toString(), QString("online"));
	
	QCOMPARE(m_d->item["available"].toBool(), true);
	QCOMPARE(m_d->item["status"].toString(), QString("away"));
	
	QCOMPARE(item->resources().size(), 1);
	
	QVERIFY(m_d->roster->hasItem("arendjr@hyves.nl"));
	
	m_d->roster->clear();
	
	QVERIFY(!m_d->roster->hasItem("arendjr@hyves.nl"));
	
	QCOMPARE(m_d->roster->isInitialized(), false);
}

void RosterTest::testStatus() {
	
	QCOMPARE(m_d->roster->statusExists("online"), true);
	QCOMPARE(m_d->roster->statusExists("offline"), true);
	QCOMPARE(m_d->roster->statusExists("phone"), true);
	QCOMPARE(m_d->roster->statusExists("sleep"), false);
	
	QCOMPARE(m_d->roster->statusShow("online"), QString("normal"));
	QCOMPARE(m_d->roster->statusShow("offline"), QString("offline"));
	QCOMPARE(m_d->roster->statusShow("phone"), QString("dnd"));
	QCOMPARE(m_d->roster->statusShow("sleep"), QString("normal"));
	
	QCOMPARE(m_d->roster->statusText("online"), tr("Online"));
	QCOMPARE(m_d->roster->statusText("offline"), tr("Offline"));
	QCOMPARE(m_d->roster->statusText("away"), tr("Away"));
	QCOMPARE(m_d->roster->statusText("sleep"), tr("Online"));
}

void RosterTest::testOwnJid() {
	
	m_d->roster->setOwnJid("arendjr@hyves.nl/siteClient");
	
	QCOMPARE(m_d->roster->ownBareJid(), QString("arendjr@hyves.nl"));
	QCOMPARE(m_d->roster->ownResource(), QString("siteClient"));
	QCOMPARE(m_d->roster->ownFullJid(), QString("arendjr@hyves.nl/siteClient"));
	
	Roster::RosterItem *item = m_d->roster->item("arendjr@hyves.nl");
	QVERIFY(item != 0);
	QCOMPARE(item->jid(), QString("arendjr@hyves.nl"));
	QCOMPARE(item->resources().size(), 1);
	QCOMPARE(item->resource().id, QString("siteClient"));
	QCOMPARE(item->resource().status, QString(""));
	QCOMPARE(item->resource().show, QString(""));
	
	m_d->roster->setOwnStatus("busy");
	
	QCOMPARE(item->resource().status, QString("busy"));
	QCOMPARE(item->resource().show, QString("dnd"));
	
	QString expectedJSON = "{ \"available\": true, \"availableClients\": 1, \"jid\": \"arendjr@hyves.nl\", \"show\": \"dnd\", \"status\": \"busy\", \"statusText\": \"" + tr("Busy") + "\", \"userhome\": \"arendjr.hyves.nl\", \"username\": \"arendjr\" }";
	QCOMPARE(m_d->roster->ownItemJSON(), expectedJSON);
	
	m_d->roster->clear();
	
	QCOMPARE(m_d->roster->ownBareJid(), QString(""));
	QCOMPARE(m_d->roster->ownResource(), QString(""));
	QCOMPARE(m_d->roster->ownFullJid(), QString(""));
}

void RosterTest::testEnsureItem() {
	
	bool result = m_d->roster->ensureItem("arendjr@hyves.nl");
	QCOMPARE(result, true);
	
	Roster::RosterItem *item = m_d->roster->item("arendjr@hyves.nl");
	QVERIFY(item != 0);
	QCOMPARE(item->jid(), QString("arendjr@hyves.nl"));
	QCOMPARE(item->property("nick"), QString("arendjr@hyves.nl"));
	QCOMPARE(item->resources().size(), 1);
	QCOMPARE(item->resource().id, QString("unknown"));
	QCOMPARE(item->resource().status, QString("online"));
	QCOMPARE(item->resource().show, QString("normal"));
	
	result = m_d->roster->ensureItem("arendjr@hyves.nl");
	QCOMPARE(result, false);
}

void RosterTest::testAllItems() {
	
	Roster::RosterItem *item = m_d->roster->item("arendjr@hyves.nl");
	item->unlock();
	item->setProperty("nick", "Arend");
	item->setProperty("memberId", "96566");
	item->addResource(Roster::Resource("desktopClient_1234", "normal", "online", 8));
	item->lock();
	
	item = m_d->roster->item("optroodt@hyves.nl");
	item->unlock();
	item->setProperty("nick", "Youri");
	item->setProperty("memberId", "7311");
	item->addResource(Roster::Resource("siteClient", "normal", "online", 4));
	item->lock();
	
	QString expectedJSON = "{ \"arendjr@hyves.nl\": { \"available\": true, \"availableClients\": 4, \"jid\": \"arendjr@hyves.nl\", \"memberId\": \"96566\", \"nick\": \"Arend\", \"show\": \"normal\", \"status\": \"online\", \"statusText\": \"" + tr("Online") + "\", \"userhome\": \"arendjr.hyves.nl\", \"username\": \"arendjr\" }, "
	                         "\"optroodt@hyves.nl\": { \"available\": true, \"availableClients\": 1, \"jid\": \"optroodt@hyves.nl\", \"memberId\": \"7311\", \"nick\": \"Youri\", \"show\": \"normal\", \"status\": \"online\", \"statusText\": \"" + tr("Online") + "\", \"userhome\": \"optroodt.hyves.nl\", \"username\": \"optroodt\" } }";
	QCOMPARE(m_d->roster->itemsJSON(), expectedJSON);
	
	item->unlock();
	item->addResource(Roster::Resource("siteClient", "away", "away", 4));
	item->lock();
	
	expectedJSON = "{ \"arendjr@hyves.nl\": { \"available\": true, \"availableClients\": 4, \"jid\": \"arendjr@hyves.nl\", \"memberId\": \"96566\", \"nick\": \"Arend\", \"show\": \"normal\", \"status\": \"online\", \"statusText\": \"" + tr("Online") + "\", \"userhome\": \"arendjr.hyves.nl\", \"username\": \"arendjr\" }, "
	                 "\"optroodt@hyves.nl\": { \"available\": true, \"availableClients\": 1, \"jid\": \"optroodt@hyves.nl\", \"memberId\": \"7311\", \"nick\": \"Youri\", \"show\": \"away\", \"status\": \"away\", \"statusText\": \"" + tr("Away") + "\", \"userhome\": \"optroodt.hyves.nl\", \"username\": \"optroodt\" } }";
	QCOMPARE(m_d->roster->itemsJSON(), expectedJSON);
}

void RosterTest::cleanup() {
	
	Roster::Roster::destroy();
}

void RosterTest::initialized() {
	
	m_d->initialized = true;
}

void RosterTest::itemChanged(const QVariantMap &item, const QVariantMap &changedProperties) {
	
	m_d->item = item;
	m_d->changedProperties = changedProperties;
}

QTEST_MAIN(RosterTest)
