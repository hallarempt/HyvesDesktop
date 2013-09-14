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


/*
 * MUCSessionTest.js
 *
 * This test suite checks proper behavior for multi-user chats:
 * - inviting a contact into a one-to-one session
 * - the other contact inviting someone into a one-to-one session
 * - getting invited into a multi-user session
 * - leaving a MUC session
 * - other participants leaving a MUC session
 * - two participants being invited at the same time
 * - ...
 */

var globalMainWindowId = 1;
var globalChatWindowId = 0;

/**
 * Returns the last message received in the chat session.
 */
function getLatestDisplayedMessage() {
	
	var message = 'null';
	
	var messageContainer = $('chatsession').down();
	while (messageContainer && messageContainer.next()) {
		messageContainer = messageContainer.next();
	}
	if (messageContainer) {
		message = messageContainer.down().next().innerHTML;
	}
	
	return message;
}

/**
 * Returns the window ID of the last opened session window.
 */
function getLastOpenedSessionWindowId() {
	
	var windowId = 0;
	for (var id in window.container.windowToSessionMap) {
		if (id > windowId) {
			windowId = id;
		}
	}
	return windowId;
}

/**
 * Checks that the chat send button is disabled for the user (greyed out).
 */
function isSendButtonDisabled() {
	
	return $('uiButtonSend').hasClassName('disabled');
}

/**
 * Sets up the session we're going to use.
 */
function testSetupMUCSession() {
	
	window.jabber.setMucMessageBody(mucMessageBody);
	
	simulateClick('dock_roster_item_optroodt');
	
	sleep(2);
	
	globalChatWindowId = window.container.sessionWindows['optroodt@hyves.nl'];
	
	assertMoreThan('globalChatWindowId', 1);
	
	window.tester.setTestWindowForNextMethod(globalChatWindowId);
}

/**
 * Opens the invite window to invite another user.
 */
function testOpenInvitationWindow() {
	
	simulateClick('inviteLink');
	
	sleep(2);
	
	assertMoreThan('window.xmppSession.inviteWindowId', 1);
	
	window.tester.setTestWindowForNextMethod(window.xmppSession.inviteWindowId);
}

/**
 * Tests the contents of the invite window.
 */
function testInviteWindow() {
	
	assertEqual('$(\'uiRosterListOnline_optroodt@hyves.nl\')', null);
	assert('$(\'uiRosterListOnline_kilian@hyves.nl\')');
	assert('$(\'uiRosterListOnline_toymachine@hyves.nl\')');
	
	simulateClick('dock_roster_item_toymachine');
	
	window.tester.setTestWindowForNextMethod(window.invitationManager.sessionWindowId);
}

/**
 * Tests whether the contact gets invited.
 */
function testContactInvited() {
	
	sleep(2);
	
	assertEqual('getLatestDisplayedMessage()', 'blaatpuk &lt;script&gt;alert(\'TypeError: Gotcha!\');&lt;/script&gt; has joined the conversation.');
	
	assertEqual('$(\'inviteLink\').style.display', 'inline');
	assertEqual('$(\'historyLink\').style.display', 'none');
}

/**
 * Opens the invite window to invite one more user.
 */
function testOpenInvitationWindow2() {
	
	simulateClick('inviteLink');
	
	sleep(2);
	
	assertMoreThan('window.xmppSession.inviteWindowId', 1);
	
	window.tester.setTestWindowForNextMethod(window.xmppSession.inviteWindowId);
}

/**
 * Tests the contents of the invite window.
 */
function testInviteWindow2() {
	
	assertEqual('$(\'uiRosterListOnline_optroodt@hyves.nl\')', null);
	assert('$(\'uiRosterListOnline_kilian@hyves.nl\')');
	assertEqual('$(\'uiRosterListOnline_toymachine@hyves.nl\')', null);
	
	simulateClick('dock_roster_item_kilian');
	
	window.tester.setTestWindowForNextMethod(window.invitationManager.sessionWindowId);
}

