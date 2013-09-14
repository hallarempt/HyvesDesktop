var XMPP_HYVES_NS = 'http://hyves.net/xmpp';

templates.addFromString('chat.tmplXmppWwwUpdate', '<iq from=${fromJid|quoteAttr} type=\'set\'><query xmlns=\'' + XMPP_HYVES_NS + '\'><item jid=${fromJid|quoteAttr} wwwEmotion=${wwwEmotion|quoteAttr} wwwLocation=${wwwLocation|quoteAttr}/></query></iq>');
templates.addFromString('chat.tmplXmppMessage', '<message id=${msgId|quoteAttr} to=${toJid|quoteAttr}{if toSessionId} toSessionId=${toSessionId|quoteAttr}{/if} type=\'chat\'>{if msg}<body>${msg|quoteElement}</body>{/if}{if extra}${extra}{/if}</message>');
templates.addFromString('chat.tmplXmppPresence', '<presence{if type} type=${type|quoteAttr}{/if}>{if show}<show>${show}</show>{/if}{if status}<status>${status}</status>{/if}{if priority}<priority>${priority}</priority>{/if}</presence>');
templates.addFromString('chat.tmplXmppRosterRequest', '<iq id=${id|quoteAttr} type="get"><query xmlns="jabber:iq:roster" /></iq>');
templates.addFromString('chat.tmplXmppExtendedRosterRequest', '<iq id=${id|quoteAttr} type="get"><query xmlns="' + XMPP_HYVES_NS + '" /></iq>');

var mucMessageBody = '<body>' + tr('You are invited for a group chat. Please download Hyves Desktop from http://www.hyves.nl/chat/download to be able to join group chats.') + '</body>';
templates.addFromString('chat.tmplXmppInviteStart', '<message id=${msgId|quoteAttr} to=${toJid|quoteAttr} type="chat"><invitestart xmlns="' + XMPP_HYVES_NS + '" invitee=${inviteeJid|quoteAttr}{if toSessionId} toSessionId=${toSessionId|quoteAttr}{/if} fromSessionId=${fromSessionId|quoteAttr} />' + mucMessageBody + '</message>');
templates.addFromString('chat.tmplXmppInviteAck', '<message id=${msgId|quoteAttr} to=${toJid|quoteAttr} type="chat"><inviteack xmlns="' + XMPP_HYVES_NS + '" invitee=${inviteeJid|quoteAttr} toSessionId=${toSessionId|quoteAttr} fromSessionId=${fromSessionId|quoteAttr} />' + mucMessageBody + '</message>');
templates.addFromString('chat.tmplXmppSessionInvite', '<message id=${msgId|quoteAttr} to=${toJid|quoteAttr} type="chat"><sessioninvite xmlns="' + XMPP_HYVES_NS + '" fromSessionId=${fromSessionId|quoteAttr}><participants>{for participant in participants}<participant jid=${participant.jid|quoteAttr} sessionId=${participant.sessionId|quoteAttr}/>{/for}</participants></sessioninvite>' + mucMessageBody + '</message>');
templates.addFromString('chat.tmplXmppSessionJoined', '<message id=${msgId|quoteAttr} to=${toJid|quoteAttr} type="chat"><sessionjoined xmlns="' + XMPP_HYVES_NS + '"{if forwardFrom} forwardFrom=${forwardFrom|quoteAttr}{/if} toSessionId=${toSessionId|quoteAttr} fromSessionId=${fromSessionId|quoteAttr} />' + mucMessageBody + '</message>');
templates.addFromString('chat.tmplXmppSessionClosed', '<message id=${msgId|quoteAttr} to=${toJid|quoteAttr} type="chat"><sessionclosed xmlns="' + XMPP_HYVES_NS + '" toSessionId=${toSessionId|quoteAttr} fromSessionId=${fromSessionId|quoteAttr} /></message>');

