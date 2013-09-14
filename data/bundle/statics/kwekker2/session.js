var xmppSession = null;

templates.addFromString('chat.inlineStatusMessage', '<li class="message"><hr /><label>${message}</label></li>');
templates.addFromString('chat.message', '<li id="${msgId}" class="${%ownMessage ? \'ownMessage\' : \'otherMessage\'%} ${extraClass}"><cite><img src="http://localhost/images/nixel.gif" alt="" onload="window.xmppSession.mediaLocator.setMedia(this, \'${mediaId}\', \'${mediaSecret}\')"></cite><blockquote><p>${msg|fancyLayout:"body!newtarget"|softHyphen:55}</p></blockquote></li>');
templates.addFromString('chat.participantInfo',
	'<div id="usr-optns">' +
		'<div class="avatar small">' +
			'<a href="http://${item.userhome}">' +
				'<img src="http://localhost/images/nixel.gif" alt="" onclick="" onload="window.mediaLocator.setMedia(this, \'${item.mediaId}\', \'${item.mediaSecret}\');" class="square-img">' +
			'</a>' +
		'</div>' +
		'<p style="padding: 9px 0px 0px">${item.nick|fancyLayout:"nickname,nosmilies"}</p>' +
		'<br />' +
		'<div class="usr-www" style="margin: -4px 0px 0px">' +
			'{if item.wwwEmotion}' + 
			'<div class="dropdown">' +
				'<div class="dropdown-content"><strong>www:</strong> ${item.wwwEmotion|default:""|fancyLayout:"oneliner"}{if item.wwwLocation}@${item.wwwLocation|default:""|fancyLayout:"oneliner"}{/if}</div>' +
			'</div>' +
			'<strong>www:</strong> ${item.wwwEmotion|default:""|fancyLayout:"oneliner"|softHyphen:20}{if item.wwwLocation}@${item.wwwLocation|default:""|fancyLayout:"oneliner"|softHyphen:20}{/if}' +
			'{/if}' +
		'</div>' +
	'</div>'
);
templates.addFromString('chat.multipleParticipantsInfo',
	'<div id="usr-optns">' +
		'{for participant in participants}' +
		'<div style="float: left; display: inline-block">' +
			'<div class="avatar extra-small">' +
				'<a href="http://${participant.userhome}">' +
					'<img src="http://localhost/images/nixel.gif" alt="" onclick="" onload="window.mediaLocator.setMedia(this, \'${participant.mediaId}\', \'${participant.mediaSecret}\');" class="square-img">' +
				'</a>' +
			'</div>' +
			'<p style="padding: 0px 3px">${participant.nick|fancyLayout:"nickname,nosmilies"|truncate:10}</p>' +
		'</div>' +
		'{/for}' +
	'</div>'
);

var dateFormat = new DateFormat();
var numberFormatMoney = '0.00';
var receiptConfirmationTimeout = 10000; // 10 seconds
var expireSessionTimeout = 1800000; // 30 minutes

var ChatState = {
	NONE: 'none',
	INACTIVE: 'inactive',
	ACTIVE: 'active',
	PAUSED: 'paused',
	COMPOSING: 'composing',
	GONE: 'gone'
}

/**
 * The XMPPChatStateMachine manages the chat state of a session
 * e.g. it generates the events 'composing', 'active', 'gone' etc.
 */
function XMPPChatStateMachine(sendMessage) {
	
	this.enabled = false;
	this.pausedTimeout = 0;
	this.inactiveTimeout = 0;
	this.goneTimeout = 0;
	this.lastMessageTimeout = 0;
	this.sendMessage = sendMessage;
	this.state = ChatState.NONE;
}

XMPPChatStateMachine.prototype.log = logging.getLogger('XMPPChatStateMachine');

XMPPChatStateMachine.prototype.onComposeInput = function(message) {
	
	this.setState(ChatState.COMPOSING, message);
}

XMPPChatStateMachine.prototype.onSendMessage = function(message) {
	
	this.setState(ChatState.ACTIVE, message);
}

XMPPChatStateMachine.prototype.onGone = function(message) {
	
	this.setState(ChatState.GONE, message);
}

XMPPChatStateMachine.prototype.setState = function(state, message) {
	
	try {
		if (!this.enabled && state != ChatState.ACTIVE) {
			return; // we only start sending chatstates after the first message is sent or received
		}
		
		this.enabled = true;
		
		if (this.state != state || message) {
			var extra = '<' + state + ' xmlns=\'http://jabber.org/protocol/chatstates\'/>'
			this.sendMessage(message, extra);
		}
		
		clearTimeout(this.pausedTimeout);
		clearTimeout(this.inactiveTimeout);
		clearTimeout(this.goneTimeout);
		
		var self = this;
		if (state == ChatState.COMPOSING) {
			this.pausedTimeout = setTimeout(function() {
				if (self.state == ChatState.COMPOSING) {
					self.setState(ChatState.PAUSED);
				}
			}, 5000);
		}
		if (state == ChatState.ACTIVE || state == ChatState.COMPOSING || state == ChatState.PAUSED) {
			this.inactiveTimeout = setTimeout(function() {
				if (self.state == ChatState.ACTIVE || self.state == ChatState.COMPOSING || self.state == ChatState.PAUSED) {
					self.setState(ChatState.INACTIVE);
				}
			}, 25000);
		}
		if (state != ChatState.GONE) {
			this.goneTimeout = setTimeout(function() {
				if (self.state != ChatState.GONE) {
					self.setState(ChatState.GONE);
				}
			}, 60000);
		}
		
		this.state = state;
	} catch(e) {
		this.log.error('session.js:setState(): ' + e, e);
	}
}

/**
 * Represents the model for a chat session.
 */
function XMPPSession() {
	
	this.extender = window.extender;
	this.windowManager = window.windowManager;
	
	this.mediaLocator = new MediaLocator();
	
	this.messageId = 0;
	this.windowId = 0;
	
	this.inviteWindowId = 0;
	this.inviteWindowInitialized = false;
	
	// every interval the amount of chars in the rte is counted
	this.smsRecalculateBodyLengthIntervalId = null;
	
	// will be retrieved the first time the sms messagetype is selected
	this.smsAccountBalance = null;
	this.smsPrice = null;
	this.smsAccountBalanceRequested = false;
	
	this.extender.connect('roster', 'itemChanged(QVariantMap, QVariantMap)', 'window.xmppSession.onItemChanged');
	this.extender.connect('windowManager', 'eventRaised(QVariantMap)', 'window.xmppSession.onEventRaised');
	
	// the handling of this signal should only start when the XMPPSession object is fully instantiated
	setTimeout(function() {
		window.windowManager.raiseEvent(ObjectToJSONString({
			name: 'sessionWindowAvailable'
		}));
	}, 10);
}