/**
 * Tests whether the contact invitiation aborts (since Kilian is not using Hyves
 * Desktop).
 */
function testContactNotInvited() {
	
	sleep(11);
	
	assertEqual('$(\'chatsession_statusmessage\').innerHTML', ' Kilian could not be invited.');
	
	window.tester.setTestWindowForNextMethod(globalMainWindowId);
}

/**
 * Tests whether we can also open a one-to-one session with a user that we are
 * already in a MUC session with.
 */
function testOpenOneToOneSession() {
	
	assertUndefined('window.container.sessionWindows[\'optroodt@hyves.nl\']');
	
	simulateClick('dock_roster_item_optroodt');
	
	sleep(2);
	
	assertMoreThan('window.container.sessionWindows[\'optroodt@hyves.nl\']', globalChatWindowId);
	assertEqual('window.container.sessionWindowInitialized[\'optroodt@hyves.nl\']', true);
	
	window.tester.setTestWindowForNextMethod(globalChatWindowId);
}

/**
 * Tests whether the messages in a MUC session are sent to both contacts.
 */
function testSendMessage() {
	
	window.xmppSession.rte.value = 'testing MUC message';
	simulateClick('uiButtonSend');
	
	sleep(1);
	
	assertEqual('window.jabber.sentMessage(0)', '<message id="m_1" to="toymachine@hyves.nl/desktopClient_1234" toSessionId="12345678" type=\'chat\'><body>testing MUC message</body><active xmlns=\'http://jabber.org/protocol/chatstates\'/><request xmlns=\'urn:xmpp:receipts\'/></message>');
	assertEqual('window.jabber.sentMessage(1)', '<message id="m_1" to="optroodt@hyves.nl/desktopClient" toSessionId="87654321" type=\'chat\'><body>testing MUC message</body><active xmlns=\'http://jabber.org/protocol/chatstates\'/><request xmlns=\'urn:xmpp:receipts\'/></message>');
}

/**
 * Tests whether an incoming message in a MUC session is displayed correctly.
 */
function testIncomingMucMessage() {
	
	window.jabber.incomingXml('<message from="optroodt@hyves.nl/desktopClient" to="' + window.roster.ownFullJid() + '" toSessionId="' + window.xmppSession.sessionId + '" type=\'chat\' id=\'m_3\'><body>MUC reply!</body><active xmlns=\'http://jabber.org/protocol/chatstates\'/></message>');
	
	sleep(1);
	
	assertEqual('getLatestDisplayedMessage()', '<p>MUC reply!</p>');
}

/**
 * Tests whether an incoming message in a one-to-one session doesn't appear in
 * the MUC session.
 */
function testIncomingOneToOneMessage() {
	
	window.jabber.incomingXml('<message from="toymachine@hyves.nl/desktopClient_1234" to="' + window.roster.ownBareJid() + '" type=\'chat\' id=\'m_1\'><body>one-to-one message</body><active xmlns=\'http://jabber.org/protocol/chatstates\'/></message>');
	
	sleep(1);
	
	assertEqual('getLatestDisplayedMessage()', '<p>MUC reply!</p>');
	
	window.tester.setTestWindowForNextMethod(globalMainWindowId);
}

/**
 * Tests the closing of a MUC session window and checks whether we sent messages
 * to the other participants that we left the session.
 */
