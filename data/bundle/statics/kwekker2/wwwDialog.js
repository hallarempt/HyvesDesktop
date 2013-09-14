
templates.addFromString('locationhistory.popupContainer', '<div id="www_pulldown" class="autocomplete" style="display:none; position:absolute; padding: 5px; border: 1px solid #cfdfef; background-color: #ebf2fa;"></div>');
templates.addFromString('locationhistory.menuItems',
	'<div style="font-weight: bold">${%tr("My spots")%}</div>' +
	'{for favoriteLocation in favoriteLocations}' +
		'<div id="${favoriteLocation.id}" style="cursor: pointer;"><span style="float: right; font-size: 8pt; color: gray;">${%tr("Private")%}</span>${favoriteLocation.name|escape:"javascript"}</div>' +
	'{/for}' +
	'{for hangout in hangouts}' +
		'<div id="${hangout.id}" style="cursor: pointer;">${hangout.name|escape:"javascript"}</div>' +
	'{/for}' +
	'<div style="font-weight: bold">${%tr("Other networks with a location")%}</div>' +
	'{for otherHub in otherHubs}' +
		'<div id="${otherHub.id}" style="cursor: pointer;">${otherHub.name|escape:"javascript"}</div>' +
	'{/for}' +
	'<div style="border-top: 1px solid #cfdfef; padding: 3px;">' +
		'<a target="blank" href="http://www.${base_url()}/profielbeheer/profielinfo/spotedit/">${%tr("Add spot")%}</a>' +
	'</div>'
);
templates.addFromString('wwwDialog.currentWww', '{if item.wwwEmotion}${item.wwwEmotion|default:""|fancyLayout:"oneliner"|truncate:50}{/if} @ {if item.wwwLocation} ${item.wwwLocation|default:""|fancyLayout:"oneliner"|truncate:15}{/if}');

function WwwDialog() {
	
	this.log.usage('WwwDialogOpened');
	
	Element.setStyle(document.body, 'height: auto');
	
	this.windowId = 0;
	
	window.extender.connect('windowManager', 'eventRaised(QVariantMap)', 'window.wwwDialog.onEventRaised');
	
	// the handling of this signal should only start when the WwwDialog object is fully instantiated
	setTimeout(function() {
		window.windowManager.raiseEvent(ObjectToJSONString({
			name: 'wwwDialogAvailable'
		}));
	}, 10);
}

WwwDialog.prototype.log = logging.getLogger('WwwDialog');

WwwDialog.prototype.onEventRaised = function(event) {
	
	if (event.name == 'initWwwDialog' && this.windowId == 0) {
		this.init(event);
	}
}

WwwDialog.prototype.init = function(event) {
	
	this.windowId = event.windowId;
	
	this.initGlobalVars(event.globalVars);
	this.initLanguage();
	
	$('visibilityInput').value = window.settingsManager.intValue('WWW/lastVisibility', 2);
	
	var self = this;
	requires(['module:RichTextModule'], function() {
		var rteInstance = new RichTextEditor({
			name: '',
			open_picker: 'none',
			showpreview: false,
			textarea_height: '65px'
		});
		rteInstance.write('rteContainer');
		rteInstance.init();
		
		self.rte = rteInstance.domCode;
	});
	
	this.initHeader();
	this.initLocationCompleter();
	
	this.fetchFriendGroups();
	
	Event.observe($('submitButton'), 'click', function() {
		self.submitWww();
	});
}

WwwDialog.prototype.initGlobalVars = function(globalVars) {
	
	try {
		for (i in globalVars) {
			window[i] = globalVars[i];
		}
	} catch(e) {
		this.log.error('initGlobalVars(): ' + e, e);
	}
}

WwwDialog.prototype.initLanguage = function() {
	
	try {
		$('wwwDialogHeader').innerHTML = tr("Show your friends where you are and what you do!");
		
		$('visibilityLabel').innerHTML = tr("Who may see it:");
		$('visibilityFriendsOnly').label = tr("Friends only");
		$('visibilityFriendsOfFriends').label = tr("Friends of friends");
		$('visibilityHyvers').label = tr("Hyvers");
		$('visibilityEverybody').label = tr("Everybody");
		
		$('submitButton').down().down().value = tr(" Send ");
		$('locationhistory_addToHub').innerHtml = tr(" Add to spot");
	} catch(e) {
		this.log.error('initLanguage(): ' + e, e);
	}
}

WwwDialog.prototype.initHeader = function() {
	
	var ownItem = window.roster.ownItemJSON().parseJSON();
	
	if (ownItem.userhome) {
		$('avatarLink').href = 'http://' + ownItem.userhome;
	}
	if (ownItem.mediaId && ownItem.mediaSecret) {
		window.mediaLocator.setMedia('avatarImage', ownItem.mediaId, ownItem.mediaSecret);
	}
	
	templates.updateElement('currentWwwHeader', 'wwwDialog.currentWww', {
		item: ownItem
	});
}

