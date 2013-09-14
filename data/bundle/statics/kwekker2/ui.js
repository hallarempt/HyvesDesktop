var SCHEDULE_ROSTER_UPDATE_TIMEOUT = 1500;

templates.addFromString('chat.userInfo', 
	'<div id="usr-optns">' +
		'<div class="avatar small">' +
			'<a href="http://${item.userhome}">' +
				'<img src="http://localhost/images/nixel.gif" alt="" onclick="" onload="window.mediaLocator.setMedia(this, \'${item.mediaId}\', \'${item.mediaSecret}\');" class="square-img">' +
			'</a>' +
		'</div>' +
		'<ul>' +
			'<li id="usr-optns-status" class="usr-optns-toggle">' +
				'<a class="status ${item.status}">${item.statusText}</a>' + 
				'<div id="usr-status">' +
					'<ul>' +
					'{for aStatus in aShowStatus}' +
						'<li class="status ${aStatus_index}"><a onclick="setTimeout(function() { window.xmppClient.setPresenceStatus(\'${aStatus_index}\'); }, 0)">${aStatus.status|fancyLayout:"oneliner"}</a></li>' +
					'{/for}' +
					'</ul>' +
				'</div>' +
			'</li>' +
		'</ul>' +
		'<br /><br />' +
		'<div id="userWwwLabel" class="usr-www"">' +
			'<div class="dropdown">' +
				'<div class="dropdown-content"><strong>www: </strong>{if item.wwwEmotion}${item.wwwEmotion|default:""|fancyLayout:"oneliner"}{/if}{if item.wwwLocation}@${item.wwwLocation|default:""|fancyLayout:"oneliner"}{/if}</div>' +
				'<div class="dropdown-footer"><a href="javascript:showInputWWW();" id="addWwwLink"><img src="http://localhost/images/icons/default/icons16/add.png" alt="" style="border: 0px; vertical-align: middle; margin-top: -3px;" /> ${%tr(\'Add www\')%}</a></div>' +
			'</div>' +
			'<strong>www: </strong>{if item.wwwEmotion}${item.wwwEmotion|default:""|fancyLayout:"oneliner"}{/if}{if item.wwwLocation}@${item.wwwLocation|default:""|fancyLayout:"oneliner"}{/if}' +
		'</div>' +
	'</div>'
);
templates.addFromString('chat.searchbox',
	'<input id="searchbox_input" type="search" placeholder="${%tr(\'Search for friends\')%}" /><a id="searchbox_close" style="position: absolute; top:56px; right:12px; visibility:hidden;" href="javascript:clearSearchbar()" title="${%tr(\'Clear\')%}"><img src="http://localhost/images/buttons/searchbar-close.png" alt="X"></a>'
);

function moveFocusToSearchbar() {

	$('searchbox_input').focus();
}

function clearSearchbar() {

	var searchbox = $('searchbox_input');
	searchbox.value = '';
	$('searchbox_close').style.visibility = 'hidden';
	window.xmppClient.ui.rosterUi.refreshRoster();
	window.xmppClient.ui.rosterUi.selectFirstItem();
	searchbox.focus();
}

function XMPPClientUI() {
	
	this.errorMessageId = 0;
	
	this.wwwWindowId = 0;
	
	var userInfoBox = $('userinfobox');
	userInfoBox.addClassName('header');
	userInfoBox.innerHTML = '<div style="height: 53px; background: url(http://localhost/images/backgrounds/page-hdr-full.png) repeat-x"></div>'
	
	var searchBox = $('searchbox');
	searchBox.addClassName('menu-bg');
	searchBox.innerHTML = templates.process('chat.searchbox');
	
	this.rosterUi = new RosterUI('window.xmppClient.ui.rosterUi');
	this.rosterUi.onItemSelected = function(jid) { window.container.initiateSession(jid); };
	this.rosterUi.filterText = function() { return $('searchbox_input').value; };
	
	window.extender.connect('jabber', 'stateChanged(int)', 'window.xmppClient.ui.updateState');
	window.extender.connect('roster', 'initialized()', 'window.xmppClient.ui.onRosterInitialized');
	window.extender.connect('roster', 'itemChanged(QVariantMap, QVariantMap)', 'window.xmppClient.ui.onItemChanged');
	window.extender.connect('windowManager', 'eventRaised(QVariantMap)', 'window.xmppClient.ui.onEventRaised');
	
	var self = this;
	Event.observe(document.body, 'keydown', function(event) { self.rosterUi.onKeyPressed(event); });
	Event.observe('searchbox_input', 'keydown', function(event) {
		if (!isEnterKey(event.keyCode) &&
			event.keyCode != 38 && event.keyCode != 40) {
			// only act on keys not used by the roster navigation
			onidle('searchBoxInputIdle', 200, function() {
				self.rosterUi.refreshRoster();
				self.rosterUi.selectFirstItem();
			});
		}
	});
	Event.observe('searchbox_input', 'keyup', function(event) {
		$('searchbox_close').style.visibility = ($('searchbox_input').value != '') ? 'visible' : 'hidden';
	});
}

