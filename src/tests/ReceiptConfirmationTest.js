/*
 * Hyves Desktop, Copyright (C) 2009 Hyves (Startphone Ltd.)
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


var globalMainWindowId = 1;
var globalChatWindowId = 0;

/**
 * Sets up a session with Youri.
 */
function testSetupSessionWithYouri() {
	
	simulateClick('dock_roster_item_optroodt');
	
	sleep(2);
	
	globalChatWindowId = window.container.sessionWindows['optroodt@hyves.nl'];
	
	assertMoreThan('globalChatWindowId', 1);
	
	window.tester.setTestWindowForNextMethod(globalChatWindowId);
}

/**
 * Tests whether we request a confirmation for the first message, and whether we
 * get a confirmation from Youri.
 */
function testConfirmationFromYouri() {
	
	window.xmppSession.rte.value = 'test1';
	$('uiButtonSend').disabled = false;
	$('uiButtonSend').click();
	
	sleep(2);
	
	window.xmppSession.rte.value = 'test2';
	$('uiButtonSend').disabled = false;
	$('uiButtonSend').click();
	
	sleep(2);
	
	assertEqual('window.jabber.sentMessage(0)', '<message id="m_1" to="optroodt@hyves.nl/desktopClient" type=\'chat\'><body>test2</body><active xmlns=\'http://jabber.org/protocol/chatstates\'/></message>');
	assertEqual('window.jabber.sentMessage(1)', '<message id="m_1" to="optroodt@hyves.nl/desktopClient" type=\'chat\'><body>test1</body><active xmlns=\'http://jabber.org/protocol/chatstates\'/><request xmlns=\'urn:xmpp:receipts\'/></message>');
	
	window.jabber.incomingXml('<message id="m_1" from="optroodt@hyves.nl/desktopClient" to="arendjr@hyves.nl" type=\'chat\'><received xmlns=\'urn:xmpp:receipts\'/></message>');
	
	sleep(10);
	
	assertEqual("$('chatsession_statusmessage').innerHTML", '');
	
	window.tester.setTestWindowForNextMethod(globalMainWindowId);
}

/**
 * Sets up a session with Henk (and closes the one with Youri).
 */
function testSetupSessionWithHenk() {
	
	window.windowManager.closeWindow(globalChatWindowId);
	
	simulateClick('dock_roster_item_toymachine');
	
	sleep(2);
	
	globalChatWindowId = window.container.sessionWindows['toymachine@hyves.nl'];
	
	assertMoreThan('globalChatWindowId', 1);
	
	window.tester.setTestWindowForNextMethod(globalChatWindowId);
}

/**
 * Tests whether we display a warning when we don't get a confirmation from
 * Henk.
 */
function testNoConfirmationFromHenk() {
	
	window.xmppSession.rte.value = 'test1';
	$('uiButtonSend').disabled = false;
	$('uiButtonSend').click();
	
	sleep(2);
	
	assertEqual('window.jabber.sentMessage(0)', '<message id="m_1" to="toymachine@hyves.nl/desktopClient_1234" type=\'chat\'><body>test1</body><active xmlns=\'http://jabber.org/protocol/chatstates\'/><request xmlns=\'urn:xmpp:receipts\'/></message>');
	
	sleep(10);
	
	// enable later when enough users have upgraded and statistics are verified
	//assertEqual("$('chatsession_statusmessage').innerHTML", tr("Could not connect to %1. %1 may not receive your messages.").arg("blaatpuk &lt;script&gt;alert('TypeError: Gotcha!');&lt;/script&gt;"));
	
	window.tester.setTestWindowForNextMethod(globalMainWindowId);
}

/**
 * Sets up a session with Kilian (and closes the one with Henk).
 */
function testSetupSessionWithKilian() {
	
	window.windowManager.closeWindow(globalChatWindowId);
	
	simulateClick('dock_roster_item_kilian');
	
	sleep(2);
	
	globalChatWindowId = window.container.sessionWindows['kilian@hyves.nl'];
	
	assertMoreThan('globalChatWindowId', 1);
	
	window.tester.setTestWindowForNextMethod(globalChatWindowId);
}

/**
 * Tests whether we don't send a confirmation request to Kilian, who's using
 * the site client.
 */
function testNoConfirmationRequestToKilian() {
	
	window.xmppSession.rte.value = 'test1';
	$('uiButtonSend').disabled = false;
	$('uiButtonSend').click();
	
	sleep(2);
	
	assertEqual('window.jabber.sentMessage(0)', '<message id="m_1" to="kilian@hyves.nl" type=\'chat\'><body>test1</body><active xmlns=\'http://jabber.org/protocol/chatstates\'/></message>');
	
	sleep(10);
	
	assertEqual("$('chatsession_statusmessage').innerHTML", tr("%1 is online at the website and may not respond.").arg(' Kilian'));
	
	window.tester.setTestWindowForNextMethod(globalMainWindowId);
}
