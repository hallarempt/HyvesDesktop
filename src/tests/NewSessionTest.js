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


/*
 * NewSessionTest.js
 *
 * This test suite checks proper behavior for chat functionality, such as :
 * - creating new chat session
 * - sending text messages
 * - sending buzz
 * - inserting smileys
 * - closing window 
 * - ...
 */

var soundsPlayed = {};
var globalMainWindowId = 1;
var globalChatWindowId = 0;


/************************************************************************************************/
/* utility functions called in tests */
/************************************************************************************************/
 
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
 * Sound signal handler.
 */
function onSoundPlayed(soundResource) {
	
	if (soundsPlayed[soundResource]) {
		soundsPlayed[soundResource]++;
	} else {
		soundsPlayed[soundResource] = 1;
	}
}

/**
 * Checks that the chat send button is disabled for the user (greyed out).
 */
function isSendButtonDisabled() {
	
	return $('uiButtonSend').hasClassName('disabled');
}


/************************************************************************************************/
/**
 * Test functions
 */
/************************************************************************************************/

/************************************************************************************************/
/* Functional test : create new session 
 *
 * - tests a session window is created.
 * - tests a message is received.
 * - tests a sound is played.
 */
/************************************************************************************************/

/**
 * Tests the creation of a new session .
 */
function testSetupNewSession() {
	
	window.extender.connect('player', 'soundPlayed(QString)', 'onSoundPlayed');
	
	window.jabber.incomingXml('<message from=\'kilian@hyves.nl/siteClient\' to=\'arendjr@hyves.nl\' type=\'chat\' id=\'m_1\'>' +
	                            '<body>test "test"\nhoe is\'t ermee?\n/me ducks\\</body>' +
	                            '<active xmlns=\'http://jabber.org/protocol/chatstates\'/>' +
	                            '<request xmlns=\'urn:xmpp:receipts\'/>' +
	                          '</message>');
	
	sleep(5);
	
	// test whether we confirmed the arrival of the message
	assertEqual('window.jabber.sentMessage(0)', '<message id="m_1" to="kilian@hyves.nl/siteClient" type=\'chat\'><received xmlns=\'urn:xmpp:receipts\'/></message>');
	
	// test whether the session window has been created
	assertMoreThan('window.container.sessionWindows[\'kilian@hyves.nl\']', 1);
}

/**
 * Tests the new session triggers a sound to the user. 
 */
function testSoundReceived() {
	
	assertEqual('soundsPlayed[\':/plugins/kwekker/sounds/newSession\']', 1);	
	
	globalChatWindowId = window.container.sessionWindows['kilian@hyves.nl'];
	
	window.tester.setTestWindowForNextMethod(globalChatWindowId);
}

/**
 * Tests whether the window is positioned correctly.
 */
function testGeometry() {
	
	window.windowManager.windowExecute(window.xmppSession.windowId, 'bringToFront');
	
	sleep(1);
	
	g_windowInfo = window.windowManager.windowInfoJSON(window.xmppSession.windowId).parseJSON();
	
	window.windowManager.windowExecute(window.xmppSession.windowId, 'minimize');
	
	var height = 570 + window.windowManager.titleBarHeight() + window.windowManager.frameHeight();
	var width = 520 + 2 * window.windowManager.frameWidth();
	var top = Math.floor((g_windowInfo.screenHeight / 2) - (height / 2));
	var left = Math.floor((g_windowInfo.screenWidth / 2) - (width / 2));
	
	assertEqual('g_windowInfo.innerHeight', 570);
	assertEqual('g_windowInfo.innerWidth', 520);
	assertEqual('g_windowInfo.height', height);
	assertEqual('g_windowInfo.width', width);
	assertEqual('g_windowInfo.top', top);
	assertEqual('g_windowInfo.left', left);
}

/**
 * Kilian is using the web client, so the invite link should be disabled.
 */
function testInviteDisabled() {
	
	assertEqual('$(\'inviteLink\').style.display', 'none');
}

/**
 * Tests the contents of the message that is received. 
 */
function testMessageReceived() {
	
	assertEqual('getLatestDisplayedMessage()', '<p>test "test"<br>hoe is\'t ermee?<br>/me ducks\\</p>');
}

/**
 * Tests whether the send button is disabled.
 */
function testSendButtonIsDisabled() {
	
	assertEqual("isSendButtonDisabled()", true);
}


/************************************************************************************************/
/* Functional test : insert smiley 
 *
 * - tests a smiley is correctly selected and sent to the user.
 * - tests send button is enabled before sending/disabled after sending
 */
/************************************************************************************************/

function testSmiley() {
	
	var smilieButton = $("richtextcontrol_msg_tab_smilie_button");
	var link = smilieButton.childNodes[0];	
	simulateClick(link);	
	sleep(1);
}

function testSmiley2() {
	
	var category = $("smiliepicker_msg_category_cool");
	simulateClick(category);	
	sleep(1);
}

function testSmiley3() {
	
	var smiley = $("smiliepicker_msg_smilie_4_16");
	simulateClick(smiley);
	sleep(1);
}

