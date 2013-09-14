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

#include <QTest>

#include "Util.h"
#include "UtilTest.h"

void UtilTest::testVariantMapToJSON() {
	
	QVariantMap variantMap;
	variantMap["name"] = "messageReceived";
	variantMap["jid"] = "kilian@hyves.nl";
	variantMap["message"] = "test \"test\"\nhoe is't ermee?\n/me ducks\\";
	
	QString expectedValue = "{ \"jid\": \"kilian@hyves.nl\", \"message\": \"test \\\"test\\\"\\nhoe is't ermee?\\n/me ducks\\\\\", \"name\": \"messageReceived\" }";
	QCOMPARE(Util::variantMapToJSON(variantMap), expectedValue);
}

void UtilTest::testJSONToVariantMap() {
	
	QString json = "{ \"jid\": \"kilian@hyves.nl\", \"message\": \"test \\\"test\\\"\\nhoe is't ermee?\\n/me ducks\\\\\", \"name\": \"messageReceived\" }";
	QVariantMap variantMap = Util::jsonToVariantMap(json);
	
	QCOMPARE(variantMap.size(), 3);
	QCOMPARE(variantMap["name"].toString(), QString("messageReceived"));
	QCOMPARE(variantMap["jid"].toString(), QString("kilian@hyves.nl"));
	QCOMPARE(variantMap["message"].toString(), QString("test \"test\"\nhoe is't ermee?\n/me ducks\\"));
}

QTEST_MAIN(UtilTest)
