var INACTIVITY_TIMEOUT = 10 * 60; // 10 minutes (in seconds) before status is updated
var MEDIA_SIZE_ICON_SMALL = 1;
var MEDIA_SIZE_ICON_MEDIUM = 2;

templates.addFromString('chat.tmplDesktopPopup', '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">\n<html><head><title>__headline__</title><link rel=\"stylesheet" href="http://localhost/statics/style20.css" type="text/css"></link><script type="text/javascript" src="http://localhost/statics/json.js"></script><script type="text/javascript">function setMedia(el, id, secret) { el.src = window.mediaCache.mediaUrl(id + \'/\' + secret, el.width, el.height); }</script></head><body class="ChatPopUpBody" style="overflow: hidden"><div class="ChatPopUpHeader ChatPopUpHeaderLogo" style="text-align: right; vertical-align: top"><span onclick="window.windowManager.raiseEvent(ObjectToJSONString({ name: \'closeSlider\', windowId: \'#windowId#\' }));" class="fakelink" style="width: 30px; font-family: arial; font-size: 8pt; padding: 0px 2px 0px 4px\">&nbsp;&nbsp;&nbsp;&nbsp;</span></div>${content}</body></html>');
templates.addFromString('chat.tmplDesktopPopupContent', '<table cellpadding="0" cellspacing="0" class="ChatPopUpMain" style="border: 0px" onclick="${action}"><tr><td style="height: 90px; padding: 7px 5px 0px 5px; vertical-align: top"><img class="ChatPopUpImageBorder" style="width: 54px; height: 54px; border: 0px" src="http://localhost/images/nixel.gif" onload="setMedia(this, \'${mediaId}\', \'${mediaSecret}\')"></td><td class="ChatPopUpText BodyTextSmall" style="padding-top: 7px; vertical-align: top; overflow: hidden;">${message}</td></tr></table>');
templates.addFromString('chat.tmplDesktopWWWNotificationMsg', '${item.nick|default:""|fancyLayout:"nickname"}, www: ${item.wwwEmotion|default:""|fancyLayout:"oneliner"}@${item.wwwLocation|default:""|fancyLayout:"oneliner"}\n');

function XMPPDesktopContainer(languageExt) {
	this.log.debug('container.js:XMPPDesktopContainer(' + languageExt + ')');
	
	this.extender = window.extender;
	this.informer = window.informer;
	this.player = window.player;
	this.translator = window.translator;
	this.settingsManager = window.settingsManager;
	this.windowManager = window.windowManager;
	
	this.translator.setLanguage(languageExt);
	
	if (window.isHyvesCrewMember || window.downloadPreUpdate) {
		if (window.isHyvesCrewMember) {
			window.updater.setDownloadHqUpdate(true);
		}
		if (window.downloadPreUpdate) {
			window.updater.setDownloadPreUpdate(true);
		}
		window.updater.checkForUpdates();
	}
	
	for (var key in aShowStatus) {
		window.systemTrayIcon.addAction('Status', key, aShowStatus[key].status, 'status');
		window.roster.setStatusText(key, aShowStatus[key].status);
	}
	
	this.mediaLocator = new MediaLocator();
	
	// one-to-one session windows
	this.sessionWindows = {}; // jid => windowId
	this.sessionWindowInitialized = {}; // jid => true/false
	
	// MUC session windows
	this.sessionToWindowMap = {}; // sessionId => windowId
	this.windowToSessionMap = {}; // windowId => sessionId
	this.remoteSessionIds = {}; // jid => { sessionId => windowId }
	this.pendingMucSessionWindows = [];
	
	this.messageQueue = {}; // jid => array()
	
	this.historyWindowId = 0;
	this.historyJid = null;
	
	this.client = null; // will be set by client
	
	this.enableSliderNotifications = false; // we wait a bit before we show sliders after startup	
	
	this.extender.connect('informer', 'secondsIdle(int)', 'window.container.secondsIdle');
	this.extender.connect('systemTrayIcon', 'menuOptionClicked(QString)', 'window.container.onMenuOptionClicked');
	this.extender.connect('windowManager', 'eventRaised(QVariantMap)', 'window.container.onEventRaised');
}

XMPPDesktopContainer.prototype.log = logging.getLogger('XMPPDesktopContainer');