function XMPPClient(ui, container, chatPlatformEnabled) {
	this.log.debug('client.js:XMPPClient()');
	
	this.ui = ui;
	this.container = container;
	this.isInactive = false;
	this.presenceLastShowStatusBeforeInactive = '';
	this.chatPlatformEnabled = chatPlatformEnabled;
	
	// can be set to override current hyves presence on sigin
	this.initialPresenceStatus = window.settingsManager.stringValue('General/chatInitialPresence');
	this.log.debug('Initial presence: ' + this.initialPresenceStatus);
	
	this.pendingIQs = {}; // IQ ID => callback name
	
	this.userame = '';
	this.password = '';
	this.realm = '';
	this.digest = '';
	this.messageId = 0;
	
	this.blockedCache = {}; // cache for blocked jids
	this.nonBlockedCache = {}; // cache for non blocked jids
	
	this.openingMucWindows = {}; // jid => inviteeJid
	
	this.manualDisconnect = false;
	this.autoReloadInterval = 0;
	
	this.container.setClient(this);
	
	window.extender.connect('jabber', 'connected()', 'window.xmppClient.onConnect');
	window.extender.connect('jabber', 'disconnected()', 'window.xmppClient.onDisconnect');
	window.extender.connect('jabber', 'connectionError(int)', 'window.xmppClient.onError');
	window.extender.connect('jabber', 'incomingXml(QDomDocument)', 'window.xmppClient.onIncomingXml');
	window.extender.connect('roster', 'initialized()', 'window.xmppClient.onRosterInitialized');
	window.extender.connect('roster', 'itemChanged(QVariantMap, QVariantMap)', 'window.xmppClient.onItemChanged');
	window.extender.connect('windowManager', 'eventRaised(QVariantMap)', 'window.xmppClient.onEventRaised');
	
	this.log.debug('initialized!');
}

XMPPClient.prototype.log = logging.getLogger("XMPPClient");

XMPPClient.prototype.onEventRaised = function(event) {
	
	if (event.name == 'retrieveMemberInfo') {
		this.retrieveMemberInfo(event.jid, event.includeSecrets);
	} else if (event.name == 'sendInviteAck') {
		this.sendInviteStart(event);
	} else if (event.name == 'sendInviteStart') {
		this.sendInviteStart(event);
	} else if (event.name == 'sendMessage') {
		this.sendMessage(event);
	} else if (event.name == 'sendSessionInvite') {
		this.sendSessionInvite(event);
	} else if (event.name == 'sendSessionJoined') {
		this.sendSessionJoined(event);
	} else if (event.name == 'sendSessionClosed') {
		this.sendSessionClosed(event);
	} else if (event.name == 'sessionRegistered') {
		this.onSessionRegistered(event.participants);
	} else if (event.name == 'sessionUnregistered') {
		this.onSessionUnregistered(event.jid);
	} else if (event.name == 'wwwChanged') {
		this.onWwwChanged(event.emotion, event.where);
	}
}

/**
 * Gets the current chatability for this member.
 *
 * This is updated through the own item in the roster model.
 */
XMPPClient.prototype.getChatability = function() {
	
	var item = window.roster.ownItemJSON().parseJSON();
	if (item && item.hasOwnProperty('chatability')) {
		return item.chatability;
	} else {
		return 0; // no chatability, will block messages as long as owner item is unknown
	}
}

/**
 * Checks whether an item exists in the roster, and updates the cache according
 * to the blocked property of the item.
 */
XMPPClient.prototype.updateBlockedCacheForJid = function(jid) {
	
	var item = window.roster.itemJSON(jid).parseJSON();
	if (item && item.hasOwnProperty('blocked')) {
		this.setBlockedValueForJid(jid, (item.blocked == 'True'));
	}
}

XMPPClient.prototype.setBlockedValueForJid = function(jid, blocked) {
	
	this.blockedCache[jid] = blocked;
	this.nonBlockedCache[jid] = !blocked;		
}

XMPPClient.prototype.removeJidFromBlockedCache = function(jid) {
	
	delete this.blockedCache[jid];
	delete this.nonBlockedCache[jid];		
}

XMPPClient.prototype.signin = function() {
	this.log.debug('client.js:signin(): username: ' + this.log.anonymizeReplace(this.username) + ', server: ' + this.log.anonymizeIPAddr(this.host) + ":" + this.port);
	
	window.jabber.setProtocolVersion(1);
	window.jabber.setHostAndPort(this.host, this.port);
	if (this.digest != "") {
		window.jabber.setDigest(this.digest);
	}
	window.jabber.signIn(this.username, this.realm, this.password, this.resource);
}

XMPPClient.prototype.disconnect = function() {
	
	this.container.closeOpenSessions();
	this.manualDisconnect = true;
	window.jabber.signOut();
}

XMPPClient.prototype.sendMessage = function(event) {
	this.log.debug('client.js:sendMessage(' + this.log.anonymizeEMail(event.toJid) + ', ' + event.toSessionId + ', ...)');
	
	var messageId = event.messageId;
	if (!messageId) {
		this.messageId++;
		messageId = 'm_' + this.messageId;
	}
	
	window.jabber.sendXml(templates.process('chat.tmplXmppMessage', {
		toJid: event.toJid,
		toSessionId: event.toSessionId,
		msg: event.message,
		msgId: messageId,
		extra: event.extra
	}));
}