function testCloseMUCWindow() {
	
	assertEqual('window.container.remoteSessionIds[\'optroodt@hyves.nl/desktopClient\'][\'87654321\']', globalChatWindowId);
	assertEqual('window.container.remoteSessionIds[\'toymachine@hyves.nl/desktopClient_1234\'][\'12345678\']', globalChatWindowId);
	
	var fromSessionId = window.container.windowToSessionMap[globalChatWindowId];
	
	window.windowManager.closeWindow(globalChatWindowId);
	
	sleep(1);
	
	assertEqual('window.jabber.sentMessage(0)', '<message id="m_1" to="toymachine@hyves.nl/desktopClient_1234" type="chat"><sessionclosed xmlns="http://hyves.net/xmpp" toSessionId="12345678" fromSessionId="' + fromSessionId + '" /></message>');
	assertEqual('window.jabber.sentMessage(1)', '<message id="m_1" to="optroodt@hyves.nl/desktopClient" type="chat"><sessionclosed xmlns="http://hyves.net/xmpp" toSessionId="87654321" fromSessionId="' + fromSessionId + '" /></message>');
	
	assertUndefined('window.container.windowToSessionMap[' + globalChatWindowId + ']');
	assertUndefined('window.container.sessionToWindowMap[\'' + fromSessionId + '\']');
	assertUndefined('window.container.remoteSessionIds[\'optroodt@hyves.nl/desktopClient\'][\'87654321\']');
	assertUndefined('window.container.remoteSessionIds[\'toymachine@hyves.nl/desktopClient_1234\'][\'12345678\']');
	
	assertMoreThan('window.container.sessionWindows[\'optroodt@hyves.nl\']', globalChatWindowId);
	window.tester.setTestWindowForNextMethod(window.container.sessionWindows['optroodt@hyves.nl']);
}

/**
 * Make sure no messages have arrived in the one-to-one session with Youri (no
 * duplicate messages).
 */
function testNoMessagesInOneToOneSessionWithYouri() {
	
	assertEqual('window.xmppSession.messageReceived', false);
	
	window.tester.setTestWindowForNextMethod(globalMainWindowId);
}

/**
 * Closes the one-to-one session with Youri.
 */
function testCloseOneToOneSessionWithYouri() {
	
	window.windowManager.closeWindow(window.container.sessionWindows['optroodt@hyves.nl']);
	
	sleep(1);
	
	assertUndefined('window.container.sessionWindows[\'optroodt@hyves.nl\']');
	
	assertMoreThan('window.container.sessionWindows[\'toymachine@hyves.nl\']', globalChatWindowId);
	window.tester.setTestWindowForNextMethod(window.container.sessionWindows['toymachine@hyves.nl']);
}

/**
 * Make sure a message arrived in the one-to-one session with Henk.
 */
function testMessageInOneToOneSessionWithHenk() {
	
	assertEqual('getLatestDisplayedMessage()', '<p>one-to-one message</p>');
	
	window.tester.setTestWindowForNextMethod(globalMainWindowId);
}

/**
 * Closes the one-to-one session.
 */
function testCloseOneToOneSessionWithHenk() {
	
	window.windowManager.closeWindow(window.container.sessionWindows['toymachine@hyves.nl']);
	
	sleep(1);
	
	assertUndefined('window.container.sessionWindows[\'toymachine@hyves.nl\']');
}

/**
 * Now test the case when we are invited by someone else.
 */
function testIncomingInvitation() {
	
	window.jabber.incomingXml('<message id="m_1" from="optroodt@hyves.nl/desktopClient" to="' + window.roster.ownBareJid() + '" type="chat"><sessioninvite xmlns="http://hyves.net/xmpp" fromSessionId="44332211"><participants><participant jid="toymachine@hyves.nl/desktopClient_1234" sessionId=\"11223344\"/></participants></sessioninvite>' + mucMessageBody + '</message>');
	
	sleep(1);
	
	assertUndefined('window.container.sessionWindows[\'optroodt@hyves.nl\']');
	assertUndefined('window.container.sessionWindows[\'toymachine@hyves.nl\']');
	
	assertMoreThan('getLastOpenedSessionWindowId()', 1);
	globalChatWindowId = getLastOpenedSessionWindowId();
	
	assertEqual('window.container.remoteSessionIds[\'optroodt@hyves.nl/desktopClient\'][\'44332211\']', globalChatWindowId);
	assertEqual('window.container.remoteSessionIds[\'toymachine@hyves.nl/desktopClient_1234\'][\'11223344\']', globalChatWindowId);
	
	var fromSessionId = window.container.windowToSessionMap[globalChatWindowId];
	
	assertEqual('window.jabber.sentMessage(0)', '<message id="m_1" to="optroodt@hyves.nl/desktopClient" type="chat"><sessionjoined xmlns="http://hyves.net/xmpp" toSessionId="44332211" fromSessionId="' + fromSessionId + '" />' + mucMessageBody + '</message>');
	assertEqual('window.jabber.sentMessage(1)', '<message id="m_1" to="toymachine@hyves.nl/desktopClient_1234" type="chat"><sessionjoined xmlns="http://hyves.net/xmpp" toSessionId="11223344" fromSessionId="' + fromSessionId + '" />' + mucMessageBody + '</message>');
	
	window.tester.setTestWindowForNextMethod(globalChatWindowId);
}