XMPPClientUI.prototype.log = logging.getLogger('XMPPClientUI');

XMPPClientUI.prototype.onEventRaised = function(event) {
	
	if (event.name == 'windowClose') {
		this.onWindowClosed(event.id);
	} else if (event.name == 'submitWww') {
		this.onSubmitWww(event);
	} else if (event.name == 'wwwDialogAvailable') {
		this.onWwwDialogAvailable();
	}
}

XMPPClientUI.prototype.onRosterInitialized = function() {
	
	this.updateUserInfo(window.roster.ownItemJSON().parseJSON());
	
	this.rosterUi.setExcludedJids([ window.roster.ownBareJid() ]);
	this.rosterUi.initialize();
}

XMPPClientUI.prototype.onItemChanged = function(item, changedProperties) {
		
	if (item.jid == window.roster.ownBareJid()) {
		if (changedProperties.hasOwnProperty('available') || changedProperties.hasOwnProperty('status')) {
			this.log.debug('presenceStatusChanged');
			window.windowManager.raiseEvent(ObjectToJSONString({
				name: 'presenceStatusChanged',
				status: item.status
			}));
		}
		
		this.updateUserInfo(item);
	} else {
		// listen for www changes so desktop can popup notification
		if (changedProperties.hasOwnProperty('wwwLocation') || changedProperties.hasOwnProperty('wwwEmotion')) {
			this.onWWWChanged(item);
		}
		
		// listen for presence changed so desktop can popup notification
		if (changedProperties.hasOwnProperty('available') || changedProperties.hasOwnProperty('status')) {
			this.onPresenceChanged(item, changedProperties);
		}
	}
}

XMPPClientUI.prototype.onWWWChanged = function(item) {
	
	var message = templates.process('chat.tmplDesktopWWWNotificationMsg', {
		item: item
	});
	var html = templates.process('chat.tmplDesktopPopupContent', {
		action: 'window.windowManager.openUrl(\'' + FRIEND_HOME_URL + '\')',
		mediaId: item.mediaId,
		mediaSecret: item.mediaSecret,
		message: message
	});
	var title = tr("New WWW from %1").arg(fancy_layout(item.nick, 'none'));
	var method = 'windowManager.openUrl';
	var parameter = FRIEND_HOME_URL;
	window.container.showSlider(html, message, title, method, parameter, 'headline', item.mediaId, item.mediaSecret);
}

XMPPClientUI.prototype.onPresenceChanged = function(item, changedProperties) {
	
	if (changedProperties.hasOwnProperty('available') && item.available) {
		var message = tr("%1 is now online").arg(fancy_layout(item.nick, 'nickname,nosmilies'));
		if (item.availableClients == ClientResourceBits.siteClient) {
			message += tr(" (Site)");
		}
		var content = templates.process('chat.tmplDesktopPopupContent', {
			action: 'window.windowManager.raiseEvent(ObjectToJSONString({ name: \'initiateSession\', jid: \'' + item.jid + '\' }))',
			mediaId: item.mediaId,
			mediaSecret: item.mediaSecret,
			message: message
		});
		var title = tr("%1 is now online").arg(fancy_layout(item.nick, 'none'));
		var method = "windowManager.raiseEvent";
		var parameter = ObjectToJSONString({ name: 'initiateSession', jid:  item.jid });
		
		window.container.showSlider(content,"", title, method, parameter, 'newOnline', item.mediaId, item.mediaSecret);
	}
}