XMPPClient.prototype.sendInviteStart = function(event) {
	this.log.debug('client.js:sendInviteStart(' + this.log.anonymizeEMail(event.toJid) + ', ' + this.log.anonymizeEMail(event.inviteeJid) + ', ' + event.sessionId + ')');
	
	this.messageId++;
	
	window.jabber.sendXml(templates.process('chat.tmplXmppInviteStart', {
		toJid: event.toJid,
		inviteeJid: event.inviteeJid,
		toSessionId: event.toSessionId,
		fromSessionId: event.fromSessionId,
		msgId: 'm_' + this.messageId
	}));
}

XMPPClient.prototype.sendInviteAck = function(event) {
	this.log.debug('client.js:sendInviteAck(' + this.log.anonymizeEMail(event.toJid) + ', ' + this.log.anonymizeEMail(event.inviteeJid) + ', ' + event.toSessionId + ', ' + event.fromSessionId + ')');
	
	this.messageId++;
	
	window.jabber.sendXml(templates.process('chat.tmplXmppInviteAck', {
		toJid: event.toJid,
		inviteeJid: event.inviteeJid,
		toSessionId: event.toSessionId,
		fromSessionId: event.fromSessionId,
		msgId: 'm_' + this.messageId
	}));
}

XMPPClient.prototype.sendSessionInvite = function(event) {
	this.log.debug('client.js:sendSessionInvite(' + this.log.anonymizeEMail(event.toJid) + ', ' + event.fromSessionId + ', ...)');
	
	this.messageId++;
	
	window.jabber.sendXml(templates.process('chat.tmplXmppSessionInvite', {
		toJid: event.toJid,
		fromSessionId: event.fromSessionId,
		participants: event.participants,
		msgId: 'm_' + this.messageId
	}));
}

XMPPClient.prototype.sendSessionJoined = function(event) {
	this.log.debug('client.js:sendSessionJoined(' + this.log.anonymizeEMail(event.toJid) + ', ' + event.toSessionId + ', ' + event.fromSessionId + ', ' + event.forwardFrom + ')');
	
	this.messageId++;
	
	window.jabber.sendXml(templates.process('chat.tmplXmppSessionJoined', {
		toJid: event.toJid,
		toSessionId: event.toSessionId,
		fromSessionId: event.fromSessionId,
		forwardFrom: event.forwardFrom,
		msgId: 'm_' + this.messageId
	}));
}

XMPPClient.prototype.sendSessionClosed = function(event) {
	this.log.debug('client.js:sendSessionClosed(' + this.log.anonymizeEMail(event.toJid) + ', ' + event.toSessionId + ', ' + event.fromSessionId + ')');
	
	this.messageId++;
	
	window.jabber.sendXml(templates.process('chat.tmplXmppSessionClosed', {
		toJid: event.toJid,
		toSessionId: event.toSessionId,
		fromSessionId: event.fromSessionId,
		msgId: 'm_' + this.messageId
	}));
}

XMPPClient.prototype.sendRosterRequest = function() {
	
	this.sendIQ('chat.tmplXmppRosterRequest', 'roster_5', 'handleRosterUpdate');
}

XMPPClient.prototype.sendExtendedRosterRequest = function() {
	
	this.sendIQ('chat.tmplXmppExtendedRosterRequest', 'Hyves_Roster_Extension');
}

XMPPClient.prototype.sendIQ = function(template, id, callback, data) {
	
	if (!data) {
		data = new Object();
	}
	
	if (callback) {
		this.pendingIQs[id] = callback;
	}
	
	data.id = id;
	window.jabber.sendXml(templates.process(template, data));
}

XMPPClient.prototype.sendInitialPresence = function() {
	
	if (this.initialPresenceStatus) {
		this.setPresenceStatus(this.initialPresenceStatus);
	} else {
		this.setPresenceStatus('online');
	}
}

XMPPClient.prototype.setPresenceStatus = function(status) {
	this.log.debug('client.js:setPresenceStatus(' + status + ')');
	
	var ownItem = window.roster.ownItemJSON().parseJSON();
	if (ownItem.chatability <= 1) {
		status = 'offline';
	}
	
	// send presence to chat server
	if (status == 'offline') {
		this.sendPresence('invisible'); // appear offline
	} else {
		this.sendPresence('', window.roster.statusShow(status), status, this.priority);
	}
	
	window.roster.setOwnStatus(status);
		
	return status;
}

XMPPClient.prototype.sendPresence = function(type, show, status, priority) {
	
	window.jabber.sendXml(templates.process('chat.tmplXmppPresence', {
		type: type,
		show: show,
		status: status,
		priority: priority
	}));
}

XMPPClient.prototype.getPresenceStatus = function() {
	
	return window.roster.ownItemJSON().parseJSON().status;
}

XMPPClient.prototype.onSessionRegistered = function(participants) {
	this.log.debug('client.js:onSessionRegistered()');
	
	for (var jid in participants) {
		if (!window.roster.hasItem(jid)) {
			this.setBlockedValueForJid(jid, false);
		}
	}
}