XMPPDesktopContainer.prototype.onEventRaised = function(event) {
	
	if (event.name == 'closeSlider') {
		this.windowManager.closeSlider(event.windowId);
	} else if (event.name == 'headlineReceived') {
		this.onHeadline(event);
	} else if (event.name == 'historyWindowAvailable') {
		this.onHistoryWindowAvailable();
	} else if (event.name == 'initiateSession') {
		this.initiateSession(event.jid, event.messageType);
	} else if (event.name == 'inviteAckReceived') {
		this.onInviteAckReceived(event);
	} else if (event.name == 'joinMucSession') {
		this.joinMucSession(event);
	} else if (event.name == 'mainMenuSettingsClicked') {
		this.showSettingsDialog();
	} else if (event.name == 'messageReceived') {
		this.onMessageReceived(event);
	} else if (event.name == 'openHistoryWindow') {
		this.openHistoryWindow(event.jid);
	} else if (event.name == 'presenceStatusChanged') {
		this.onPresenceStatusChanged(event.status);
	} else if (event.name == 'register1to1Session') {
		this.register1to1Session(event.jid, event.windowId);
	} else if (event.name == 'sessionClosed') {
		this.onSessionClosed(event);
	} else if (event.name == 'sessionJoined') {
		this.onSessionJoined(event);
	} else if (event.name == 'sessionWindowAvailable') {
		this.onSessionWindowAvailable();
	} else if (event.name == 'unregister1to1Session') {
		this.unregister1to1Session(event.jid);
	} else if (event.name == 'userLoggedOut') {
		window.mainWindow.reloadKwekker();
	} else if (event.name == 'windowClose' && event['class'] == 'session') {
		this.onSessionWindowClose(event.id);
	} else if (event.name == 'windowClose' && event['class'] == 'history') {
		this.onHistoryWindowClosed();
	} else if (event.name == 'windowPositionChanged' && event['class'] == 'session') {
		this.onSessionWindowPositionChanged(event);
	}
}

XMPPDesktopContainer.prototype.hasSession = function(jid) {
	
	return this.sessionWindows.hasOwnProperty(jid);
}

/**
 * Returns the window ID for a specific session.
 *
 * @param jid JID of the contact in the session.
 * @param fromSessionId ID the remote contact has associated with the session.
 */
XMPPDesktopContainer.prototype.windowIdForSession = function(jid, fromSessionId) {
	
	bareJid = jidbare(jid);
	
	var windowId = 0;
	if (this.remoteSessionIds.hasOwnProperty(jid) &&
	    this.remoteSessionIds[jid].hasOwnProperty(fromSessionId)) {
		windowId = this.remoteSessionIds[jid][fromSessionId];
	} else if (this.sessionWindows.hasOwnProperty(bareJid)) {
		windowId = this.sessionWindows[bareJid];
	}
	return windowId;
}

/**
 * Called when the user wants to initiate a new session.
 */
XMPPDesktopContainer.prototype.initiateSession = function(toJid, messageType) {
	this.log.debug('container.js:initiateSession(' + this.log.anonymizeEMail(toJid) + ')');
	
	this.ensureSession(toJid, true);
	
	if (typeof(messageType) != 'undefined') {
		setTimeout(function() {
			this.windowManager.raiseEvent(ObjectToJSONString({
				name: 'setMessageType',
				jid: toJid,
				type: messageType
			}));
		}, 700);
	}
}

XMPPDesktopContainer.prototype.ensureSession = function(jid, ownerInitiated) {
	
	if (window.roster.ensureItem(jid)) {
		this.client.retrieveMemberInfo(jid);
	}
	
	if (!this.hasSession(jid)) {
		var windowId = this.newSession(jid, ownerInitiated);
		
		this.sessionWindows[jid] = windowId;
		this.sessionWindowInitialized[jid] = false;
	} else {
		if (ownerInitiated) {
			this.focusSession(jid);
		}
	}
}

XMPPDesktopContainer.prototype.ensureMucSession = function(fromJid, fromSessionId) {
	
	var windowId = this.windowIdForSession(fromJid, fromSessionId);
	if (windowId) {
		var sessionId = this.windowToSessionMap[windowId];
	} else {
		var sessionId = this.uniqueSessionId();
		var windowId = this.joinMucSession({
			inviterJid: fromJid,
			inviterSessionId: fromSessionId,
			sessionId: sessionId,
			participants: []
		});
	}
	
	this.registerRemoteSessionId(fromJid, fromSessionId, windowId);
	
	return sessionId;
}