XMPPSession.prototype.log = logging.getLogger('XMPPSession');

XMPPSession.prototype.onEventRaised = function(event) {
	
	if (this.participants) {
		if (event.name == 'inviteWindowAvailable') {
			this.onInviteWindowAvailable(event);
		} else if (event.name == 'inviteAckReceived' && event.toSessionId == this.sessionId) {
			this.onInviteAckReceived(event);
		} else if (event.name == 'inviteStartReceived' && event.toSessionId == this.sessionId) {
			this.onInviteStartReceived(event);
		} else if (event.name == 'inviteContact' && event.sessionWindowId == this.windowId) {
			this.onInviteContact(event);
		} else if (event.name == 'sessionJoined' && event.toSessionId == this.sessionId) {
			this.onSessionJoined(event);
		} else if (event.name == 'sessionClosed' && event.toSessionId == this.sessionId) {
			this.onSessionClosed(event);
		} else if (event.name == 'windowClose' && event['class'] == 'invite') {
			this.onInviteWindowClosed(event.id);
		} else if (event.name == 'setMessageType' && event.jid == this.singleParticipantJid()) {
			this.setMessageType(event.type);
		}
		
		// filter out all messages not meant for this session
		if (!event.jid || !this.isParticipant(event.jid)) {
			return;
		}
		if (event.toSessionId) {
			if (event.toSessionId != this.sessionId) {
				return;
			}
		} else {
			if (this.singleParticipantJid() == null) {
				return;
			}
		}
		
		if (event.name == 'chatStateReceived' && event.state) {
			this.onChatStateReceived(event.jid, event.state);
		} else if (event.name == 'messageReceived' && event.message) {
			this.onMessageReceived(event.jid, event.message);
		} else if (event.name == 'receiptReceived') {
			this.onReceiptReceived(event.jid);
		}
	} else {
		if (event.name == 'sessionRegistered' && this.windowId == 0) {
			this.init(event);
		}
	}
}

/**
 * Returns whether the given jid is a participant in this session.
 */
XMPPSession.prototype.isParticipant = function(jid) {
	
	var bareJid = jidbare(jid);
	for (var jid in this.participants) {
		if (bareJid == jid) {
			return true;
		}
	}
	return false;
}

/**
 * Initializes the session window for communication.
 *
 * @param event Event containing jid, windowId, and other stuff.
 */
XMPPSession.prototype.init = function(event) {
	this.log.debug('init()');
	
	this.participants = {};
	this.composingParticipants = [];
	for (var i = 0; i < event.participants.length; i++) {
		var item = event.participants[i];
		var bareJid = jidbare(item.jid);
		
		this.log.debug('Adding JID to session: ' + this.log.anonymizeEMail(item.jid));
		
		this.participants[bareJid] = item;
		
		if (item.sessionId) {
			window.windowManager.raiseEvent(ObjectToJSONString({
				name: 'sendSessionJoined',
				toJid: item.jid,
				toSessionId: item.sessionId,
				fromSessionId: event.sessionId
			}));
		}
	}
	
	this.ownMemberInfo = window.roster.ownItemJSON().parseJSON();
	
	this.windowId = event.windowId;
	this.sessionId = event.sessionId;
	
	this.openInvitations = {};
	
	this.initGlobalVars(event.globalVars);
	
	this.initLanguage();
	
	this.initListeners();
	
	this.requestReceiptConfirmation = this.shouldRequestReceiptConfirmation();
	this.receiptConfirmationReceived = false;
	this.messageReceived = false;
	this.sessionTimeout = null;
	
	var self = this;
	this.xmppChatStateMachine = new XMPPChatStateMachine(function(message, extra) {
		if (self.requestReceiptConfirmation && !self.messageReceived) {
			extra += '<request xmlns=\'urn:xmpp:receipts\'/>';
			setTimeout(function() {
				self.onReceiptConfirmationTimeout();
			}, receiptConfirmationTimeout);
			
			self.requestReceiptConfirmation = false;
		}
		
		for (var jid in self.participants) {
			window.windowManager.raiseEvent(ObjectToJSONString({
				name: 'sendMessage',
				toJid: self.participants[jid].jid, // make sure we select the full JID
				toSessionId: self.participants[jid].sessionId,
				message: message,
				extra: extra
			}));
		}
	});
	
	this.renderParticipantsInfo();
	
	// will initialize correct handlers for unified messaging
	this.setMessageType('chat');
	
	this.initStatusMessage();
	this.initRTE();
	
	this.loadHistory();
	
	setInterval(function() { self.checkBuzzerIsEnabled(); }, 1000);
	
	this.log.usage('SessionWindowOpened(' + this.windowId + ')');
}

/**
 * Returns whether we should request a receipt confirmation for the first
 * message from the receiving party.
 *
 * We only request a receipt confirmation when there's a single participant,
 * which has a single resource which indicates the client is using Hyves
 * Desktop.
 */
XMPPSession.prototype.shouldRequestReceiptConfirmation = function() {
	
	var keys = $H(this.participants).keys();
	if (keys.length == 0 || keys.length > 1) {
		return false;
	}
	
	var jid = keys[0];
	var participant = this.participants[jid];
	if (participant.jid.length > jid.length) {
		return (participant.jid.substr(jid.length, 14) == '/desktopClient');
	}
	
	var resources = window.roster.itemResourcesJSON(jid).parseJSON().resources;
	if (resources.length == 0 || resources.length > 1) {
		return false;
	}
	
	if (resources[0].id.substr(0, 13) == 'desktopClient') {
		this.participants[jid].jid = jid + '/' + resources[0].id;
		return true;
	}
	
	return false;
}

XMPPSession.prototype.initGlobalVars = function(globalVars) {
	
	try {
		for (i in globalVars) {
			window[i] = globalVars[i];
		}
	} catch(e) {
		this.log.error('initGlobalVars(): ' + e, e);
	}
}

XMPPSession.prototype.initLanguage = function() {
	
	try {
		$('chatTabLabel').innerHTML = templates.translate('GENERAL_CHAT');
		$('scrapTabLabel').innerHTML = templates.translate('GENERAL_SCRAP');
		$('emailTabLabel').innerHTML = templates.translate('GENERAL_LABEL_EMAIL');
		$('privateMessageTabLabel').innerHTML = templates.translate('GENERAL_MESSAGE_PRIVATE');
		$('smsTabLabel').innerHTML = templates.translate('SMS_SUBJECT');
		
		$('uiButtonBuzz').firstChild.innerHTML = templates.translate('CHAT_BUZZ_BUTTON');
		$('uiButtonSend').firstChild.innerHTML = templates.translate('GENERAL_SEND');
		
		$('subjectrow').firstChild.nodeValue = templates.translate('UT_FB_HM_SUBJECT') + ' ';
		
		$('historyLink').firstChild.innerHTML = tr('History');
		$('inviteLink').firstChild.innerHTML = tr('Invite');
	} catch(e) {
		this.log.error('initLanguage(): ' + e, e);
	}
}