WwwDialog.prototype.initLocationCompleter = function() {
	
	var mapDivName = 'location_history_map';
	var wwwFavId = 'www_favoritelocation';
	var wwwBoxId = 'www_box';
	var addToHub = 'locationhistory_addToHub_span';
	
	var completer = new LocationHistoryCompleter();
	completer.init(wwwFavId, wwwBoxId, addToHub);
	completer.locationSelected.observe(function(event) {
		$(mapDivName).hide();
		completer.getMapForLocation(event.memo.value, function(map_html) {
			if(map_html != false) {
				$(mapDivName).show();
				$(mapDivName).update(map_html);
			}
		});
	});

	completer.pulldownMenuShown.observe(function(event) { wwwDialog.updateDialogHeight(); });
	completer.pulldownMenuHidden.observe(function(event) { wwwDialog.updateDialogHeight(); });

	$(addToHub).hide();
}

WwwDialog.prototype.fetchFriendGroups = function() {
	
	var self = this;
	new Ajax.Request('/', {
		method: 'get', 
		parameters: 'module=FriendGroup&action=getFriendGroupsJSON',
		onComplete: function(request) {
			self.initFriendGroups(request);
		},
		onException: function(request, exception) {
			self.log.error('wwwDialog.js:fetchFriendGroups(): ' + exception, exception);
		}
	});
}

WwwDialog.prototype.initFriendGroups = function(request) {
	
	var visibilityInput = $('visibilityInput');
	visibilityInput.insert('<option value="0" label="---" fgId="-1" />');
	
	var friendGroups = request.responseText.parseJSON();
	if (friendGroups && !(friendGroups instanceof Array)) {
		for (var id in friendGroups) {
			var name = friendGroups[id];
			visibilityInput.insert('<option value="10" label="' + escape(name) + '" fgId="' + escape(id) + '" />');
		}
	} else {
		this.log.warn('wwwDialog.js:initFriendGroups(): Could not parse result.');
	}
	
	visibilityInput.insert('<option value="0" label="' + tr('Modify friend groups') + '" fgId="-2" />');
	
	var childElements = visibilityInput.childElements();
	for (var i = 0; i < childElements.length; i++) {
		var element = childElements[i];
		if (element.selected) {
			this.currentVisibilityOption = element;
			break;
		}
	}
}

WwwDialog.prototype.onVisibilityChanged = function() {
	
	var childElements = $('visibilityInput').childElements();
	for (var i = 0; i < childElements.length; i++) {
		var element = childElements[i];
		if (element.selected) {
			var fgId = element.getAttribute('fgId');
			if (fgId == -2) {
				window.windowManager.openUrl('http://' + hyves_base_url + '/vrienden/friendgroup/');
			}
			if (fgId < 0) {
				if (this.currentVisibilityOption) {
					this.currentVisibilityOption.selected = true;
				}
				break;
			}
			
			$('friendGroup').value = fgId;
			this.currentVisibilityOption = element;
			break;
		}
	}
}

WwwDialog.prototype.submitWww = function() {
	
	this.log.usage('SubmitWww');
	
	window.settingsManager.setValue('WWW/lastVisibility', $('visibilityInput').value);
	
	var self = this;
	setTimeout(function() {
		window.windowManager.raiseEvent(ObjectToJSONString({
			name: 'submitWww',
			emotion: self.rte.value,
			where: $('www_favoritelocation').value,
			whereOther: $('www_box').value,
			visibility: $('visibilityInput').value,
			addToHub: $('locationhistory_addToHub').checked,
			friendGroup: $('friendGroup').value
		}));
	}, 50);
}

/**
 * Resizes the WWW dialog when the smilies or media tab is toggled
 */
WwwDialog.prototype.updateDialogHeight = function() {

	var id = this.windowId;
	setTimeout(function() {
		// add 25px padding for the bottom
		var dialogHeight = window.document.body.offsetHeight + 25 + window.windowManager.titleBarHeight() + window.windowManager.frameHeight();
		var pulldownMenu = $('www_pulldown');
		// the location finder menu might overlap... add extra space if necessary
		var pulldownHeight = pulldownMenu.offsetHeight + pulldownMenu.offsetTop;
		if ((pulldownHeight - window.document.body.offsetHeight) > 0) {
			dialogHeight += pulldownHeight - window.document.body.offsetHeight;
		}
		// only increase window height (do not resize window if less space is needed)
		if (dialogHeight > window.innerHeight) {
			window.windowManager.windowExecute(id, 'update', ObjectToJSONString({ height: dialogHeight }));
		}
	}, 25);
}

/**
 * Initializes the WWW dialog on window load event.
 */
function wwwDialogOnload() {
	logging.debug('wwwDialogOnload()');
	
	window.mediaLocator = new MediaLocator();
	window.wwwDialog = new WwwDialog();
}