XMPPClient.prototype.onSessionUnregistered = function(jid) {
	this.log.debug('client.js:onSessionUnregistered()');
	
	if (!window.roster.hasItem(jid)) {
		this.log.debug('unregister session for non friend, removing from blocked cache: ' + this.log.anonymizeEMail(jid));
		this.removeJidFromBlockedCache(jid);
	}
}

XMPPClient.prototype.alertError = function(key, errorcode) {
	
	var message = templates.translate(key, { code: errorcode });
	this.ui.alert(message);
}

XMPPClient.prototype.reconnect = function(autoReconnect, seconds) {
	
	templates.updateElement('disconnectedbox', 'chat.tmplReconnect', { autoReconnect: autoReconnect });
	
	if (!seconds) {
		seconds = window.mainWindow.reconnectSeconds();
	}
	
	if (!this.autoReloadInterval && autoReconnect && seconds) {
		var reloadCountdownStart = new Date();
		var reloadStarted = false;
		var secondsTotal = seconds;
		this.autoReloadInterval = setInterval(function() {
			var msPassed = new Date().getTime() - reloadCountdownStart.getTime();
			var msRemaining = (secondsTotal * 1000) - msPassed;
			var secondsRemaining = Math.round(msRemaining / 1000);
			
			Element.update('autoReconnectSeconds', templates.translate('CHAT_AUTORECONNECT_SECONDS', {
				secondsRemaining: (secondsRemaining > 0 ? secondsRemaining : 0)
			}));
			
			if (secondsRemaining <= 0 && !reloadStarted) {
				window.mainWindow.reloadKwekker();
				reloadStarted = true;
			}
		}, 200);
	}
}

XMPPClient.prototype.isRosterListLoaded = function(rosterList) {
	
	return (this.rosterUpdateCount >= XMPP_NUM_INITIAL_ROSTER_UPDATES);
}

XMPPClient.prototype.onRosterInitialized = function() {
	
	setTimeout(windowResized, 100);
	
	window.mainWindow.resetReconnectSeconds();
	
	var self = this;
	setTimeout(function() {
		var ownItem = window.roster.ownItemJSON().parseJSON();
		if (ownItem.chatability <= 1) {
			self.forceOfflineBecauseOfChatability();
		}
	}, 200);
}

XMPPClient.prototype.onItemChanged = function(item, changedProperties) {
	
	if (changedProperties.hasOwnProperty('blocked')) {
		this.setBlockedValueForJid(item.jid, (item.blocked == 'True'));
	}
	
	if (item.jid == window.roster.ownBareJid() && changedProperties.hasOwnProperty('chatability')) {
		if (item.chatability <= 1) {
			this.forceOfflineBecauseOfChatability();
		}
	}
}

XMPPClient.prototype.forceOfflineBecauseOfChatability = function() {
	
	this.setPresenceStatus('offline');
	
	window.windowManager.showMessage(window.mainWindow.windowId(), '',
		tr('Your status has been set to offline, because you have selected that nobody is allowed to chat with you. ' +
		   'Check <a href="http://www.hyves.nl/profielbeheer/privacy/hyvesdesktop/">your settings at Hyves</a> and try again.')
	);
}

XMPPClient.prototype.onConnect = function() {
	this.log.debug('client.js:onConnect()');
	
	this.sendRosterRequest();
	
	this.container.onClientAuthorized();
}

/**
 * Called from the desktop if the user is active.
 */ 
XMPPClient.prototype.onDesktopStateActive = function() {
	
	// if the user is inactive, wake him up
	if (this.isInactive) {
		this.log.debug('waking up from inactive state, restoring old state ' + this.presenceLastShowStatusBeforeInactive);
		this.isInactive = false;
		this.setPresenceStatus(this.presenceLastShowStatusBeforeInactive);
	}
}

/**
 * Called form the desktop if the user is inactive.
 */ 
XMPPClient.prototype.onDesktopStateInactive = function() {
	
	// only go to away if
	// 1. the desktop client is the last connected resource for this user
	// 2. the current state is active
	// 3. the current presence show status is not offline
	var status = this.getPresenceStatus();
	var resources = window.roster.ownItemResourcesJSON().parseJSON().resources;
	if (resources.length == 1 && !this.isInactive && status != 'offline') {
		this.log.debug('storing current presence status: ' + status);
		this.presenceLastShowStatusBeforeInactive = status;
		this.isInactive = true;
		this.setPresenceStatus('away');
	}
}

XMPPClient.prototype.onDisconnect = function() {
	this.log.debug('client.js:onDisconnect()');
	
	this.container.onDisconnect();
}