XMPPSession.prototype.canInviteContacts = function() {
	
	if (this.singleParticipantJid() == null) {
		return true;
	}
	
	for (var jid in this.participants) {
		if ((this.participants[jid].availableClients & ClientResourceBits.desktopClient) == 0) {
			return false;
		}
	}
	
	return true;
}

XMPPSession.prototype.initListeners = function() {
	
	var self = this;
	Event.observe('historyLink', 'click', function(event) { self.openHistoryWindow(); });
	Event.observe('inviteLink', 'click', function(event) { self.inviteContact(); });
}

XMPPSession.prototype.initStatusMessage = function() {
	
	var jid = this.singleParticipantJid();
	if (jid) {
		var nick = fancy_layout(this.participants[jid].nick, 'nickname,nosmilies');	
		if (!this.participants[jid].available) {
			this.setStatusMessage(tr("%1 is offline and will probably not respond.").arg(nick));
			return;
		} else if (this.participants[jid].availableClients == ClientResourceBits.siteClient) {
			this.setStatusMessage(tr("%1 is online at the website and may not respond.").arg(nick));
			return;
		}
	}
	
	if (this.composingParticipants.length == 0) {
		this.setStatusMessage('');
	} else if (this.composingParticipants.length == 1) {
		var nick = fancy_layout(this.participants[this.composingParticipants[0]].nick, 'nickname,nosmilies');
		this.setStatusMessage(tr("%1 is typing a message...").arg(nick));
	} else {
		var nicks = '';
		for (var i = 0; i < this.composingParticipants.length; i++) {
			if (i > 0) {
				if (i == this.composingParticipants.length - 1) {
					nicks += tr(" and ");
				} else {
					nicks += ', ';
				}
			}
			nicks += fancy_layout(this.participants[this.composingParticipants[i]].nick, 'nickname,nosmilies');
		}
		
		this.setStatusMessage(tr("%1 are typing a message...").arg(nicks));
	}
}

XMPPSession.prototype.setStatusMessage = function(message, color) {
	
	var element = $('chatsession_statusmessage');
	element.style.color = (color ? color : '');
	element.innerHTML = message;
}

/**
 * Initializes the Rich Text editor.
 */
XMPPSession.prototype.initRTE = function() {
	
	var self = this;
	requires(['module:RichTextModule'], function() {
		var rteInstance = new RichTextEditor({
			name: 'msg',
			open_picker: 'none',
			showpreview: false,
			textarea_height: '65px',
			allowgadgets: true,
			allowlinks: false
		});
		rteInstance.write('uiRTEContainer');
		rteInstance.init();
		
		self.rte = rteInstance.domCode;
		$('uiButtonSend').addClassName('disabled');
		
		Event.observe(self.rte, 'keypress', function(event) {
			if (isControlCharacter(event.keyCode) &&
			    !isEnterKey(event.keyCode) &&
			    !isBackspaceKey(event.keyCode)) {
				Event.stop(event);
				return;
			}
			
			if (self.messageType == 'chat') {
				if (isEnterKey(event.keyCode) && event.shiftKey == false) {
					Event.stop(event);
					self.sendMessageByType();
					return;
				}
				
				self.onRteKeyDown();
			}
		});
		Event.observe(self.rte, 'keydown', function(event) {
			setTimeout(function() {
				var enabled = (self.rte.value != '');
				if (enabled) {
					$('uiButtonSend').removeClassName('disabled');
				} else {
					$('uiButtonSend').addClassName('disabled');
				}
			}, 10);
		});
		Event.observe($('uiButtonSend'), 'click', function(event) {
			self.sendMessageByType();
		});
		
		// set initial focus
		setTimeout(function() {
			if (self.rte && self.rte.focus) {
				self.rte.focus();
			}
		}, 300);
	});
}

/**
 * Returns the bare JID of the only participant in the session. If there are
 * multiple participants, returns null.
 */
XMPPSession.prototype.singleParticipantJid = function() {
	
	var numParticipants = 0;
	for (var jid in this.participants) {
		numParticipants++;
	}
	
	return (numParticipants == 1 ? jid : null);
}

/**
 * Checks whether the buzzer is enabled (i.e. there is a participant to which we
 * may send buzzes), and enables or disables the buzzer button accordingly.
 */
XMPPSession.prototype.checkBuzzerIsEnabled = function() {
	
	if (!window.buzzManager) {
		return;
	}
	
	var enabled = false;
	for (var jid in this.participants) {
		if (window.buzzManager.maySendBuzzTo(jid)) {
			enabled = true;
			break;
		}
	}
	
	if (enabled) {
		$("uiButtonBuzz").removeClassName('disabled');
	} else {
		$("uiButtonBuzz").addClassName('disabled');
	}
}

/**
 * Loads the last 5 messages with the contact from the history and displays them
 * on top of the chat window.
 */
XMPPSession.prototype.loadHistory = function() {
	
	var jid = this.singleParticipantJid();
	if (!jid) {
		this.log.debug('Can not load history for multiple participants.');
		return;
	}
	
	var messages = window.historyKeeper.lastMessagesJSON(jid).parseJSON().messages;
	
	var time = false;
	var options = { skipTimeMessage: true, doNotLog: true, extraClass: 'history' };
	
	for (var i = 0; i < messages.length; i++) {
		var historyMessage = messages[i];
		if (!historyMessage) {
			break;
		}
		
		var message = historyMessage['message'];
		time = historyMessage['date'] + ' ' + historyMessage['time'];
		if (historyMessage['direction'] == 'from') {
			this.appendToSession(jid, this.participants[jid].nick, message, options);
		} else {
			this.appendToSession(this.ownMemberInfo.jid, this.ownMemberInfo.nick, message, options);
		}
	}
	
	if (time) {
		this.appendStatusMessage(tr("Sent on %1").arg(dateFormat.formatDateTimeLong(parseHistoryDateString(time))));
	}
}

XMPPSession.prototype.onChatStateReceived = function(fromJid, status) {
	
	var bareJid = jidbare(fromJid);
	if (status == 'composing') {
		if (this.composingParticipants.indexOf(bareJid) == -1) {
			this.composingParticipants.push(bareJid);
		}
	} else {
		var index = this.composingParticipants.indexOf(bareJid);
		if (index != -1) {
			this.composingParticipants.splice(index, 1);
		}
	}
	
	this.initStatusMessage();
}

/**
 * Handles incoming messages, it is appended to the session window and focus is
 * managed.
 *
 * @param fromJid Full JID of the sender of the message.
 * @param message The message (string) received. 
 */