XMPPDesktopContainer.prototype.registerRemoteSessionId = function(jid, sessionId, windowId) {
	
	if (!this.remoteSessionIds.hasOwnProperty(jid)) {
		this.remoteSessionIds[jid] = {};
	}
	this.remoteSessionIds[jid][sessionId] = windowId;
}

/**
 * Creates a new session window.
 *
 * @param jid The JID of the contact for which to create the session window.
 * @param ownerInitiated Boolean indicating whether the current user initiated
 *                       the session.
 * @return The window ID of the newly created session window.
 */
XMPPDesktopContainer.prototype.newSession = function(jid, ownerInitiated) {
	this.log.debug('container.js:newSession(' + this.log.anonymizeEMail(jid) + ', ' + ownerInitiated + ')');
	
	var position = this.getNewSessionWindowPosition();
	
	// lookup item so that we can set caption of window to nick
	var item = window.roster.itemJSON(jid).parseJSON();
	var caption = jid;
	if (item && item.nick) {
		caption = fancy_layout(item.nick, 'none');
	}
	// create the session window
	var args = {
		visible: true,
		caption: caption,
		left: position.left,
		top: position.top,
		height: position.height,
		width: position.width,
		trackWindowPosition: true,
		minimumSize: true
	};
	
	if (!ownerInitiated) {
		args.windowState = 'minimized';
	}
	
	var windowId = this.windowManager.createWindow('session', ObjectToJSONString(args));
	
	this.log.debug('session window created for jid: ' + this.log.anonymizeEMail(jid));
	
	this.windowManager.windowExecute(windowId, 'navigate', 'url=http://localhost/statics/kwekker2/session.html');
	
	if (ownerInitiated) {
		this.windowManager.windowExecute(windowId, 'bringToFront');
	}
	
	// when somebody starts talking to us, we also want to show a slider with
	// the first msg to indicate that someone wants to talk to us
	if (!ownerInitiated) {
		this.showFirstMessageInSlider(jid);
	}
	
	return windowId;
}

XMPPDesktopContainer.prototype.getNewSessionWindowPosition = function() {
	
	var windowInfo = this.windowManager.windowInfoJSON(window.mainWindow.windowId()).parseJSON();
	
	var height = 570 + this.windowManager.titleBarHeight() + this.windowManager.frameHeight();
	var width = 520 + 2 * this.windowManager.frameWidth();
	
	// calculate left and top such that subsequence session windows are stacked
	// if the windows bottom is out of the screen we start on top again
	// otherwise we stack the window from the upper left corner to
	// the bottom right, like MSN
	var windowCount = $H(this.sessionWindows).keys().length;
	this.log.debug('window count: ' + windowCount);
	
	var windowOffset = 30;
	var layersOnStack = windowCount;
	if (layersOnStack * windowOffset + height > windowInfo.screenHeight) {
		var lastWindowTop = windowInfo.screenHeight - height;
		if (lastWindowTop < 0) {
			lastWindowTop = 0;
		}
		
		var layerCountPerStack = Math.floor(lastWindowTop / windowOffset) + 1;
		layersOnStack = windowCount % layerCountPerStack;
	}
	
	var top = Math.floor(((windowInfo.screenHeight / 2) - (height / 2)) + layersOnStack * windowOffset);
	var left = Math.floor(((windowInfo.screenWidth / 2) - (width / 2)) + layersOnStack * windowOffset);
	this.log.debug('top: ' + top + ', left: ' + left);
	
	return { width: width, height: height, top: top, left: left };
}

XMPPDesktopContainer.prototype.focusSession = function(jid) {
	
	if (!this.hasSession(jid)) {
		this.log.error('container.js:focusSession(): Trying to focus non-existing session.');
		return;
	}
	
	this.windowManager.windowExecute(this.sessionWindows[jid], 'bringToFront');
}

