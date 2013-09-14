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

#include "Version.h"
#include "VersionTest.h"

void VersionTest::testCreation() {
	
	Version v;
	QCOMPARE(v.major, 0);
	QCOMPARE(v.minor, 0);
	QCOMPARE(v.patch, 0);
	
	v = Version("1.1.1");
	QCOMPARE(v.major, 1);
	QCOMPARE(v.minor, 1);
	QCOMPARE(v.patch, 1);
	
	v = Version("1");
	QCOMPARE(v.major, 1);
	QCOMPARE(v.minor, 0);
	QCOMPARE(v.patch, 0);
	
	v = Version("1.1");
	QCOMPARE(v.major, 1);
	QCOMPARE(v.minor, 1);
	QCOMPARE(v.patch, 0);
	
	v = Version("1.1.1");
	QCOMPARE(v.major, 1);
	QCOMPARE(v.minor, 1);
	QCOMPARE(v.patch, 1);
	
	v = Version("1.1.1.1.1");
	QCOMPARE(v.major, 1);
	QCOMPARE(v.minor, 1);
	QCOMPARE(v.patch, 1);
	
	v = Version("1.1.1b");
	QCOMPARE(v.major, 1);
	QCOMPARE(v.minor, 1);
	QCOMPARE(v.patch, 0);
}

void VersionTest::testToString() {
	
	Version v;
	v.major = 10;
	v.minor = 5;
	v.patch = 0;
	QCOMPARE(v.toString(), QString("10.5.0"));
}

void VersionTest::testOperators() {
	
	QVERIFY(Version("0.0.1") == Version("0.0.1"));
	QVERIFY(!(Version("0.0.0") == Version("0.0.1")));
	QVERIFY(!(Version("0.1.0") == Version("0.0.1")));
	QVERIFY(!(Version("1.0.0") == Version("0.0.1")));
	
	QVERIFY(Version("0.0.1") != Version("0.1.0"));
	QVERIFY(!(Version("0.0.0") != Version("0.0.0")));
	QVERIFY((Version("1.0.0") != Version("0.0.0")));
	QVERIFY((Version("1.0.0") != Version("0.0.1")));
	
	QVERIFY(!(Version("0.0.0") >= Version("0.0.1")));
	QVERIFY(Version("0.0.1") >= Version("0.0.1"));
	QVERIFY(Version("0.0.2") >= Version("0.0.1"));
	QVERIFY(Version("0.1.0") >= Version("0.0.1"));
	QVERIFY(Version("1.0.0") >= Version("0.0.1"));
	QVERIFY(Version("0.0.1") >= Version("0.0.1"));
	
	QVERIFY(!(Version("0.0.0") > Version("0.0.1")));
	QVERIFY(!(Version("0.0.1") > Version("0.0.1")));
		
	QVERIFY(Version("0.0.2") > Version("0.0.1"));
	QVERIFY(Version("0.1.0") > Version("0.0.1"));
	QVERIFY(Version("1.0.0") > Version("0.0.1"));
	QVERIFY(Version("0.1.1") > Version("0.0.1"));
	
	QVERIFY(Version("0.3.1") > Version("0.2.1"));
	QVERIFY(Version("0.3.1") > Version("0.2.0"));
	QVERIFY(Version("3.0.1") > Version("0.0.1"));
}
	
QTEST_MAIN(VersionTest);