XMPPSession.prototype.onMessageReceived = function(fromJid, message) {
	
	this.log.usage('MessageReceived(' + this.windowId + ')');
	
	bareJid = jidbare(fromJid);
	this.appendToSession(bareJid, this.participants[bareJid].nick, message);
	
	this.participants[bareJid].jid = fromJid; // replies should go to the full JID of the sender
	
	this.xmppChatStateMachine.enabled = true;
	this.clearNoReceiptMessage();
	this.messageReceived = true;
	
	clearTimeout(this.sessionTimeout);
	var self = this;
	this.sessionTimeout = setTimeout(function() {
		self.expireSession();
	}, expireSessionTimeout);
	
	if (message.indexOf('__buzz__') != -1) {
		requires(['module:BuzzManager'], function() {
			if (buzzManager.mayReceiveBuzzFrom(bareJid)) {
				buzzManager.receiveBuzz(bareJid);
				self.shakeWindow();
				window.player.playSound(':/plugins/kwekker/sounds/buzz');
			}
		});
	}
	
	this.flashTaskbarIfInactive();
}

/**
 * No messages have come in for a long time, so the session is going to be
 * expired. This means that any full JIDs we were addressing are now
 * reduced to bare JIDs again.
 */
XMPPSession.prototype.expireSession = function() {
	
	for (var bareJid in this.participants) {
		this.participants[bareJid].jid = bareJid;
	}
}

/**
 * Handles a received message receipt confirmation.
 *
 * @param fromJid Full JID of the sender of the receipt confirmation.
 */
XMPPSession.prototype.onReceiptReceived = function(fromJid) {
	
	this.receiptConfirmationReceived = true;
	
	this.clearNoReceiptMessage();
}

/**
 * Called when the receipt confirmation should have been received.
 */
XMPPSession.prototype.onReceiptConfirmationTimeout = function() {
	
	if (this.receiptConfirmationReceived || this.messageReceived) {
		this.submitMessageReceiptStatus(true);
		return;
	}
	
	this.submitMessageReceiptStatus(false);
	
	var jid = this.singleParticipantJid();
	if (!jid) {
		return;
	}
	
	// enable later when enough users have upgraded and statistics are verified
	//var nick = this.participants[jid].nick;
	//this.setStatusMessage(tr("Could not connect to %1. %1 may not receive your messages.").arg(fancy_layout(nick, 'nickname,nosmilies')));
}

XMPPSession.prototype.submitMessageReceiptStatus = function(success) {
	
	var jidParameter = '';
	if (!success) {
		jidParameter = '&jid=' + escape(this.singleParticipantJid());
	}
	new Ajax.Request('/?module=Chat&action=submitMessageReceiptStatus&success=' + (success ? 1 : 0) + jidParameter, {
		method: 'get'
	});
}

XMPPSession.prototype.clearNoReceiptMessage = function() {
	
	if ($('chatsession_statusmessage').innerHTML = tr("Could not connect to %1. %1 may not receive your messages.")) {
		this.setStatusMessage('');
	}
}

/**
 * Adds a message to the chat session window and scrolls the window so that it
 * is visible at the bottom.
 */
XMPPSession.prototype.appendToSession = function(jid, nick, message, options) {
	
	if (!options) {
		options = {};
	}
	
	this.messageId++;
	
	var messageId = 'msg_' + this.messageId;
	var ownMessage = (jid == this.ownMemberInfo.jid);
	templates.insertBottom('chatsession', 'chat.message', {
		msgId: messageId,
		msg: filterMessageForDisplay(message),
		mediaId: (ownMessage ? this.ownMemberInfo.mediaId : this.participants[jid].mediaId),
		mediaSecret: (ownMessage ? this.ownMemberInfo.mediaSecret : this.participants[jid].mediaSecret),
		ownMessage: ownMessage,
		extraClass: (options.extraClass ? options.extraClass : '')
	});
	
	this.scrollToBottomOfSession();
	
	if (!options.doNotLog) {
		var logJid = this.singleParticipantJid();
		if (logJid) {
			if (ownMessage) {
				window.historyKeeper.logMessage('>', logJid, message);
			} else {
				window.historyKeeper.logMessage('<', logJid, message);
			}
		}
	}
	
	if (!options.skipTimeMessage) {
		clearTimeout(this.lastMessageTimeout);
		var time = new Date();
		var self = this;
		this.lastMessageTimeout = setTimeout(function() {
			self.appendStatusMessage(tr("Sent on %1").arg(dateFormat.formatDateTimeLong(time)));
		}, 180000);
	}
}

/**
 * Appends an inline status message to the chat session.
 *
 * @param message The status message to append.
 */
XMPPSession.prototype.appendStatusMessage = function(message) {
	
	templates.insertBottom('chatsession', 'chat.inlineStatusMessage', {
		message: message
	});
	
	this.scrollToBottomOfSession();
}

/**
 * Scrolls the session view to the bottom, so it displays the last message.
 */
XMPPSession.prototype.scrollToBottomOfSession = function() {
	
	var scrollFunction = function() {
		var element = $('chatsession_container');
		element.scrollTop = element.scrollHeight;
	};
	
	// timeout used to give the DOM a chance to update
	setTimeout(scrollFunction, 30);
	setTimeout(scrollFunction, 100);
}

XMPPSession.prototype.shakeWindow = function() {
	this.log.debug('shakeWindow()');
	
	try {
		var windowInfo = this.windowManager.windowInfoJSON(this.windowId).parseJSON();
		if (windowInfo.windowState == 'minimized') {
			return;
		}
		
		this.windowManager.windowExecute(this.windowId, 'bringToFront');
		
		var self = this;
		var doShake = function(top, left, i, j) {
			switch (j) {
				case 0:
					left += i;
					break;
				case 1:
					top += i;
					break;
				case 2:
					left -= i;
					break;
				case 3:
					top -= i;
					i--;
			}
			j = (j + 1) % 4;
			this.windowManager.windowExecute(self.windowId, 'update', ObjectToJSONString({ left: left, top: top }));
			if (i > 0) {
				setTimeout(function() {
					doShake(top, left, i, j);
				}, 7);
			}
		}
		
		doShake(windowInfo.top, windowInfo.left, 15, 0);
	} catch(e) {
		this.log.error('session.js:shakeWindow(): ' + e, e);
	}
}