XMPPDesktopContainer.prototype.onSessionWindowClose = function(windowId) {
	this.log.debug('container.js:onSessionWindowClose()');
	
	var sessionId = this.windowToSessionMap[windowId];
	delete this.windowToSessionMap[windowId];
	delete this.sessionToWindowMap[sessionId];
	
	// unregister MUC sessions
	for (var jid in this.remoteSessionIds) {
		var sessionToWindowMap = this.remoteSessionIds[jid];
		for (var sessionId in sessionToWindowMap) {
			if (sessionToWindowMap[sessionId] == windowId) {
				delete this.remoteSessionIds[jid][sessionId];
			}
		}
	}
	
	// unregister one-to-one sessions
	for (jid in this.sessionWindows) {
		if (this.sessionWindows[jid] != windowId) {
			continue;
		}
		
		this.log.debug('unregistering session for ' + this.log.anonymizeEMail(jid));
		
		this.windowManager.raiseEvent(ObjectToJSONString({
			name: 'sendMessage',
			toJid: jid,
			message: '',
			extra: '<gone xmlns=\'http://jabber.org/protocol/chatstates\'/>'
		}));
		
		this.windowManager.raiseEvent(ObjectToJSONString({
			name: 'sessionUnregistered',
			jid: jid,
			windowId: windowId
		}));
		
		delete this.sessionWindows[jid];
		delete this.sessionWindowInitialized[jid];
		this.log.debug('remaining number of session windows: ' + $H(this.sessionWindows).keys().length);
	}
}

XMPPDesktopContainer.prototype.onSessionWindowAvailable = function() {
	this.log.debug('container.js:onSessionWindowAvailable()');
	
	// first try to find an uninitialized one-to-one session
	for (jid in this.sessionWindowInitialized) {
		if (this.sessionWindowInitialized[jid]) {
			continue;
		}
		
		this.sessionWindowInitialized[jid] = true;
		
		var windowId = this.sessionWindows[jid];
		var sessionId = this.uniqueSessionId();
		this.sessionToWindowMap[sessionId] = windowId;
		this.windowToSessionMap[windowId] = sessionId;
		
		this.windowManager.raiseEvent(ObjectToJSONString({
			name: 'sessionRegistered',
			participants: [ window.roster.itemJSON(jid).parseJSON() ],
			windowId: windowId,
			sessionId: sessionId,
			globalVars: globalVars()
		}));
		
		this.deliverQueuedMessages(jid);
		
		return;
	}
	
	// try to find an uninitialized multi-user chat session
	var mucSessionWindow = this.pendingMucSessionWindows.pop();
	if (!mucSessionWindow) {
		this.log.debug('no session window expected!');
		return;
	}
	
	var windowId = mucSessionWindow.windowId;
	var sessionId = (mucSessionWindow.sessionId ? mucSessionWindow.sessionId : this.uniqueSessionId());
	this.sessionToWindowMap[sessionId] = windowId;
	this.windowToSessionMap[windowId] = sessionId;
	
	this.windowManager.raiseEvent(ObjectToJSONString({
		name: 'sessionRegistered',
		participants: mucSessionWindow.participants,
		windowId: windowId,
		sessionId: sessionId,
		globalVars: globalVars()
	}));
}

XMPPDesktopContainer.prototype.uniqueSessionId = function() {
	
	do {
		var sessionId = Math.floor(Math.random() * 1000000) + 1;
	} while(this.sessionToWindowMap.hasOwnProperty(sessionId));
	
	return sessionId;
}

XMPPDesktopContainer.prototype.deliverQueuedMessages = function(jid) {
	this.log.debug('container.js:deliverQueuedMessages()');
	
	var queuedMessages = this.queuedMessages(jid, { clearQueue: true });
	if (!queuedMessages) {
		this.log.debug('no messages to deliver');
		return;
	}
	
	for (var i = 0; i < queuedMessages.messages.length; i++) {
		var message = queuedMessages.messages[i];
		this.windowManager.raiseEvent(ObjectToJSONString({
			name: 'messageReceived',
			jid: queuedMessages.jid,
			message: message
		}));
	}
}

/**
 * Returns messages sent by a contact and queued for display in a session
 * window.
 *
 * @param fromJid JID of the contact that sent the messages. Can be either a
 *                bare JID or a full JID.
 * @param options Associative array of options. If 'clearQueue' is true, the
 *                queue for the contact is cleared after the messages are
 *                retrieved.
 * @return Object containing two properties: 'jid' and 'messages'. 'jid' is the
 *         full JID of the sender, 'messages' is an array of messages (strings).
 *         Returns null if there is no queue for the specified contact.  
 */