/**
 * Tests whether the messages in a MUC session are sent to both contacts.
 */
function testSendMessage2() {
	
	window.xmppSession.rte.value = 'testing MUC message 2';
	simulateClick('uiButtonSend');
	
	sleep(1);
	
	assertEqual('window.jabber.sentMessage(0)', '<message id="m_1" to="optroodt@hyves.nl/desktopClient" toSessionId="44332211" type=\'chat\'><body>testing MUC message 2</body><active xmlns=\'http://jabber.org/protocol/chatstates\'/></message>');
	assertEqual('window.jabber.sentMessage(1)', '<message id="m_1" to="toymachine@hyves.nl/desktopClient_1234" toSessionId="11223344" type=\'chat\'><body>testing MUC message 2</body><active xmlns=\'http://jabber.org/protocol/chatstates\'/></message>');
}

/**
 * Tests how the case where one user leaves the session is handled.
 */
function testUserClosesSession() {
	
	window.jabber.incomingXml('<message id="m_1" from="optroodt@hyves.nl/desktopClient" to="arendtest@hyves.nl/desktopClient" type="chat"><sessionclosed xmlns="http://hyves.net/xmpp" toSessionId="' + window.xmppSession.sessionId + '" fromSessionId="44332211" /></message>');
	
	sleep(1);
	
	assert('window.xmppSession.participants[\'toymachine@hyves.nl\']');
	assert('window.xmppSession.participants[\'toymachine@hyves.nl\'].sessionId');
	assertUndefined('window.xmppSession.participants[\'optroodt@hyves.nl\']');
	
	window.jabber.incomingXml('<message id="m_1" from="toymachine@hyves.nl/desktopClient_1234" to="arendjr@hyves.nl/desktopClient" type="chat"><sessionclosed xmlns="http://hyves.net/xmpp" toSessionId="' + window.xmppSession.sessionId + '" fromSessionId="11223344" /></message>');
	
	sleep(1);
	
	assertUndefined('window.xmppSession.participants[\'toymachine@hyves.nl\'].sessionId');
}

/**
 * Tests whether the messages in a used-to-be MUC session is sent correctly.
 */
function testSendMessage3() {
	
	window.xmppSession.rte.value = 'testing MUC message 3';
	simulateClick('uiButtonSend');
	
	sleep(1);
	
	assertEqual('window.jabber.sentMessage(1)', '<message id="m_1" to="optroodt@hyves.nl/desktopClient" toSessionId="44332211" type=\'chat\'><body>testing MUC message 2</body><active xmlns=\'http://jabber.org/protocol/chatstates\'/></message>');
	assertEqual('window.jabber.sentMessage(0)', '<message id="m_1" to="toymachine@hyves.nl/desktopClient_1234" type=\'chat\'><body>testing MUC message 3</body><active xmlns=\'http://jabber.org/protocol/chatstates\'/></message>');
	
	window.tester.setTestWindowForNextMethod(globalMainWindowId);
}