XMPPClient.prototype.onError = function(errorCode) {
	this.log.debug('client.js:onError(' + errorCode + ')');
	
	
	
	if (errorCode == '401') {
		this.alertError('CHAT_ERROR_INVALID_LOGIN', errorCode);
		this.reconnect(false);
	} else if (errorCode == 10 || errorCode == '503') {
		this.alertError('CHAT_ERROR_SERVER_UNREACHABLE', errorCode);
		if (this.chatPlatformEnabled) {
			// should be able to connect (firewall?)
			this.alertError('CHAT_ERROR_SERVER_UNREACHABLE_FIREWALL', errorCode);
			this.reconnect(true);
		} else {
			// scheduled maintenance
			this.alertError('CHAT_ERROR_SERVER_UNREACHABLE_MAINTENANCE', errorCode);
			this.reconnect(true, 3600);
		}
	} else if(errorCode == '409') {
		this.alertError('CHAT_ERROR_CONFLICT', errorCode);
		this.reconnect(false);
	} else {
		this.alertError('CHAT_ERROR_GENERAL', errorCode);
		this.reconnect(true);
	}
}

XMPPClient.prototype.onHeadline = function(fromJid, message) {
	
	window.windowManager.raiseEvent(ObjectToJSONString({
		name: 'headlineReceived',
		subject: message.subject,
		body: message.body,
		url: message.notification['@url'],
		date: message.notification['@datetime'],
		fromJid: fromJid
	}));
}

/**
 * Returns whether we accept incoming messages from the given JID.
 *
 * @return true if the message is accepted, false if the message is not
 *         accepted, or null if it cannot be determined whether we want to
 *         accept. In the latter case, a site callback is required for fetching
 *         the blocked status.
 */
XMPPClient.prototype.acceptMessagesFrom = function(jid) {
	
	bareJid = jidbare(jid);
	
	// if there already is an option session, then we always allow the message to proceed
	if (this.container.hasSession(bareJid)) {
		this.log.debug(this.log.anonymizeEMail(bareJid) + ' is not blocked, because there is already a session');
		return true;
	}
	
	// if we are 'offline', we always block incoming messages
	if (this.getPresenceStatus() == 'offline') {
		this.log.debug('rejected message from ' + this.log.anonymizeEMail(jid) + ' because we appear offline');
		return false;
	}
	
	// if we have chatability 'nobody' or lower than we always block incoming messages
	if (this.getChatability() <= 1) {
		this.log.debug('rejected message from ' + this.log.anonymizeEMail(jid) + ' because of chatability setting');
		return false;
	}
	
	this.updateBlockedCacheForJid(bareJid);
	
	if (this.blockedCache[bareJid]) {
		this.log.debug(this.log.anonymizeEMail(bareJid) + ' is blocked');
		return false;
	}
	
	if (this.nonBlockedCache[bareJid]) {
		this.log.debug(this.log.anonymizeEMail(bareJid) + ' is not blocked');
		return true;
	}
	
	return null;
}

/**
 * A message came in from fromJid. 
 *
 * Depending on security settings we may or may not deliver it to the session
 * associated with fromJid.
 */
XMPPClient.prototype.onMessage = function(fromJid, toSessionId, message) {
	this.log.debug('client.js:onMessage(' + this.log.anonymizeEMail(fromJid) + ', ' + toSessionId + ', ...)');
	
	var acceptMessage = this.acceptMessagesFrom(fromJid);
	
	if (acceptMessage === true) {
		this.deliverMessage(fromJid, toSessionId, message);
	} else if (acceptMessage === false) {
		// do nothing... the message is discarded
	} else {
		this.log.debug(this.log.anonymizeEMail(fromJid) + " has unknown blocking status now fetching");
		
		var self = this;
		new Ajax.Request('/index.php?module=chat&action=onNewMessage', {
			parameters: {
				fromJid: jidbare(fromJid)
			},
			onSuccess: function(request) {
				var response = request.responseText.parseJSON();
				self.onNewMessageResponse(response, fromJid, message);
			}
		});
	}
}

XMPPClient.prototype.onNewMessageResponse = function(response, fromJid, message) {
	this.log.debug('client.js:onNewMessageResponse()');
	
	var bareJid = response.fromJid;
	if (response.isBlocked) {
		this.log.debug(this.log.anonymizeEMail(bareJid) + " is blocked, adding to blocked cache");
		this.blockedCache[bareJid] = true;
	} else {
		this.log.debug(this.log.anonymizeEMail(bareJid) + " is free to talk, adding to non blocked cache");
		this.nonBlockedCache[bareJid] = true;
		this.deliverMessage(fromJid, null, message);
	}
}

