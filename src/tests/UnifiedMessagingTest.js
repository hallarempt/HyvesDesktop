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

function testOpenNewChatSession() {
	
	window.container.ensureSession('kilian@hyves.nl', true);
	
	sleep(5);
	
	assertMoreThan('window.container.sessionWindows[\'kilian@hyves.nl\']', 1);
}

/**
 * Tests the creation of a new session and the sending of all different message
 * types.
 */
function testUnifiedMessaging() {
	
	assertMoreThan('window.container.sessionWindows[\'kilian@hyves.nl\']', 1);
	
	var windowId = window.container.sessionWindows['kilian@hyves.nl'];
	
	window.windowManager.windowExecute(windowId, 'loadJsFile', ':/src/tester/Tester.js');
	window.windowManager.windowExecute(windowId, 'loadJsFile', ':/src/tests/UnifiedMessagingTest.js');
	window.windowManager.windowExecute(windowId, 'executeJs', 'sendMessages()');
	
	window.windowManager.closeWindow(windowId);
}

function sendMessages() {
	
	sendIM();
	sendScrap();
	sendEmail();
	sendPrivateMessage();
	sendSms();
}

function sendIM() {
	
	window.xmppSession.rte.value = 'test1';
	
	sendMessageWithType('radioMessageTypeChat');
	
	assertEqual('latestDisplayedMessage()', 'test1');
}

function sendScrap() {
	
	window.xmppSession.rte.value = 'test2';
	
	sendMessageWithType('radioMessageTypeScrap');
	
	assertEqual("$('messageSendReply').style.color", 'green');
}

function sendEmail() {
	
	window.xmppSession.rte.value = 'test3';
	
	sendMessageWithType('radioMessageTypeEmail');
	
	assertEqual("$('messageSendReply').style.color", 'green');
}

function sendPrivateMessage() {
	
	window.xmppSession.rte.value = 'test4';
	
	sendMessageWithType('radioMessageTypeMessage');
	
	assertEqual("$('messageSendReply').style.color", 'green');
}

function sendSms() {
	
	window.xmppSession.rte.value = 'test5';
	
	sendMessageWithType('radioMessageTypeSms');
	
	assertEqual("$('messageSendReply').style.color", 'green');
}

function sendMessageWithType(radioButtonId) {
	
	$('messageSendReply').style.color = '';
	$(radioButtonId).checked = true;
	window.xmppSession.onMessageTypeChanged();
	sleep(3);
	$('uiButtonSend').disabled = false;
	$('uiButtonSend').click();
	sleep(3);
}

function latestDisplayedMessage() {
	
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