function testSendButtonIsEnabled() {
	
	assertEqual("isSendButtonDisabled()", false);
}

function testSmiley4() {
	
	$('uiButtonSend').click();
	sleep(1);
}

/**
 * Returns if message passed as parameter contains the selected smiley (rasta smiley).
 */
function hasSmileyImage(msg) {
	
	var pattern = /smiley_rasta\.gif/;
	return (String(msg).search(pattern) != -1);
}

function testSmiley5() {
	
	assertEqual('hasSmileyImage(getLatestDisplayedMessage())',true );
	assertEqual("isSendButtonDisabled()", true);
}


/************************************************************************************************/
/* Functional test : insert picture 
 *
 * - tests a picture is correctly selected and sent to the user.
 * - tests send button is enabled before sending/disabled after sending
 */
/************************************************************************************************/

function testPicture() {
	
	var mediaButton = $("richtextcontrol_msg_tab_media_button");
	var mediaLink = mediaButton.childNodes[0];
	simulateClick(mediaLink);
	sleep(3);
}

function testPicture2() {
	
	var category = $("mediapicker_msg_albums_button");
	simulateClick(category);	
	sleep(3);
}

function testPicture3() {
	
	var category = $("mediapicker_msg_memberalbums_-1_loadalbum");
	simulateClick(category);
	sleep(5);
}

function testPicture4() {
	
	var pickPane = $("mediapicker_msg_pickplane");
	g_mediaAdd = pickPane.down().next().next().down();
	assert("g_mediaAdd.id.substr(0, 29) == 'mediapicker_msg_memberalbums_'");
	assert("g_mediaAdd.id.substr(-4) == '_add'");
	simulateClick(g_mediaAdd);
	sleep(3);
}

function testSendButtonIsEnabled() {
	
	assertEqual("isSendButtonDisabled()", false);
}

function testPicture5() {

	$('uiButtonSend').click();
	sleep(3);
}

/**
 * Returns if message passed as parameter contains a picture
 */
function hasPicture(msg) {
	
	var pattern = /media/;
	return (String(msg).search(pattern) != -1);
}

function testPictures6() {
	
	assertEqual('hasPicture(getLatestDisplayedMessage())', true);
	assertEqual("isSendButtonDisabled()", true);
}


/************************************************************************************************/
/* Functional test : send reply 
 *
 * - tests a reply is sent.
 * - tests a sound is played.
 */
/************************************************************************************************/

/**
 * Tests that a reply is sent to the correct full JID.
 */
function testReply() {
	
	function sendReply() {
	
		window.xmppSession.rte.value = 'testing full JID';
		$('uiButtonSend').disabled = false;
		$('uiButtonSend').click();
	}
	
	sendReply();
	
	sleep(5);
}

function testReplyReceived() {
	
	// this message is sent by JabberMock if addressed to the full JID
	assertEqual('getLatestDisplayedMessage()', '<p>Ack</p>');
	
	window.tester.setTestWindowForNextMethod(globalMainWindowId);
}

function testSoundAfterReply() {
	
	assertEqual('soundsPlayed[\':/plugins/kwekker/sounds/newSession\']', 2);
	
	window.tester.setTestWindowForNextMethod(globalChatWindowId);
}

/************************************************************************************************/
/* Functional test : send buzz.
 *
 * - tests a buzz is received.
 * - tests a sound is played.
 */
/************************************************************************************************/

/**
 * Tests sending a Buzz and receiving it.
 */
function testBuzz() {
	
	simulateClick('uiButtonBuzz');
	
	sleep(3);
	
	assertEqual('getLatestDisplayedMessage()', '<p>Buzzzzzzzzzzzzzz! </p>');
	
	window.tester.setTestWindowForNextMethod(globalMainWindowId);
}

/**
 * Tests the sound associated with the previous Buzz was correctly played.
 */
function testSoundAfterBuzz() {
	
	assertEqual('soundsPlayed[\':/plugins/kwekker/sounds/newSession\']', 2);
}


/************************************************************************************************/
/* Functional test : close window. */
/************************************************************************************************/

/**
 * Tests that the window is correctly unregistered when it is closed.
 */
function testCloseWindow() {
	
	var windowId = window.container.sessionWindows['kilian@hyves.nl'];
	
	window.windowManager.closeWindow(windowId);
	
	sleep(3);
	
	assertUndefined('window.container.sessionWindows[\'kilian@hyves.nl\']');
}


/************************************************************************************************/
/* Functional test : check no session created for state message. */
/************************************************************************************************/

/**
 * Tests that no new session window is created when a chat state message comes
 * in. 
 */
function testIncomingChatState() {
	
	window.jabber.incomingXml('<message from=\'kilian@hyves.nl/siteClient\' to=\'arendjr@hyves.nl\' type=\'chat\' id=\'m_101\'><paused xmlns=\'http://jabber.org/protocol/chatstates\'/></message>');
	
	sleep(5);
	
	assertUndefined('window.container.sessionWindows[\'kilian@hyves.nl\']');
	
	assertEqual('soundsPlayed[\':/plugins/kwekker/sounds/newSession\']', 2);
}