XMPPClient.prototype.onChatState = function(fromJid, toSessionId, state) {
	this.log.debug('client.js:onChatState(' + this.log.anonymizeEMail(fromJid) + ', ' + toSessionId + ', ' + state + ')');
	
	bareJid = jidbare(fromJid);
	
	// we only do something with those if we have a session
	if (this.container.hasSession(bareJid) || this.container.sessionToWindowMap.hasOwnProperty(toSessionId)) {
		window.windowManager.raiseEvent(ObjectToJSONString({
			name: 'chatStateReceived',
			jid: fromJid,
			toSessionId: toSessionId,
			state: state
		}));
	}
}

XMPPClient.prototype.onReceiptRequest = function(fromJid, messageId) {
	this.log.debug('client.js:onReceiptRequest(' + this.log.anonymizeEMail(fromJid) + ', ' + messageId + ')');
	
	if (window.settingsManager.boolValue('General/sendMessageReceipts', true)) {
		this.sendMessage({ toJid: fromJid, messageId: messageId, extra: '<received xmlns=\'urn:xmpp:receipts\'/>' });
	}
}

XMPPClient.prototype.onReceiptReceived = function(fromJid, messageId) {
	this.log.debug('client.js:onReceiptReceived(' + this.log.anonymizeEMail(fromJid) + ', ' + messageId + ')');
	
	window.windowManager.raiseEvent(ObjectToJSONString({
		name: 'receiptReceived',
		jid: fromJid,
		messageId: messageId
	}));
}

/**
 * Retrieves member info from the site about member with given
 * jid. This extra info (nick, scrapSecret, etc) is merged
 * with roster item on retrieval
 */
XMPPClient.prototype.retrieveMemberInfo = function(jid, includeSecrets) {
	this.log.debug('client.js:retrieveMemberInfo(' + this.log.anonymizeEMail(jid) + ', ' + this.log.anonymizeReplace(includeSecrets) + ')');
	
	var self = this;
	var url = '?index.php&module=Chat&action=getMemberInfo&jid=' + jid;
	if (includeSecrets) {
		url += '&includeSecrets=1';
	}
	new Ajax.Request(url, {
		method: 'get', 
		onComplete: function(request) {
			self.onMemberInfoRetrieved(request);
		},
		onException: function(request, exception) {
			logging.error('client.js:retrieveMemberInfo: ' + exception, exception);
		}
	});
}

/**
 * Site has responded with extra member info, now merge it with roster
 */
XMPPClient.prototype.onMemberInfoRetrieved = function(request) {
	this.log.debug('client.js:onMemberInfoRetrieved()');
	
	try {
		var item = request.responseText.parseJSON();
		if (!item) {
			logging.error('client.js:onMemberInfoRetrieved(): Could not parse reply: ' +  request.responseText);
			return;
		}
		
		if (!item.jid) {
			logging.error('client.js:onMemberInfoRetrieved(): Item did not contain JID.');
			return;
		}
		
		var jid = item.jid;
		delete item.jid;
		
		window.roster.setItemProperties(jid, ObjectToJSONString(item));
	} catch(e) {
		this.log.error('client.js:onMemberInfoRetrieved(): ' + e, e);
	}
}

XMPPClient.prototype.onWwwChanged = function(emotion, where) {
	this.log.debug('client.js:onWwwChanged()');
	
	try {
		window.roster.setItemProperties(window.roster.ownBareJid(), ObjectToJSONString({
			wwwEmotion: emotion,
			wwwLocation: where,
			wwwTime: (new Date()).toString()
		}));
	} catch(e) {
		this.log.error('client.js:onWwwChanged(): ' + e, e);
	}
}

/**
 * Handles an incoming start of invitation.
 *
 * @param fromJid The contact that started the invitation.
 * @param inviteeJid JID of the invitee.
 * @param fromSessionId ID of the session used by the inviter.
 */
XMPPClient.prototype.onInviteStart = function(fromJid, inviteeJid, toSessionId, fromSessionId) {
	this.log.debug('client.js:onInviteStart(' + this.log.anonymizeEMail(fromJid) + ', ' + this.log.anonymizeEMail(inviteeJid) + ', ' + toSessionId + ', ' + fromSessionId + ')');
	
	if (!this.acceptMessagesFrom(fromJid)) {
		return;
	}
	
	var sessionId;
	if (toSessionId) {
		if (this.container.sessionToWindowMap.hasOwnProperty(toSessionId)) {
			sessionId = toSessionId;
		} else {
			this.log.debug('Received an invitestart message from ' + this.log.anonymizeEMail(fromJid) + ', but session is not registered.');
			return;
		}
	} else {
		sessionId = this.container.ensureMucSession(fromJid, fromSessionId);
	}
	
	this.sendInviteAck({
		toJid: fromJid,
		inviteeJid: inviteeJid,
		toSessionId: fromSessionId,
		fromSessionId: '' + sessionId
	});
	
	window.windowManager.raiseEvent(ObjectToJSONString({
		name: 'inviteStartReceived',
		fromJid: fromJid,
		fromSessionId: fromSessionId,
		inviteeJid: inviteeJid,
		toSessionId: '' + sessionId
	}));
	
	this.setBlockedValueForJid(inviteeJid, false);
}

