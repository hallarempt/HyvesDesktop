
function InvitationManager() {
	this.log.debug('invite.js:InvitationManager()');
	
	this.windowId = 0;
	this.sessionWindowId = 0;
	
	this.participants = [];
	
	this.initLanguage();
	
	var self = this;
	
	this.rosterUi = new RosterUI('window.invitationManager.rosterUi');
	this.rosterUi.setShowWWWs(false);
	this.rosterUi.setShowToolTips(false);
	this.rosterUi.onItemSelected = function(jid) { self.inviteContact(jid); };
	this.rosterUi.filterText = function() { return $('searchInput').value; };
	this.rosterUi.showOfflineContacts = function() { return false; };
	
	window.extender.connect('windowManager', 'eventRaised(QVariantMap)', 'window.invitationManager.onEventRaised');
	
	Event.observe(document.body, 'keydown', function(event) { self.rosterUi.onKeyPressed(event); });
	Event.observe('searchInput', 'keydown', function(event) {
		if (!isEnterKey(event.keyCode) &&
		    event.keyCode != 38 && event.keyCode != 40) {
			// only act on keys not used by the roster navigation
			onidle('searchInputIdle', 200, function() { self.rosterUi.refreshRoster(); });
		}
	});
	
	setTimeout(function() {
		window.windowManager.raiseEvent(ObjectToJSONString({
			name: 'inviteWindowAvailable'
		}));
	}, 10);
}

InvitationManager.prototype.log = logging.getLogger('InvitationManager');

InvitationManager.prototype.initLanguage = function() {
	
	try {
		$('inviteWindowHeader').innerHTML = tr('Invite Contact to Conversation');
		$('searchInput').setAttribute('placeholder', tr('Search for friends'));
	} catch(e) {
		logging.error('invite.js:initLanguage(): ' + e, e);
	}
}

InvitationManager.prototype.onEventRaised = function(event) {
	
	if (event.name == 'initInviteWindow') {
		this.initInviteWindow(event);
	}
}

InvitationManager.prototype.initInviteWindow = function(event) {
	this.log.debug('invite.js:initInviteWindow()');
	
	this.windowId = event.windowId;
	this.sessionWindowId = event.sessionWindowId;
	
	this.initGlobalVars(event.globalVars);
	
	var excludedJids = [ window.roster.ownBareJid() ];
	for (var jid in event.participants) {
		excludedJids.push(jid);
	}
	
	this.rosterUi.setExcludedJids(excludedJids);
	this.rosterUi.initialize();
	
	$('searchInput').focus();
}

InvitationManager.prototype.initGlobalVars = function(globalVars) {
	
	try {
		for (i in globalVars) {
			window[i] = globalVars[i];
		}
	} catch(e) {
		this.log.error('invite.js:initGlobalVars(): ' + e, e);
	}
}

/**
 * Selects a contact by JID to invite.
 */
InvitationManager.prototype.inviteContact = function(jid) {
	
	window.windowManager.raiseEvent(ObjectToJSONString({
		name: 'inviteContact',
		sessionWindowId: this.sessionWindowId,
		jid: jid
	}));
}

InvitationManager.prototype.updateGeometry = function() {
	
	$('contactsContainer').setStyle('height: ' +
		(document.body.clientHeight - $('inviteWindowHeader').clientHeight - $('menu').clientHeight) + 'px'
	);
}

function initInvitationManager() {
	
	window.invitationManager = new InvitationManager();
}

/**
 * Callback for when the window is resized (called from WebView.cpp).
 */
function windowResized(width, height) {
	
	if (window.invitationManager) {
		window.invitationManager.updateGeometry();
	}
}