/**
 * Tests the closing of a MUC session window and checks whether we sent messages
 * to the other participants that we left the session.
 */
function testCloseWindow2() {
	
	assertEqual('window.container.sessionWindows[\'toymachine@hyves.nl\']', globalChatWindowId);
	assertUndefined('window.container.remoteSessionIds[\'toymachine@hyves.nl/desktopClient_1234\'][\'11223344\']');
	assertUndefined('window.container.remoteSessionIds[\'optroodt@hyves.nl/desktopClient\'][\'44332211\']');
	
	var fromSessionId = window.container.windowToSessionMap[globalChatWindowId];
	
	window.windowManager.closeWindow(globalChatWindowId);
	
	sleep(1);
	
	assertEqual('window.jabber.sentMessage(0)', '<message id="m_1" to="toymachine@hyves.nl" type=\'chat\'><gone xmlns=\'http://jabber.org/protocol/chatstates\'/></message>');
	assert('window.jabber.sentMessage(1).substr(0, 39) != \'<message id="m_1" to="optroodt@hyves.nl\'');
	
	assertUndefined('window.container.sessionWindows[\'toymachine@hyves.nl\']');
	assertUndefined('window.container.windowToSessionMap[' + globalChatWindowId + ']');
	assertUndefined('window.container.sessionToWindowMap[\'' + fromSessionId + '\']');
	assertUndefined('window.container.remoteSessionIds[\'toymachine@hyves.nl/desktopClient_1234\'][\'11223344\']');
}

/**
 * Now test an invitation for a chat that already has 3 people.
 */
function testIncomingInvitationForFourWayChat() {
	
	window.jabber.incomingXml('<message id="m_1" from="optroodt@hyves.nl/desktopClient" to="arendtest@hyves.nl" type="chat"><sessioninvite xmlns="http://hyves.net/xmpp" fromSessionId="11111111"><participants><participant jid="emielvanliere@hyves.nl/desktopClient" sessionId="22222222"/><participant jid="salsel@hyves.nl/desktopClient" sessionId="33333333"/></participants></sessioninvite>' + mucMessageBody + '</message>');
	
	sleep(2);
	
	assertMoreThan('getLastOpenedSessionWindowId()', globalChatWindowId);
	globalChatWindowId = getLastOpenedSessionWindowId();
	
	window.tester.setTestWindowForNextMethod(globalChatWindowId);
}

/**
 * Tests whether the messages in a MUC session are sent to both contacts.
 */
function testParticipantsInFourWayChat() {
	
	assert('window.xmppSession.participants[\'optroodt@hyves.nl\']');
	assert('window.xmppSession.participants[\'emielvanliere@hyves.nl\']');
	assert('window.xmppSession.participants[\'salsel@hyves.nl\']');
	
	assertEqual('$(\'inviteLink\').style.display', 'inline');
	assertEqual('$(\'historyLink\').style.display', 'none');
	
	window.tester.setTestWindowForNextMethod(globalMainWindowId);
}

/**
 * Tests the closing of a MUC session window and checks whether we sent messages
 * to the other participants that we left the session.
 */
function testCloseWindow3() {
	
	assert('window.container.windowToSessionMap[globalChatWindowId]');
	window.windowManager.closeWindow(globalChatWindowId);
	
	sleep(1);
	
	assertUndefined('window.container.windowToSessionMap[globalChatWindowId]');
}

/**
 * Sets up another session we're going to use. In this session we'll check what
 * happens when two contacts are invited simultaneously.
 */
function testSetupMUCSessionForSimultaneousInvites() {
	
	simulateClick('dock_roster_item_optroodt');
	
	sleep(2);
	
	globalChatWindowId = window.container.sessionWindows['optroodt@hyves.nl'];
	
	assertMoreThan('globalChatWindowId', 1);
	
	window.tester.setTestWindowForNextMethod(globalChatWindowId);
}

/**
 * Opens the invite window to invite another user.
 */