/**
 * Handles an incoming acknowledgment of an invitation start.
 *
 * @param fromJid The contact that acknowledged the invitation.
 * @param inviteeJid The contact that is going to be invited.
 * @param sessionId ID of the session used by the acknowledging contact.
 */
XMPPClient.prototype.onInviteAck = function(fromJid, inviteeJid, toSessionId, fromSessionId) {
	this.log.debug('client.js:onInviteAck(' + this.log.anonymizeEMail(fromJid) + ', ' + this.log.anonymizeEMail(inviteeJid) + ', ' + toSessionId + ', ' + fromSessionId + ')');
	
	if (!this.acceptMessagesFrom(fromJid)) {
		return;
	}
	
	window.windowManager.raiseEvent(ObjectToJSONString({
		name: 'inviteAckReceived',
		fromJid: fromJid,
		fromSessionId: fromSessionId,
		inviteeJid: inviteeJid,
		toSessionId: toSessionId
	}));
}

/**
 * Handles an incoming acknowledgment of an invitation start.
 *
 * @param fromJid The contact that acknowledged the invitation.
 * @param inviteeJid The contact that is going to be invited.
 * @param sessionId ID of the session used by the acknowledging contact.
 */
XMPPClient.prototype.onSessionInvite = function(fromJid, fromSessionId, participants) {
	this.log.debug('client.js:onSessionInvite(' + this.log.anonymizeEMail(fromJid) + ', ' + fromSessionId + ', ...)');
	
	if (!this.acceptMessagesFrom(fromJid)) {
		return;
	}
	
	var sessionParticipants = [];
	var participants = participants.participant;
	if (!participants.length) {
		participants = [ participants ];
	}
	for (var i = 0; i < participants.length; i++) {
		var participant = participants[i];
		var jid = participant['@jid'];
		var sessionId = participant['@sessionId'];
		
		var bareJid = jidbare(jid);
		window.roster.ensureItem(bareJid);
		
		var item = window.roster.itemJSON(bareJid).parseJSON();
		if (!item.scrapSecret) {
			this.retrieveMemberInfo(bareJid, true);
		}
		
		this.setBlockedValueForJid(bareJid, false);
		
		item.jid = jid;
		item.sessionId = sessionId;
		sessionParticipants.push(item);
	}
	
	window.windowManager.raiseEvent(ObjectToJSONString({
		name: 'joinMucSession',
		inviterJid: fromJid,
		inviterSessionId: fromSessionId,
		participants: sessionParticipants
	}));
}

/**
 * Handles an incoming notification of a user that joined a session.
 *
 * @param fromJid The contact that is joining the session.
 * @param toSessionId The session ID of our session.
 * @param fromSessionId ID of the session used by the invitee.
 * @param forwardFrom If this variable is set, fromJid is the one that sent the
 *                    sessionjoined message, but actually is already in the
 *                    session. Instead, he forwarded the sessionjoined message
 *                    from this JID, who _is_ joining the session.
 */
XMPPClient.prototype.onSessionJoined = function(fromJid, toSessionId, fromSessionId, forwardFrom) {
	this.log.debug('client.js:onSessionJoined(' + this.log.anonymizeEMail(fromJid) + ', ' + toSessionId + ', ' + fromSessionId + ')');
	
	if (!this.acceptMessagesFrom(fromJid)) {
		return;
	}
	
	if (forwardFrom) {
		fromJid = forwardFrom;
	}
	
	if (!this.container.sessionToWindowMap.hasOwnProperty(toSessionId)) {
		this.log.debug('Received a sessionjoined message from ' + this.log.anonymizeEMail(fromJid) + ', but session is not registered.');
		return;
	}
	
	var bareJid = jidbare(fromJid);
	
	this.setBlockedValueForJid(bareJid, false);
	window.roster.ensureItem(bareJid);
	
	var item = window.roster.itemJSON(bareJid).parseJSON();
	if (!item.scrapSecret) {
		this.retrieveMemberInfo(bareJid, true);
	}
	
	window.windowManager.raiseEvent(ObjectToJSONString({
		name: 'sessionJoined',
		toSessionId: toSessionId,
		inviteeJid: fromJid,
		inviteeSessionId: fromSessionId
	}));
}