XMPPSession.prototype.flashTaskbarIfInactive = function() {
	
	var flash = false;
	
	if (this.alreadyFlashed) {
		// if we have already flashed, we only flash if the window is inactive
		var windowInfo = this.windowManager.windowInfoJSON(this.windowId).parseJSON();
		if (!windowInfo) {
			this.log.error('session.js:flashTaskbarIfInactive(): Could not get window info!');
			return;
		}
		if (!windowInfo.active) {
			flash = true;
			this.log.debug('session window not active, flashing!');
		}
	} else {
		// we always flash for the first message that is received
		flash = true;
		this.log.debug('first message received, flashing!');
		this.alreadyFlashed = true;
	}
	
	if (flash) {
		this.windowManager.windowExecute(this.windowId, 'flash');
		
		if (window.settingsManager.boolValue('Notifications/newSessionSound', true)) {
			this.log.debug('sound enabled for when flashing, now playing');
			window.player.playSound(':/plugins/kwekker/sounds/newSession');
		}
	}
}

XMPPSession.prototype.onItemChanged = function(item, changedProperties) {
	
	if (!this.participants.hasOwnProperty(item.jid)) {
		return;
	}
	
	// make sure these session-specific properties don't get lost
	item.jid = this.participants[item.jid].jid;
	item.sessionId = this.participants[jidbare(item.jid)].sessionId;
	
	this.participants[jidbare(item.jid)] = item;
	
	this.renderParticipantsInfo();
	this.initStatusMessage();
}

XMPPSession.prototype.sendMessageByType = function() {
	this.log.debug('sendMessageByType()');
	
	this.log.usage('MessageSent(' + this.windowId + ', ' + this.messageType + ')');
	
	if (this.messageType == 'chat') {
		this.sendMessage();
	} else if (this.messageType == 'scrap') {
		this.sendScrap();
	} else if (this.messageType == 'email') {
		this.sendMessageNoChat(messageTypeEmail);
	} else if (this.messageType == 'privateMessage') {
		this.sendMessageNoChat(messageTypeMemberToMember);
	} else if (this.messageType == 'sms') {
		this.sendMessageNoChat(messageTypeSms);
	}
}

XMPPSession.prototype.setMessageType = function(messageType) {
	
	try {
		if (this.messageType) {
			$(this.messageType + 'Tab').removeClassName('current');
		}
		
		this.messageType = messageType;
		
		$(this.messageType + 'Tab').addClassName('current');
		
		this.onMessageTypeChanged();
	} catch(e) {
		this.log.error('session.js:setMessageType(): ' + e, e);
	}
}

/**
 * When the type of msg changes (e.g. moving from chat to sms to krabbel etc,
 * the function of the rte changes and we need to do something different to 
 * send the message.
 * This function reattaches the correct events to the current xmppSession depending
 * on the message type
 */
XMPPSession.prototype.onMessageTypeChanged = function() {
	
	if (this.messageType == 'chat') {
		this.initStatusMessage();
	} else {
		this.setStatusMessage('');
	}
	
	$('sms_subject').style.display = (this.messageType == 'sms' ? 'block' : 'none');
	$('subjectrow').style.display = (this.messageType == 'email' || this.messageType == 'privateMessage' ? 'block' : 'none');
	$('uiButtonBuzz').style.display = (this.messageType == 'chat' ? 'block' : 'none');
	
	if (this.messageType == 'scrap') {
		this.retrieveScrapSecrets();
	}
	
	if (this.smsRecalculateBodyLengthIntervalId) {
		clearInterval(this.smsRecalculateBodyLengthIntervalId);
	}
	if (this.messageType == 'sms') {
		var self = this;
		this.smsRecalculateBodyLengthIntervalId = setInterval(function() {
			self.recalculateSMSBodyLength();
		}, 250);
		this.smsAccountBalanceRequested = false;
	}
	
	this.updateSessionHeight();
	this.scrollToBottomOfSession();
}

/**
 * Makes sure we have or retrieve the scrap secrets for all participants.
 */
XMPPSession.prototype.retrieveScrapSecrets = function() {
	
	for (var jid in this.participants) {
		if (!this.participants[jid].scrapSecret) {
			this.windowManager.raiseEvent(ObjectToJSONString({
				name: 'retrieveMemberInfo',
				jid: jid,
				includeSecrets: true
			}));
		}
	}
}

XMPPSession.prototype.updateSessionHeight = function() {
	
	var self = this;
	setTimeout(function() {
		var windowInfo = window.windowManager.windowInfoJSON(self.windowId).parseJSON();
		var otherHeight = $('participantsInfo').offsetHeight + $('menu').offsetHeight + $('windowBottom').offsetHeight;
		$('chatsession_container').style.height = (windowInfo.innerHeight - otherHeight) + 'px';
	}, 25);
}

/**
 * Sends the current content of the RTE as a chat message to the other party.
 * The message is also appended to the session window.
 */
XMPPSession.prototype.sendMessage = function(message) {
	this.log.debug('sendMessage()');
	
	try {
		if (typeof(message) == 'undefined') {
			message = this.rte.value;
			this.rte.value = '';
			$('uiButtonSend').addClassName('disabled');
		}
		
		if (message == '') {
			return; // don't allow to send empty messages
		}
		
		this.appendToSession(this.ownMemberInfo.jid, this.ownMemberInfo.nick, message);
		
		this.xmppChatStateMachine.onSendMessage(message);
		
		this.rte.focus();
	} catch(e) {
		this.log.error('session.js:sendMessage(): ' + e, e);
	}
}

XMPPSession.prototype.sendBuzz = function() {
	this.log.debug('sendBuzz()');
	
	if ($('uiButtonBuzz').hasClassName('disabled')) {
		return;
	}
	
	this.log.usage('BuzzSent(' + this.windowId + ')');
	
	var self = this;
	requires(['module:BuzzManager'], function() {
		var message = templates.translate('CHAT_BUZZ_MESSAGE') + ' __buzz__';
		
		self.appendToSession(self.ownMemberInfo.jid, self.ownMemberInfo.nick, message);
		
		for (var jid in self.participants) {
			self.sendBuzzToJid(self.participants[jid].jid, message);
		}
		
		setTimeout(function() {
			self.shakeWindow();
		}, 100);
		
		self.rte.focus();
	});
}

XMPPSession.prototype.sendBuzzToJid = function(jid, message) {
	this.log.debug('sendBuzzToJid(' + this.log.anonymizeEMail(jid) + ', ...)');
	
	try {
		if (buzzManager.maySendBuzzTo(jid)) {
			var enabled = buzzManager.sendBuzz(jid);
			if (enabled) {
				$("uiButtonBuzz").removeClassName('disabled');
			} else {
				$("uiButtonBuzz").addClassName('disabled');
			}
			
			var self = this;
			setTimeout(function() {
				self.xmppChatStateMachine.onSendMessage(message);
			}, 50);
		}
	} catch(e) {
		this.log.error('session.js:sendBuzzToJid(): ' + e, e);
	}
}

XMPPSession.prototype.sendMessageNoChat = function(messageType) {
	
	for (var jid in this.participants) {
		this.sendMessageNoChatToJid(jid, messageType);
	}
}

