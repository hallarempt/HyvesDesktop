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

#include <QtTest>
#include <QUrl>

#include "UrlFetcher.h"
#include "UrlFetcherTest.h"

const QUrl TEST_URL("http://www.hyves.nl");

void UrlFetcherTest::testCreation() {
	
	UrlFetcher f(TEST_URL, UrlFetcher::FetchString);
}

void UrlFetcherTest::testFetch() {
	
	UrlFetcher f(TEST_URL, UrlFetcher::FetchString);
	connect(&f, SIGNAL(replyReady(const QString &)), this, SLOT(fetched(const QString &)));
	QSignalSpy spy(&f, SIGNAL(replyReady(const QString &)));
	QVERIFY(spy.isValid());
	QCOMPARE(spy.count(), 0);
	while (spy.count() == 0) {
		QTest::qWait(250);
	}
	QCOMPARE(spy.count(), 1);
	QVERIFY(!fetch.isNull());
}

void UrlFetcherTest::testNoFetch() {
	
	UrlFetcher f(QUrl("blablabla"), UrlFetcher::FetchString);
	connect(&f, SIGNAL(replyReady(const QString &)), this, SLOT(fetched(const QString &)));
	QSignalSpy spy(&f, SIGNAL(replyReady(const QString &)));
	QVERIFY(spy.isValid());
	QCOMPARE(spy.count(), 0);
	while (spy.count() == 0) {
		QTest::qWait(250);
	}
	QCOMPARE(spy.count(), 1);
	QVERIFY(fetch.isEmpty());
}

void UrlFetcherTest::fetched(const QString &reply) {
	
	fetch = reply;
}

QTEST_MAIN(UrlFetcherTest);
