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

var rosterWindowId = 1;
var chatWindowId = 0;
var historyWindowId = 0;

/*
 * HistoryTest.js
 *
 * This test suite checks whether previous chat messages are saved in the
 * history, and are displayed correctly, both inline and in the history window.
 *
 * This test depends on the messages sent by NewSessionTest.js.
 */

/**
 * First opens a chat window to inspect.
 */
function testOpenChatWindow() {
	
	window.container.initiateSession('kilian@hyves.nl');
	
	sleep(2);
	
	chatWindowId = window.container.sessionWindows['kilian@hyves.nl'];
	
	assertMoreThan('chatWindowId', 1);
	
	window.tester.setTestWindowForNextMethod(chatWindowId);
}

/**
 * Tests whether the last chat messages are visible in the chat window.
 */
function testLastHistoryMessages() {
	
	assertEqual("getDisplayedMessage('chatsession', 'ownMessage', 1)", '<p>testing full JID</p>');
	
	assertEqual("getDisplayedMessage('chatsession', 'otherMessage', 0)", '<p>Ack</p>');
	
	assertEqual("getDisplayedMessage('chatsession', 'ownMessage', 2)", '<p>Buzzzzzzzzzzzzzz! </p>');
	
	simulateClick('historyLink');
	
	window.tester.setTestWindowForNextMethod(rosterWindowId);
}

/**
 * Tests whether the history window was opened by the click in the previous
 * test, and switch context to the history window.
 */
function testOpenHistoryWindow() {
	
	historyWindowId = window.container.historyWindowId;
	
	assertMoreThan('historyWindowId', 1);
	
	window.tester.setTestWindowForNextMethod(historyWindowId);
}

/**
 * Tests whether the last chat messages are visible in the chat window, and
 * tests deleting of the history.
 */
function testMessagesInHistoryWindow() {
	
	assertEqual("getDisplayedMessage('messageContainer', 'otherMessage', 0)", '<p>test "test" hoe is\'t ermee? /me ducks\\</p>');
	
	assertEqual("getDisplayedMessage('messageContainer', 'ownMessage', 2)", '<p>testing full JID</p>');
	
	assertEqual("getDisplayedMessage('messageContainer', 'otherMessage', 1)", '<p>Ack</p>');
	
	assertEqual("getDisplayedMessage('messageContainer', 'ownMessage', 3)", '<p>Buzzzzzzzzzzzzzz! </p>');
	
	simulateClick('deleteButton');
	
	assertEqual("getDisplayedMessage('messageContainer', 'otherMessage', 0)", 'null');
	assertEqual("getDisplayedMessage('messageContainer', 'ownMessage', 0)", 'null');
	
	window.tester.setTestWindowForNextMethod(rosterWindowId);
}

/**
 * Closes and reopens the history window, so after we can check whether it's
 * now empty (the history should still be deleted).
 */
function testReopenHistoryWindow() {
	
	window.windowManager.closeWindow(historyWindowId);
	
	sleep(3);
	
	assertEqual('window.container.historyWindowId', 0);
	
	window.container.openHistoryWindow('kilian@hyves.nl');
	
	sleep(1);
	
	historyWindowId = window.container.historyWindowId;
	
	assertMoreThan('historyWindowId', 1);
	
	window.tester.setTestWindowForNextMethod(historyWindowId);
}

/**
 * Now test whether the window is really still empty.
 */
function testHistoryWindowStillEmpty() {
	
	assertEqual("getDisplayedMessage('messageContainer', 'otherMessage', 0)", 'null');
	assertEqual("getDisplayedMessage('messageContainer', 'ownMessage', 0)", 'null');
	
	window.tester.setTestWindowForNextMethod(rosterWindowId);
}

/**
 * Tests whether the windows are closed correctly.
 */
function testCloseWindows() {
	
	window.windowManager.closeWindow(chatWindowId);
	window.windowManager.closeWindow(historyWindowId);
	
	sleep(3);
	
	assertUndefined('window.container.sessionWindows[\'kilian@hyves.nl\']');
	assertEqual('window.container.historyWindowId', 0);
}

/**
 * Returns a chat message displayed in the chat session.
 *
 * @param container Container in which to find the chat messages.
 * @param className CSS class name of the message, use 'ownMessage' or
 *                  'otherMessage'.
 * @param index Index of the message to return.
 */
function getDisplayedMessage(container, className, index) {
	
	var message = 'null';
	
	var messageContainer = $(container).down();
	while (messageContainer) {
		if (messageContainer.hasClassName(className)) {
			if (index == 0) {
				break;
			}
			index--;
		}
		messageContainer = messageContainer.next();
	}
	if (messageContainer) {
		message = messageContainer.down().next().innerHTML;
	}
	
	return message;
}