XMPPSession.prototype.sendMessageNoChatToJid = function(jid, messageType) {
	
	try {
		this.setStatusMessage('<img src="http://localhost/images/ajax_action.gif" alt="" />');
		
		var messageTo = this.participants[jid].username;
		var pars = messageSendPostman + '&returnType=json&sendmessage_type=' + messageType + '&sendmessage_to='
		                              + messageTo + '&sendmessage_subject=' + escape($('messageSubject').value)
		                              + '&rt_sendmessage_body=' + escape(this.rte.value) + '&sms_sendmessage_body=' + escape(this.rte.value)
		                              + '&sendmessage_body' + escape(this.rte.value);
		
		var self = this;
		new Ajax.Request('/', {
			method: 'post', 
			parameters: pars,
			onComplete: function(request) {
				self.afterMessageSent(request);
			},
			onException: function(request, exception) {
				logging.error('session.js:sendMessageNoChat(): ' + exception, exception);
			}
		});
	} catch(e) {
		this.log.error('session.js:sendMessageNoChatToJid(): ' + e, e);
	}
}

XMPPSession.prototype.afterMessageSent = function(originalRequest) {
	
	var messagesendResponse = originalRequest.responseText.parseJSON();
	if (!messagesendResponse) {
		logging.error('session.js:afterMessageSent(): Could not parse reply: ' + originalRequest.responseText);
		this.setStatusMessage(templates.translate('GENERAL_ERROR_UNEXPECTED'), 'red');
		return;
	}
	
	messagesendResponse = messagesendResponse[0];
	if (messagesendResponse['nr_successes'] == 1) {
		this.setStatusMessage(messagesendResponse['errorMessage'], 'green');
	} else if (messagesendResponse['nr_failures'] == 1) {
		this.setStatusMessage(messagesendResponse['errorMessage'], 'red');
	} else {
		this.setStatusMessage(messagesendResponse, 'red');
	}
	
	this.smsAccountBalanceRequested = false; // will (re)request the account balance,
	                                         // it could be lower after sending sms
}

XMPPSession.prototype.sendScrap = function() {
	
	for (var jid in this.participants) {
		this.sendScrapToJid(jid);
	}
}

XMPPSession.prototype.sendScrapToJid = function(jid) {
	this.log.debug('sendScrap()');
	
	try {
		$('chatsession_statusmessage').innerHTML = '<img src="http://localhost/images/ajax_action.gif" alt="" />';
		
		var memberToId = this.participants[jid].memberId;
		var scrapSecret = this.participants[jid].scrapSecret;
		this.log.debug('scrapSecret: ' + this.log.anonymizeReplace(scrapSecret));
		
		var pars = scrapSendPostman + '&returnType=none&reactionaddsecret=' + scrapSecret
		           + '&objectId=' + memberToId + '&objecttype=' + scrapObjectType
		           + '&objectsubtype=' + scrapObjectSubtype + '&type=1&send=verstuur&content=' + escape(this.rte.value);
		
		var self = this;
		new Ajax.Request('/', {
			method: 'post', 
			parameters: pars,
			onComplete: function(request) {
				self.afterScrapSent(request);
			},
			onException: function(request, exception) {
				logging.error('session.js:sendScrap(): ' + exception, exception);
			}
		});
	} catch(e) {
		this.log.error('session.js:sendScrapToJid(): ' + e, e);
	}
}

XMPPSession.prototype.afterScrapSent = function(request) {
	
	var responseText = request.responseText;
	var reactionReply = responseText.parseJSON();
	if (!reactionReply) {
		logging.error('session.js:afterScrapSent(): Could not parse reply: ' +  responseText);
		this.setStatusMessage(templates.translate('GENERAL_ERROR_UNEXPECTED'), 'red');
		return;
	}
	
	logging.debug('succesfully parsed reaction response, result = ' + reactionReply);
	
	$('chatsession_statusmessage').innerHTML = '';
	
	var self = this;
	if (reactionReply['aError']) {
		var aErrors = reactionReply['aError'].toArray();  
		logging.debug('found errors: ' + aErrors.inspect());
		aErrors.each(function(value, index) {
			self.setStatusMessage($('chatsession_statusmessage').innerHTML + value + '<br/>', 'red');
		});
	}
	
	if (reactionReply['aSuccess']) {
		var aSuccess = reactionReply['aSuccess'].toArray(); 
		aSuccess.each(function(value, index) {
			self.setStatusMessage($('chatsession_statusmessage').innerHTML + value + '<br/>', 'green');
		});
		
		this.rte.value = '';
		$('uiButtonSend').addClassName('disabled');
	}
}

XMPPSession.prototype.onAccountBalanceRetrieved = function(request) {
	this.log.debug('onAccountBalanceRetrieved()');
	
	try {
		var result = request.responseText.parseJSON();
		this.smsAccountBalance = result['accountBalance'] / 100.0;
		this.smsPrice = result['smsPrice'] / 100.0;
	} catch(e) {
		this.log.error('session.js:onAccountBalanceRetrieved(): ' + e, e);
	}
}

XMPPSession.prototype.recalculateSMSBodyLength = function() {
	
	if (this.rte.value.length > MAX_SMS_LENGTH) {
		this.rte.value = rte.value.substr(0, MAX_SMS_LENGTH);
	}
	var charsLeft = MAX_SMS_LENGTH - this.rte.value.length;
	
	if (!this.smsAccountBalanceRequested) {
		// let's see what is account balance of member of from member
		logging.debug('asking for account balance');
		this.smsAccountBalanceRequested = true;
		
		var self = this;
		new Ajax.Request('/?module=Chat&action=getMemberInfo', {
			method: 'get',
			onComplete: function(request) {
				self.onAccountBalanceRetrieved(request);
			},
			onException: function (request, exception) {
				logging.error('session.js:recalculateSMSBodyLength(): ' + exception, exception);
			}
		});
	}
	
	if (this.smsAccountBalance != null) {
		var messagesLeft = Math.floor(this.smsAccountBalance / this.smsPrice);
		var data = { charsLeft: charsLeft, creditsLeft: this.smsAccountBalance, messagesLeft: messagesLeft }
	} else {
		var data = { charsLeft: charsLeft, creditsLeft: null, messagesLeft: null }
	}
	
	templates.updateElement('sms_subject', 'chat.smsInfo', data);
	this.updateSessionHeight();
}

/**
 * Updates the display with member info (to member)
 */
XMPPSession.prototype.renderParticipantsInfo = function() {
	logging.debug('renderParticipantsInfo()');
	
	var jid = this.singleParticipantJid();
	if (jid) {
		templates.updateElement('participantsInfo', 'chat.participantInfo', {
			item: this.participants[jid]
		});
		
		$('historyLink').setStyle('display: inline');
	} else {
		templates.updateElement('participantsInfo', 'chat.multipleParticipantsInfo', {
			participants: this.participants
		});
		
		$('historyLink').setStyle('display: none');
	}
	
	if (this.canInviteContacts()) {
		$('inviteLink').setStyle('display: inline');
	} else {
		$('inviteLink').setStyle('display: none');
	}
	
	this.updateWindowTitle();
}