function testOpenInvitationWindowForSimultaneousInvites() {
	
	simulateClick('inviteLink');
	
	sleep(2);
	
	assertMoreThan('window.xmppSession.inviteWindowId', 1);
	
	window.tester.setTestWindowForNextMethod(window.xmppSession.inviteWindowId);
}

/**
 * Tests the contents of the invite window.
 */
function testInviteToymachineForSimultaneousInvites() {
	
	simulateClick('dock_roster_item_toymachine');
	
	window.tester.setTestWindowForNextMethod(window.invitationManager.sessionWindowId);
}

/**
 * Tests whether the contact gets invited.
 */
function testBothContactsJoinedAndSessionJoinedForwarded() {
	
	window.jabber.incomingXml('<message id="m_1" from="optroodt@hyves.nl/desktopClient" to="' + window.roster.ownFullJid() + '" type="chat"><invitestart xmlns="http://hyves.net/xmpp" invitee="salsel@hyves.nl/desktopClient" toSessionId="' + window.xmppSession.sessionId + '" fromSessionId=\"87654321\" />' + mucMessageBody + '</message>');
	window.jabber.incomingXml('<message id="m_1" from="salsel@hyves.nl/desktopClient" to="' + window.roster.ownFullJid() + '" type="chat"><sessionjoined xmlns="http://hyves.net/xmpp" toSessionId="' + window.xmppSession.sessionId + '" fromSessionId=\"44444444\" />' + mucMessageBody + '</message>');
	
	sleep(2);
	
	assert('window.xmppSession.participants.hasOwnProperty(\'optroodt@hyves.nl\')');
	assert('window.xmppSession.participants.hasOwnProperty(\'toymachine@hyves.nl\')');
	assert('window.xmppSession.participants.hasOwnProperty(\'salsel@hyves.nl\')');
	
	var forwardedMessageStart = '<message id="m_1" to="toymachine@hyves.nl/desktopClient_1234" type="chat"><sessionjoined xmlns="http://hyves.net/xmpp" forwardFrom="salsel@hyves.nl/desktopClient" toSessionId="12345678" fromSessionId="44444444" />';
	assertEqual('window.jabber.sentMessage(0).substr(0, ' + forwardedMessageStart.length + ')', forwardedMessageStart);
	
	assertEqual('$(\'inviteLink\').style.display', 'inline');
	assertEqual('$(\'historyLink\').style.display', 'none');
	
	window.tester.setTestWindowForNextMethod(globalMainWindowId);
}

/**
 * Tests the closing of a MUC session window and checks whether we sent messages
 * to the other participants that we left the session.
 */
function testCloseMUCWindowForSimultaneousInvites() {
	
	var fromSessionId = window.container.windowToSessionMap[globalChatWindowId];
	
	window.windowManager.closeWindow(globalChatWindowId);
	
	sleep(1);
	
	assertEqual('window.jabber.sentMessage(0)', '<message id="m_1" to="salsel@hyves.nl/desktopClient" type="chat"><sessionclosed xmlns="http://hyves.net/xmpp" toSessionId="44444444" fromSessionId="' + fromSessionId + '" /></message>');
	assertEqual('window.jabber.sentMessage(1)', '<message id="m_1" to="toymachine@hyves.nl/desktopClient_1234" type="chat"><sessionclosed xmlns="http://hyves.net/xmpp" toSessionId="12345678" fromSessionId="' + fromSessionId + '" /></message>');
	assertEqual('window.jabber.sentMessage(2)', '<message id="m_1" to="optroodt@hyves.nl/desktopClient" type="chat"><sessionclosed xmlns="http://hyves.net/xmpp" toSessionId="87654321" fromSessionId="' + fromSessionId + '" /></message>');
	
	assertUndefined('window.container.windowToSessionMap[' + globalChatWindowId + ']');
	assertUndefined('window.container.sessionToWindowMap[\'' + fromSessionId + '\']');
}