/**
 * Handles an incoming notification of a user that left a session.
 *
 * @param fromJid The contact that is leaving the session.
 * @param toSessionId The session ID of our session.
 * @param fromSessionId ID of the session used by the participant.
 */
XMPPClient.prototype.onSessionClosed = function(fromJid, toSessionId, fromSessionId) {
	this.log.debug('client.js:onSessionClosed(' + this.log.anonymizeEMail(fromJid) + ', ' + toSessionId + ', ' + fromSessionId + ')');
	
	if (!this.acceptMessagesFrom(fromJid)) {
		return;
	}
	
	if (!this.container.sessionToWindowMap.hasOwnProperty(toSessionId)) {
		this.log.debug('Received a sessionclosed message from ' + this.log.anonymizeEMail(fromJid) + ', but session is not registered.');
		return;
	}
	
	window.windowManager.raiseEvent(ObjectToJSONString({
		name: 'sessionClosed',
		toSessionId: toSessionId,
		participantJid: fromJid,
		participantSessionId: fromSessionId
	}));
}

/**
 * Delivers a message by emitting the appropriate event.
 *
 * @param fromJid Full JID of the contact that sent the message.
 * @param toSessionId ID of the session that is being addressed, may be null for
 *                    one-to-one chats.
 * @param message The message to deliver.
 */
XMPPClient.prototype.deliverMessage = function(fromJid, toSessionId, message) {
	this.log.debug('client.js:deliverMessage(' + this.log.anonymizeEMail(fromJid) + ', ' + toSessionId + ', ...)');
	
	window.windowManager.raiseEvent(ObjectToJSONString({
		name: 'messageReceived',
		jid: fromJid,
		toSessionId: toSessionId,
		message: message
	}));
}

XMPPClient.prototype.onIncomingXml = function(json) {
	
	json = json.parseJSON();
	
	if (json.iq) {
		this.handleIQ(json);
	} else if (json.message) {
		this.handleMessage(json);
	}
}

XMPPClient.prototype.handleIQ = function(json) {
	
	var id = json.iq["@id"];
	
	if (this.pendingIQs[id]) {
		this[this.pendingIQs[id]](json);
		delete this.pendingIQs[id];
		return;
	}
}

XMPPClient.prototype.handleRosterUpdate = function(json) {
	
	this.sendExtendedRosterRequest();
	this.sendInitialPresence();
}

XMPPClient.prototype.handleMessage = function(json) {
	
	if (json.message['@type'] == 'headline' && json.message.notification && json.message.notification['@from']) {
		this.onHeadline(json.message['@from'], json.message);
	} else if (json.message.invitestart && json.message.invitestart['@xmlns'] == XMPP_HYVES_NS) {
		this.onInviteStart(json.message['@from'], json.message.invitestart['@invitee'], json.message.invitestart['@toSessionId'], json.message.invitestart['@fromSessionId']);
	} else if (json.message.inviteack && json.message.inviteack['@xmlns'] == XMPP_HYVES_NS) {
		this.onInviteAck(json.message['@from'], json.message.inviteack['@invitee'], json.message.inviteack['@toSessionId'], json.message.inviteack['@fromSessionId']);
	} else if (json.message.sessioninvite && json.message.sessioninvite['@xmlns'] == XMPP_HYVES_NS) {
		this.onSessionInvite(json.message['@from'], json.message.sessioninvite['@fromSessionId'], json.message.sessioninvite.participants);
	} else if (json.message.sessionjoined && json.message.sessionjoined['@xmlns'] == XMPP_HYVES_NS) {
		this.onSessionJoined(json.message['@from'], json.message.sessionjoined['@toSessionId'], json.message.sessionjoined['@fromSessionId'], json.message.sessionjoined['@forwardFrom']);
	} else if (json.message.sessionclosed && json.message.sessionclosed['@xmlns'] == XMPP_HYVES_NS) {
		this.onSessionClosed(json.message['@from'], json.message.sessionclosed['@toSessionId'], json.message.sessionclosed['@fromSessionId']);
	} else if (json.message['@type'] == 'chat') {
		if (json.message.body) {
			this.onMessage(json.message['@from'], json.message['@toSessionId'], json.message.body);
		}
		if (json.message.active || json.message.composing || json.message.gone ||
		    json.message.inactive || json.message.none || json.message.paused) {
			this.onChatState(json.message['@from'], json.message['@toSessionId'], json.message.composing ? 'composing' : 'inactive');
		}
		if (json.message.request && json.message.request['@xmlns'] == 'urn:xmpp:receipts') {
			this.onReceiptRequest(json.message['@from'], json.message['@id']);
		}
	}
	
	if (json.message.received && json.message.received['@xmlns'] == 'urn:xmpp:receipts') {
		this.onReceiptReceived(json.message['@from'], json.message['@id']);
	}
}