XMPPSession.prototype.updateWindowTitle = function() {
	
	var caption = '';
	for (var jid in this.participants) {
		if (caption != '') {
			caption += ', ';
		}
		
		caption += fancy_layout(this.participants[jid].nick, 'none');
	}
	
	window.windowManager.windowExecute(this.windowId, 'update', ObjectToJSONString({ caption: caption }));
}

XMPPSession.prototype.close = function() {
	this.log.debug('close()');
	
	try {
		this.xmppChatStateMachine.onGone();
	} catch(e) {
		logging.error('session.js:close()' + e, e);
	}
}

/**
 * This method is called when the message type is chat and the user 
 * presses a key in the RTE. In that case we might need to signal a new 
 * chatstate to the other party.
 */
XMPPSession.prototype.onRteKeyDown = function() {
	
	this.xmppChatStateMachine.onComposeInput();
}

/**
 * Opens the history window, where the user can view the complete history with a
 * user.
 */
XMPPSession.prototype.openHistoryWindow = function() {
	
	var jid = this.singleParticipantJid();
	if (!jid) {
		this.log.debug('Cannot open history window for multiple participants.');
		return;
	}
	
	window.windowManager.raiseEvent(ObjectToJSONString({
		name: 'openHistoryWindow',
		jid: jid
	}));
}

/**
 * Opens a popup window for inviting contacts to the session.
 */
XMPPSession.prototype.inviteContact = function() {
	
	this.log.usage('InviteButtonClicked(' + this.windowId + ')');
	
	if (this.inviteWindowId != 0) {
		window.windowManager.windowExecute(this.inviteWindowId, 'bringToFront');
		return;
	}
	
	var windowInfo = window.windowManager.windowInfoJSON(this.windowId).parseJSON();
	if (!windowInfo || windowInfo.id == 0) {
		this.log.error('no info about window!');
		return;
	}
	
	var height = 500;
	var width = 300;
	var top = Math.floor(windowInfo.top + (windowInfo.height / 2) - (height / 2));
	var left = Math.floor(windowInfo.left + (windowInfo.width / 2) - (width / 2));
	
	var args = {
		visible: true,
		caption: tr("Invite Contact"),
		left: left,
		top: top,
		height: height,
		width: width,
		minimumSize: true
	};
	this.inviteWindowId = this.windowManager.createWindow('invite', ObjectToJSONString(args));
	this.inviteWindowInitialized = false;
	
	window.windowManager.windowExecute(this.inviteWindowId, 'navigate',
		                           'url=http://localhost/statics/kwekker2/invite.html');
	window.windowManager.windowExecute(this.inviteWindowId, 'bringToFront');
}

/**
 * Called when the invite window is opened.
 */
XMPPSession.prototype.onInviteWindowAvailable = function(event) {
	this.log.debug('session.js:onInviteWindowAvailable()');
	
	if (this.inviteWindowId == 0 || this.inviteWindowInitialized) {
		return;
	}
	
	window.windowManager.raiseEvent(ObjectToJSONString({
		name: 'initInviteWindow',
		participants: this.participants,
		windowId: this.inviteWindowId,
		sessionWindowId: this.windowId,
		globalVars: globalVars()
	}));
	
	this.inviteWindowInitialized = true;
}

/**
 * Called when a new contact is going to be invited.
 */
XMPPSession.prototype.onInviteContact = function(event) {
	this.log.debug('session.js:onInviteContact(' + this.log.anonymizeEMail(event.jid) + ')');
	
	this.log.usage('ParticipantInvited(' + this.windowId + ')');
	
	var item = window.roster.itemJSON(event.jid).parseJSON();
	this.setStatusMessage(tr("Sending invitation to %1.").arg(fancy_layout(item.nick, 'nickname,nosmilies')));
	
	if (this.openInvitations.hasOwnProperty(event.jid) &&
	    !this.openInvitations[event.jid].joined) {
		return;
	}
	
	this.openInvitations[event.jid] = { jid: event.jid, acks: {}, joined: false, forwards: [], sessionId: undefined };
	
	for (var jid in this.participants) {
		this.openInvitations[event.jid].acks[jid] = false;
		
		window.windowManager.raiseEvent(ObjectToJSONString({
			name: 'sendInviteStart',
			toJid: this.participants[jid].jid, // make sure we select the full JID
			toSessionId: this.participants[jid].sessionId,
			inviteeJid: event.jid,
			fromSessionId: this.sessionId
		}));
	}
	
	var self = this;
	setTimeout(function() {
		window.windowManager.closeWindow(self.inviteWindowId);
	}, 20);
	
	setTimeout(function() {
		self.checkInviteAcksReceived(event.jid);
	}, 10000);
}

/**
 * Called when an acknowledgment for an invitation is received.
 */
XMPPSession.prototype.onInviteAckReceived = function(event) {
	this.log.debug('session.js:onInviteAckReceived()');
	
	if (!this.openInvitations.hasOwnProperty(event.inviteeJid)) {
		this.log.debug('Received an inviteack message, but there is no open invitation to the invitee.');
		return;
	}
	
	var bareJid = jidbare(event.fromJid);
	
	if (!this.participants.hasOwnProperty(bareJid)) {
		this.log.debug('Received an inviteack message from ' + this.log.anonymizeEMail(event.fromJid) + ' who is not part of the session.');
		return;
	}
	
	this.openInvitations[event.inviteeJid].acks[bareJid] = true;
	this.participants[bareJid].jid = event.fromJid;
	this.participants[bareJid].sessionId = event.fromSessionId;
	
	this.checkAndSendSessionInvite(event.inviteeJid);
}

/**
 * Called when the start of an invitation is received.
 */
XMPPSession.prototype.onInviteStartReceived = function(event) {
	
	var bareJid = jidbare(event.fromJid);
	
	if (!this.participants.hasOwnProperty(bareJid)) {
		this.log.debug('Received an invitestart message from ' + this.log.anonymizeEMail(event.fromJid) + ' who is not part of the session.');
		return;
	}
	
	this.participants[bareJid].jid = event.fromJid;
	this.participants[bareJid].sessionId = event.fromSessionId;
}

/**
 * Called when a new participant has joined the session.
 */