XMPPDesktopContainer.prototype.queuedMessages = function(fromJid, options) {
	
	var bareJid = jidbare(fromJid);
	var messages = null;
	var fullJid = null;
	for (var jid in this.messageQueue) {
		if (bareJid == jidbare(jid)) {
			messages = this.messageQueue[jid];
			fullJid = jid;
			if (options && options.clearQueue) {
				delete this.messageQueue[jid];
			}
			break;
		}
	}
	return (messages ? { jid: fullJid, messages: messages } : null);
}

XMPPDesktopContainer.prototype.closeOpenSessions = function() {
	
	try {
		for (jid in this.sessionWindows) {
			var windowId = this.sessionWindows[jid];
			this.log.debug('closing for jid: ' + this.log.anonymizeEMail(jid));
			this.onSessionWindowClose(windowId);
			this.windowManager.closeWindow(windowId);
		}
	} catch(e) {
		this.log.error('container.js:closeOpenSessions(): ' + e, e);
	}
}

XMPPDesktopContainer.prototype.onMessageReceived = function(event) {
	
	bareJid = jidbare(event.jid);
	
	if (event.toSessionId) {
		// the message is addressed at a multi-user chat, which will
		// show the message (if the session is really still open)
	} else if (!this.sessionWindows[bareJid] || !this.sessionWindowInitialized[bareJid]) {
		// the session window is not yet open or initialized, so queue the message
		if (!this.messageQueue[event.jid]) {
			this.messageQueue[event.jid] = [];
		}
		
		this.messageQueue[event.jid].push(event.message);
		
		// now make sure that we have a session eventually
		this.ensureSession(bareJid, false);
	} else {
		// the session window will show the message
	}
}

/**
 * Checks for user activity.
 */
XMPPDesktopContainer.prototype.secondsIdle = function(idleSeconds) {
	
	if (idleSeconds > INACTIVITY_TIMEOUT) {
		//this.log.debug('Idle for ' + idleSeconds + ' seconds, now inactive.'); 
		this.client.onDesktopStateInactive();
	} else {
		this.client.onDesktopStateActive();
	}
}

/**
 * Called when user succesfully signs in on the chat server
 */
XMPPDesktopContainer.prototype.onClientAuthorized = function() {
	
	try {
		window.systemTrayIcon.setIcon('active');
		
		// and enable presence notifications some time after this
		var self = this;
		setTimeout(function() {
			if (!window.roster.isInitialized()) {
				window.jabber.setErrorCode(504);
			}
			
			self.enableSliderNotifications = true;
			self.log.debug('slider notifications now enabled');
		}, PERIOD_WITHOUT_SLIDERS);
		
		// also write this last login time in our settings
		var lastLogin = Math.floor(((new Date()).getTime()) / 1000.0);
		this.settingsManager.setValue('main.lastLogin', lastLogin);
	} catch(e) {
		this.log.error('' + e, e);
	}
}

XMPPDesktopContainer.prototype.onDisconnect = function() {
	
	window.systemTrayIcon.setIcon('inactive');
	this.closeOpenSessions();
}

XMPPDesktopContainer.prototype.setClient = function(client) {
	this.log.debug('container.js:setClient()');
	
	this.client = client;
}

/**
 * called when a session window changes position or size
 * we will save its position for later use
 */
XMPPDesktopContainer.prototype.onSessionWindowPositionChanged = function(event) {
	
	var self = this;
	onidle('storeSessionWindowPos', 2000, function() {
		try {
			self.log.debug('storing sessing window pos!');
			self.settingsManager.setValue('session.x', event.x);
			self.settingsManager.setValue('session.y', event.y);
			self.settingsManager.setValue('session.cx', event.cx);
			self.settingsManager.setValue('session.cy', event.cy);
		} catch(e) {
			self.log.error('container.js:onSessionWindowPositionChanged(): ' + e, e);
		}
	});
}