XMPPClientUI.prototype.alert = function(message) {
	
	this.errorMessageId++;
	templates.insertBottom('errorbox', 'chat.tmplErrorMsg', {
		msg: message,
		msgId: this.errorMessageId
	});
}

/**
 * Declaratively define which parts of the ui are visible in the various
 * application states.
 */
var BOXES_ALL = [ 'wwwbox', 'progressbox', 'inactivebox', 'searchbox', 'rosterbox', 'disconnectedbox' ];

var BOXES_STATE = [];
BOXES_STATE[ClientState.Disconnected] = ['disconnectedbox'];
BOXES_STATE[ClientState.Connecting] = ['progressbox'];
BOXES_STATE[ClientState.Authenticating] = ['progressbox'];
BOXES_STATE[ClientState.Active] = ['wwwbox', 'searchbox', 'rosterbox'];

/**
 * Updates the visibility of the UI parts when the application state changes.
 */ 
XMPPClientUI.prototype.updateState = function(state) {
	
	BOXES_ALL.each(function(box, i) {
		var enabledInState = false;
		BOXES_STATE[state].each(function(stateBox, i) {
			if (box == stateBox) {
				enabledInState = true;
			}
		});
		if (enabledInState) {
			Element.show(box);
		} else {
			Element.hide(box);
		}
	});
}

/**
 * Updates the info box containing the own user info (www, icon, etc).
 */
XMPPClientUI.prototype.updateUserInfo = function(item) {
	this.log.debug('ui.js:updateUserInfo()');
	
	templates.updateElement('userinfobox', 'chat.userInfo', {
		item: item
	});
	windowResized();
}

XMPPClientUI.prototype.showInputWWW = function() {
	
	if (this.wwwWindowId > 0) {
		window.windowManager.windowExecute(this.wwwWindowId, 'bringToFront');
		return;
	}
	
	var windowInfo = window.windowManager.windowInfoJSON(window.mainWindow.windowId()).parseJSON();
	var width = 720;
	var height = 340;
	var left = Math.floor((windowInfo.screenWidth / 2) - (width / 2));
	var top = Math.floor((windowInfo.screenHeight / 2) - (height / 2));
	
	var windowId = window.windowManager.createWindow('www', ObjectToJSONString({
		visible: true,
		left: left,
		top: top,
		height: height,
		width: width,
		caption: 'WWW',
		minimumSize: true
	}));
	window.windowManager.windowExecute(windowId, 'navigate', 'url=http://localhost/statics/kwekker2/wwwDialog.html');
	
	this.wwwWindowId = windowId;
}

XMPPClientUI.prototype.onWwwDialogAvailable = function() {
	
	window.windowManager.raiseEvent(ObjectToJSONString({
		name: 'initWwwDialog',
		globalVars: globalVars(),
		windowId: this.wwwWindowId
	}));
}

XMPPClientUI.prototype.onSubmitWww = function(event) {
	
	if (typeof(event.visibility) == 'undefined') {
		event.visibility = 2; // VISIBILITY_FRIEND
	}
	
	// just post to the site like normal www update would
	var self = this;
	var pars = 'id=0&xmlHttp=1&sce=' + WWW_SECRET + '&postman=createedit&postman_secret=' + WWW_POSTMAN_SECRET +
		   '&formname=LocationHistory&formVarPrefix=locationhistory&locationhistory_emotion=' + escape(event.emotion) +
		   '&locationhistory_where=' + escape(event.where) + '&locationhistory_whereother=' + escape(event.whereOther) +
		   '&locationhistory_visibility=' + event.visibility + (event.addToHub ? '&locationhistory_addToHub=1' : '') +
		   (event.visibility == 10 ? '&friendGroupAcl[]={"subjectType": 61, "subjectId": "' + event.friendGroup + '", "matchType": 1}' : '');
	new Ajax.Request('/index.php', {
		method: 'post',
		postBody: pars,
		onException: function(request, exception) { self.log.error('Failed to submit WWW:' + exception, exception); }
	});
	
	window.windowManager.closeWindow(this.wwwWindowId);
	this.wwwWindowId = 0;
}

XMPPClientUI.prototype.onWindowClosed = function(windowId) {
	
	if (this.wwwWindowId == windowId) {
		this.wwwWindowId = 0;
	}
}

/**
 * Some function used in the main display.
 */
function showInputWWW() {
	
	window.xmppClient.ui.showInputWWW();
}