XMPPSession.prototype.onSessionJoined = function(event) {
	
	this.log.usage('ParticipantJoined(' + this.windowId + ')');
	
	var jid = this.singleParticipantJid();
	if (jid) {
		window.windowManager.raiseEvent(ObjectToJSONString({
			name: 'unregister1to1Session',
			jid: jid
		}));
	}
	
	var bareJid = jidbare(event.inviteeJid);
	if (this.openInvitations.hasOwnProperty(bareJid)) {
		// this is someone we sent an invitation to
		if (this.openInvitations[bareJid].joined == true) {
			return; // duplicate sessionjoined message
		}
		
		this.openInvitations[bareJid].jid = event.inviteeJid;
		this.openInvitations[bareJid].sessionId = event.inviteeSessionId;
		this.openInvitations[bareJid].joined = true;
		
		for (var i = 0; i < this.openInvitations[bareJid].forwards.length; i++) {
			this.forwardSessionJoinedMessage(event.inviteeJid, this.openInvitations[bareJid].forwards[i]);
		}
		
		var self = this;
		setTimeout(function() {
			// set the timer to delete the invitations later,
			// after this period we will stop forwarding
			// sessionjoined messages
			if (self.openInvitations.hasOwnProperty(bareJid) &&
			    self.openInvitations[bareJid].joined) {
				delete self.openInvitations[bareJid];
			}
		}, 20000);
	} else {
		if (this.participants.hasOwnProperty(bareJid)) {
			return; // duplicate sessionjoined message
		}
		
		// invitee was invited by someone else
		for (var bareInviteeJid in this.openInvitations) {
			if (this.openInvitations[bareInviteeJid].joined) {
				this.forwardSessionJoinedMessage(this.openInvitations[bareInviteeJid].jid, event);
			} else {
				// queue messages for someone not yet in the session
				this.openInvitations[bareInviteeJid].forwards.push(event);
			}
		}
	}
	
	var item = window.roster.itemJSON(bareJid).parseJSON();
	item.jid = event.inviteeJid;
	item.sessionId = event.inviteeSessionId;
	this.participants[bareJid] = item;
	
	this.appendStatusMessage(tr('%1 has joined the conversation.').arg(fancy_layout(item.nick, 'nickname,nosmilies')));
	
	this.renderParticipantsInfo();
}

/**
 * Forwards a sessionjoined message from one contact to another. This is done
 * for contacts which were invited by us, and which may not yet have received
 * the sessionjoined from the invitee.
 */
XMPPSession.prototype.forwardSessionJoinedMessage = function(toJid, event) {
	this.log.debug('session.js:forwardSessionJoinedMessage(' + this.log.anonymizeEMail(toJid) + ', ...)');
	
	window.windowManager.raiseEvent(ObjectToJSONString({
		name: 'sendSessionJoined',
		toJid: toJid,
		toSessionId: this.openInvitations[jidbare(toJid)].sessionId,
		fromSessionId: event.inviteeSessionId,
		forwardFrom: event.inviteeJid
	}));
}

/**
 * Called when a participant has left the session.
 */
XMPPSession.prototype.onSessionClosed = function(event) {
	
	var bareJid = jidbare(event.participantJid);
	if (!this.participants.hasOwnProperty(bareJid)) {
		this.log.debug('Received a sessionclose message from ' + this.log.anonymizeEMail(event.participantJid) + ' who is not part of the session.');
		return;
	}
	
	if ($H(this.participants).keys().length > 1) {
		this.appendStatusMessage(tr('%1 has left the conversation.').arg(fancy_layout(this.participants[bareJid].nick, 'nickname,nosmilies')));
		
		delete this.participants[bareJid];
		
		// convert this session back to a regular 1-to-1 session if there is
		// only one participant left (otherwise the participant will refuse the
		// incoming message since the MUC session is closed on his side)
		var jid = this.singleParticipantJid();
		if (jid) {
			window.windowManager.raiseEvent(ObjectToJSONString({
				name: 'register1to1Session',
				jid: jid,
				windowId: this.windowId
			}));
		}
	} else {
		delete this.participants[bareJid].sessionId;
	}
	
	this.onChatStateReceived(event.participantJid, 'inactive');
	
	this.renderParticipantsInfo();
}

/**
 * Checks whether we can send a session invite to the given JID and sends it if
 * ready.
 */
XMPPSession.prototype.checkAndSendSessionInvite = function(inviteeJid) {
	
	for (var jid in this.openInvitations[inviteeJid].acks) {
		if (!this.openInvitations[inviteeJid].acks[jid]) {
			return;
		}
	}
	
	window.windowManager.raiseEvent(ObjectToJSONString({
		name: 'sendSessionInvite',
		toJid: inviteeJid,
		fromSessionId: this.sessionId,
		participants: this.participants
	}));
}

/**
 * Checks whether all invitestart messages have been acknowledged. If not, the
 * invitation procedure is aborted.
 *
 * This method is called 10 seconds after the invitestart messages are sent.
 *
 * @param inviteeJid JID of the invitee.
 */
XMPPSession.prototype.checkInviteAcksReceived = function(inviteeJid) {
	
	if (!this.openInvitations.hasOwnProperty(inviteeJid) ||
	    this.openInvitations[inviteeJid].joined) {
		return;
	}
	
	this.log.usage('ParticipantNotJoined(' + this.windowId + ')');
	
	var item = window.roster.itemJSON(inviteeJid).parseJSON();
	this.setStatusMessage(tr("%1 could not be invited.").arg(fancy_layout(item.nick, 'nickname,nosmilies')));
	
	delete this.openInvitations[inviteeJid];
}

/**
 * Notifies participants we are leaving the session in case of multi-user
 * sessions.
 */
XMPPSession.prototype.onWindowClosed = function() {
	
	this.log.usage('SessionWindowClosed(' + this.windowId + ')');
	
	for (var jid in this.participants) {
		var participant = this.participants[jid];
		if (!participant.sessionId) {
			continue;
		}
		
		window.windowManager.raiseEvent(ObjectToJSONString({
			name: 'sendSessionClosed',
			toJid: participant.jid,
			toSessionId: participant.sessionId,
			fromSessionId: this.sessionId
		}));
	}
}

/**
 * Called when an invitation window is closed.
 *
 * @param windowId ID of the closed window.
 */
XMPPSession.prototype.onInviteWindowClosed = function(windowId) {
	
	if (windowId == this.inviteWindowId) {
		this.inviteWindowId = 0;
	}
}

/**
 * Initializes the session window on window load event.
 */
function sessionWindowOnload() {
	logging.debug('sessionWindowOnload()');
	
	window.xmppSession = new XMPPSession();
}

/**
 * Callback for when the window is resized (called from WebView.cpp).
 */
function windowResized(width, height) {
	
	if (window.xmppSession) {
		window.xmppSession.updateSessionHeight();
	}
}

/**
 * Callback for when the window is closed (called from ScriptWindow.cpp).
 */
function windowClosed() {
	
	if (window.xmppSession) {
		window.xmppSession.onWindowClosed();
	}
}