XMPPDesktopContainer.prototype.showFirstMessageInSlider = function(toJid) {
	
	var queuedMessages = this.queuedMessages(toJid);
	if (queuedMessages && queuedMessages.messages.length > 0) {
		// show a headline with the first message...
		// and some info on the person talking to me
		this.log.debug('yes there are messages, so i will show slider...');
		var self = this;
		setTimeout(function() {
			self.log.debug('process new session popup template and showing slider');
			var item = window.roster.itemJSON(toJid).parseJSON();
			var body = fancy_layout(filterMessageForDisplay(queuedMessages.messages[0]), 'body');
			var message = tr("%1 says:").arg(fancy_layout(item.nick, 'nickname,nosmilies')) + '<br>' + body;
			var content = templates.process('chat.tmplDesktopPopupContent', {
				action: 'window.windowManager.raiseEvent(ObjectToJSONString({ name: \'initiateSession\', jid: \'' + item.jid + '\' }))',
				mediaId: item.mediaId,
				mediaSecret: item.mediaSecret,
				message: message
			});
			var title = tr("New message from %1").arg(fancy_layout(item.nick, 'none'));
			var method = "windowManager.raiseEvent";
			var parameter = ObjectToJSONString({ name: 'initiateSession', jid:  item.jid });
			self.showSlider(content, body, title, method, parameter, 'newSession', item.mediaId, item.mediaSecret, true);
		}, 500);
	}
}

XMPPDesktopContainer.prototype.onMenuOptionClicked = function(menuOption) {
	this.log.debug('container.js:onMenuOptionClicked(' + menuOption + ')');
	
	try {
		menu = menuOption.split("/")[0];
		item = menuOption.split("/")[1];
		if (menu == "Status") {
			this.client.setPresenceStatus(item);
		}
	} catch(e) {
		this.log.error('container.js:onMenuOptionClicked(): ' + e, e);
	}
}

XMPPDesktopContainer.prototype.onHeadline = function(event) {
	
	try {
		var message = fancy_layout(event.subject, 'body');
		var memberInfo = window.roster.itemJSON(event.fromJid).parseJSON();
		var mediaId =  memberInfo ? memberInfo.mediaId : null;
		var mediaSecret =  memberInfo ? memberInfo.mediaSecret : null;
		var data = {
			action: 'window.windowManager.openUrl(\'' + event.url + '\')',
			mediaId: mediaId,
			mediaSecret: mediaSecret,
			message: message
		};
		var title = tr("New notification");
		var html = templates.process('chat.tmplDesktopPopupContent', data);
		var method = 'windowManager.openUrl';
		var parameter = event.url;
		this.showSlider(html, message, title, method, parameter,'headline', mediaId, mediaSecret);
	} catch(e) {
		this.log.error('container.js:onHeadline(): ' + e, e);
	}
}

  XMPPDesktopContainer.prototype.showSlider = function(content, message, title, method, parameter, slideName, mediaId, mediaSecret, forceShow) {
	
	try {
		if (!this.enableSliderNotifications && !forceShow) {
			this.log.debug('not showing slider, still disabled: ' + slideName);
			return;
		}
		
		// play sound if requested
		if (slideName != 'newSession') { // the session window will play the sound itself
			var defaultValue = (slideName == 'newOnline' ? false : true);
			if (this.settingsManager.boolValue('Notifications/' + slideName + 'Sound', defaultValue)) {
				this.log.debug('sound enabled for slider ' + slideName + ', now playing');
				this.player.playSound(':/plugins/kwekker/sounds/' + slideName);
			}
		}
		
		// show the slider
		if (this.settingsManager.boolValue('Notifications/' + slideName + "Alert", true)) {
			this.log.debug('alert enabled for ' + slideName + ', now showing slider');
			var html = templates.process('chat.tmplDesktopPopup', { content: content });
			this.windowManager.showSlider(html, message, title, method, parameter, mediaId, mediaSecret);
		}
	} catch(e) {
		this.log.debug('container.js:showSlider(): ' + e, e);
	}
}

