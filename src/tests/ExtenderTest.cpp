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

#include <QtWebKit>
#include <QTest>

#include "extender/Extender.h"

#include "ExtenderTest.h"

void ExtenderTest::init() {
	
	Extender::Extender::instance()->registerObject("extendertest", this);
}

void ExtenderTest::testWebFrameEnvironment() {
	
	Extender::Extender *extender = Extender::Extender::instance();
	
	QWebPage webPage;
	QWebFrame *webFrame = webPage.mainFrame();
	extender->registerEnvironment(webFrame);
	
	{
		webFrame->evaluateJavaScript("var slot1Called = false; "
			                         "function slot1() { "
		                             "  slot1Called = true; "
		                             "} "
		                             "extender.connect('extendertest', 'signal1()', 'slot1');");
		
		emit signal1();
		QCOMPARE(webFrame->evaluateJavaScript("slot1Called").toBool(), true);
	}
	
	{
		webFrame->evaluateJavaScript("var slot2Value = 0; "
			                         "function slot2(value) { "
		                             "  slot2Value = value; "
		                             "} "
		                             "extender.connect('extendertest', 'signal2(int)', 'slot2');");
		
		emit signal2(42);
		QCOMPARE(webFrame->evaluateJavaScript("slot2Value").toInt(), 42);
	}
	
	{
		webFrame->evaluateJavaScript("var slot3String = ''; "
			                         "function slot3(string) { "
		                             "  slot3String = string; "
		                             "} "
		                             "extender.connect('extendertest', 'signal3(QString)', 'slot3');");
		
		QString testString = "\"shizzle ma nizzle\", she said";
		emit signal3(testString);
		QCOMPARE(webFrame->evaluateJavaScript("slot3String").toString(), testString);
	}
	
	{
		webFrame->evaluateJavaScript("var slot4String = ''; "
			                         "function slot4(string) { "
		                             "  slot4String = string; "
		                             "} "
		                             "extender.connect('extendertest', 'signal4(QDomDocument)', 'slot4');");
		
		QString xmlString = "<iq type=\"result\" to=\"arendjr@hyves.nl/desktopClient\" id=\"roster_5\" >"
		                      "<query xmlns=\"jabber:iq:roster\">"
		                        "<item subscription=\"both\" name=\"Floris\" jid=\"florisrost@hyves.nl\" >"
		                          "<group>Friends</group>"
		                        "</item>"
		                        "<item subscription=\"both\" name=\"Boris! \" jid=\"boris@hyves.nl\" >"
		                           "<group>Friends</group>"
		                        "</item>"
		                        "<item subscription=\"both\" name=\"&quot;taco chief&quot; michel\" jid=\"taco-chief@hyves.nl\" >"
		                           "<group>Friends</group>"
		                        "</item>"
		                      "</query>"
		                    "</iq>";
		QString jsonString = "{ \"iq\": { \"@id\": \"roster_5\", \"@to\": \"arendjr@hyves.nl/desktopClient\", \"@type\": \"result\", "
		                                 "\"query\": { \"@xmlns\": \"jabber:iq:roster\", "
		                                	          "\"item\": [ { \"@jid\": \"florisrost@hyves.nl\", \"@name\": \"Floris\", \"@subscription\": \"both\", \"group\": \"Friends\" }, "
		                                                          "{ \"@jid\": \"boris@hyves.nl\", \"@name\": \"Boris! \", \"@subscription\": \"both\", \"group\": \"Friends\" }, "
		                                                          "{ \"@jid\": \"taco-chief@hyves.nl\", \"@name\": \"\\\"taco chief\\\" michel\", \"@subscription\": \"both\", \"group\": \"Friends\" } ] } } }";
		
		QDomDocument document;
		document.setContent(xmlString);
		emit signal4(document);
		QCOMPARE(webFrame->evaluateJavaScript("slot4String").toString(), jsonString);
	}
	
	{
		webFrame->evaluateJavaScript("var slot5String = ''; "
			                         "function slot5(list) { "
		                             "  slot5List = list; "
		                             "} "
		                             "extender.connect('extendertest', 'signal5(QVariantList)', 'slot5');");
		
		QVariantList testList;
		testList << QVariant("The magic answer is: ") << QVariant(42);
		emit signal5(testList);
		QCOMPARE(webFrame->evaluateJavaScript("slot5List[0] + slot5List[1]").toString(),
		         QString("The magic answer is: 42"));
	}
}

void ExtenderTest::cleanup() {
	
	Extender::Extender::destroy();
}

QTEST_MAIN(ExtenderTest)