XMPPDesktopContainer.prototype.openHistoryWindow = function(jid) {
	
	this.historyJid = jidbare(jid);
	
	if (!this.historyWindowId) {
		var windowInfo = this.windowManager.windowInfoJSON(window.mainWindow.windowId()).parseJSON();
		if (!windowInfo || windowInfo.id == 0) {
			this.log.error('no info about main window!');
			return;
		}
		
		var height = 570;
		var width = 520;
		var top = Math.floor((windowInfo.screenHeight / 2) - (height / 2));
		var left = Math.floor((windowInfo.screenWidth / 2) - (width / 2));
		
		var args = {
			visible: true,
			caption: tr("History"),
			left: left,
			top: top,
			height: height,
			width: width,
			minimumSize: true
		};
		this.historyWindowId = this.windowManager.createWindow('history', ObjectToJSONString(args));
		
		this.windowManager.windowExecute(this.historyWindowId, 'navigate',
		                                 'url=http://localhost/statics/kwekker2/history.html');
	} else {
		this.onHistoryWindowAvailable();
	}
}

XMPPDesktopContainer.prototype.onHistoryWindowAvailable = function() {
	
	window.windowManager.raiseEvent(ObjectToJSONString({
		name: 'openHistory',
		jid: this.historyJid,
		fromMemberInfo: window.roster.ownItemJSON().parseJSON(),
		toMemberInfo: window.roster.itemJSON(this.historyJid).parseJSON(),
		globalVars: globalVars(),
		windowId: this.historyWindowId
	}));
}

XMPPDesktopContainer.prototype.onPresenceStatusChanged = function(status) {
	this.log.debug('container.js:onPresenceStatusChanged(' + status + ')');
	
	setInitialPresenceCookie(status);
	
	window.systemTrayIcon.setSelectedAction('Status', status);
}

XMPPDesktopContainer.prototype.onHistoryWindowClosed = function() {
	
	this.historyWindowId = 0;
}

/**
 * Called when an acknowledgment for an invitation is received.
 */
XMPPDesktopContainer.prototype.onInviteAckReceived = function(event) {
	
	if (!this.sessionToWindowMap.hasOwnProperty(event.toSessionId)) {
		this.log.debug('Received an inviteack message from ' + this.log.anonymizeEMail(event.fromJid) + ' for a non-existing session.');
		return;
	}
	
	var windowId = this.sessionToWindowMap[event.toSessionId];
	
	this.registerRemoteSessionId(event.fromJid, event.fromSessionId, windowId);
}

/**
 * Called when a participant enters a multi-user chat.
 */
XMPPDesktopContainer.prototype.onSessionJoined = function(event) {
	
	this.registerRemoteSessionId(event.inviteeJid, event.inviteeSessionId, this.sessionToWindowMap[event.toSessionId]);
}

/**
 * Called when a participant from a multi-user chat has left the session.
 */
XMPPDesktopContainer.prototype.onSessionClosed = function(event) {
	
	if (this.remoteSessionIds.hasOwnProperty(event.participantJid)) {
		delete this.remoteSessionIds[event.participantJid][event.participantSessionId];
	}
}

/**
 * Called when we are going to join a multi-user session.
 */
XMPPDesktopContainer.prototype.joinMucSession = function(event) {
	
	var windowId = this.newSession(event.inviterJid, false);
	
	var participants = event.participants;
	
	var item = window.roster.itemJSON(jidbare(event.inviterJid)).parseJSON();
	item.jid = event.inviterJid;
	item.sessionId = event.inviterSessionId;
	participants.push(item);
	
	for (var i = 0; i < participants.length; i++) {
		var participant = participants[i];
		this.registerRemoteSessionId(participant.jid, participant.sessionId, windowId);
	}
	
	this.pendingMucSessionWindows.push({
		windowId: windowId,
		participants: participants,
		sessionId: event.sessionId
	});
	
	return windowId;
}

/**
 * Registers a window for one-to-one session (after it was previously a MUC
 * session).
 */
XMPPDesktopContainer.prototype.register1to1Session = function(jid, windowId) {
	this.log.debug('container.js:register1to1Session(' + this.log.anonymizeEMail(jid) + ', ' + windowId + ')');
	
	this.sessionWindows[jid] = windowId;
	this.sessionWindowInitialized[jid] = true;
}

/**
 * Unregisters a window as a one-to-one session (because it became a MUC
 * session).
 */
XMPPDesktopContainer.prototype.unregister1to1Session = function(jid) {
	this.log.debug('container.js:unregister1to1Session(' + this.log.anonymizeEMail(jid) + ')');
	
	delete this.sessionWindows[jid];
	delete this.sessionWindowInitialized[jid];
}
