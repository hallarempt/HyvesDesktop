
templates.addFromString('roster.item', '<div id="${uiId}">${%templates.process(\'roster.itemContent\', { item: item, selected: selected, visible: visible, classPath: classPath, showWWWs: showWWWs, showToolTips: showToolTips })%}</div>');
templates.addFromString('roster.itemContent', 
	'<div id="dock_roster_item_${item.username}" class="RosterItem fakelink{if selected} selected{/if}" onclick="${classPath}.onItemSelected(\'${item.jid}\'); return false"{if showToolTips} onmouseover="showToolTip(\'${item.jid}\', this)\" onmouseout=\"hideToolTip(this)\"{/if}{if !visible} style="display: none"{/if}>' +
		'<div class="avatar extra-small">' +
			'<img src="http://localhost/images/nixel.gif"{if item.mediaId && item.mediaSecret} onload="window.mediaLocator.setMedia(this, \'${item.mediaId}\', \'${item.mediaSecret}\');"{/if} alt="">' +
		'</div>' +
		'<div class="RosterItemName {if item.available}{if item.status} ${item.status}{else} online{/if}{else} offline{/if}">' +
			'<span class="nick">${item.nick|fancyLayout:"nickname,nosmilies"|softHyphen:15}</span>' +
			'{if showWWWs}<span class="www">{if item.wwwEmotion}${item.wwwEmotion|fancyLayout:\"oneliner\"|softHyphen:15}{/if}{if item.wwwLocation}@${item.wwwLocation|fancyLayout:"oneliner"|softHyphen:15}</span>' +
			'{if item.wwwTime} <span class="wwwTime">${dateFormat.formatDateTimeLong(parseDateString14(item.wwwTime))}</span>{/if}{/if}{/if}' +
		'</div>' +
	'</div>'
);

/**
 * The Roster UI provides a generic UI for displaying the roster list.
 *
 * After instantiating this class, you may want to override the onItemSelected()
 * and filterText() methods for customization.
 *
 * For this class to work, two DIVs with IDs "uiRosterListOnline" and
 * "uiRosterListOffline" should be present in the DOM.
 *
 * If you want to enable keyboard navigation, you will need to add key listeners
 * to the respective DOM elements yourself and connect them to the
 * onKeyPressed() method.
 *
 * @param classPath Path through which this class can be accessed, like
 *                  'window.xmppClient.ui.roster'.
 */
function RosterUI(classPath) {
	
	this.classPath = classPath;
	
	this.sortedOnlineItems = [];
	this.sortedOfflineItems = [];
	
	this.selectedJid = null;
	this.excludedJids = [];
	
	this.showWWWs = true;
	this.showToolTips = true;
	
	window.extender.connect('roster', 'itemChanged(QVariantMap, QVariantMap)', classPath + '.onItemChanged');
	window.extender.connect('settingsManager', 'settingChanged(QString)', classPath + '.onSettingChanged');
}

RosterUI.prototype.log = logging.getLogger('RosterUI');

/**
 * Sets a list of JIDs which should not appear in the roster list.
 *
 * @param jids An array of JIDs to exclude.
 */
RosterUI.prototype.setExcludedJids = function(jids) {
	
	this.excludedJids = jids;
}

/**
 * Sets whether the roster items should display inline WWWs (Who What Where).
 *
 * By default, displaying of WWWs is enabled.
 */
RosterUI.prototype.setShowWWWs = function(showWWWs) {
	
	this.showWWWs = showWWWs;
}

/**
 * Sets whether tool tips should be displayed when hovering over roster items.
 *
 * By default, tool tips are enabled.
 */
RosterUI.prototype.setShowToolTips = function(showToolTips) {
	
	this.showToolTips = showToolTips;
}

/**
 * Loads the initial roster.
 *
 * This method should only be called after the Roster class has emitted the
 * initialized() signal.
 */
RosterUI.prototype.initialize = function() {
	this.log.debug('roster.js:initialize()');
	
	var showOfflineContacts = this.showOfflineContacts();
	var filterText = this.filterText();
	
	var items = window.roster.itemsJSON().parseJSON();
	for (var jid in items) {
		if (this.excludedJids.indexOf(jid) != -1) {
			continue;
		}
		
		this.addItem(items[jid], showOfflineContacts, filterText);
	}
}

/**
 * Performs some action when an item is selected.
 *
 * Override this method to implement a custom action. The default implementation
 * just writes a log entry.
 *
 * @param jid JID of the selected item.
 */
RosterUI.prototype.onItemSelected = function(jid) {
	
	this.log.debug('roster.js:onItemSelected(' + this.log.anonymizeEMail(jid) + ')');
}

RosterUI.prototype.onItemChanged = function(item, changedProperties) {
	
	if (this.excludedJids.indexOf(item.jid) != -1) {
		return;
	}
	
	var uiRosterList = (item.available ? 'uiRosterListOnline' : 'uiRosterListOffline');
	var uiId = uiRosterList + '_' + item.jid;
	
	if (!$(uiId)) {
		// if the item is not found, the available property changed and the item
		// should move from the online list to the offline list or vice versa
		this.removeItem(item); // remove it from the old list
		this.addItem(item); // and add it to the new list
		return;
	}
	
	if (changedProperties.nick) {
		var listName = (item.available ? 'sortedOnlineItems' : 'sortedOfflineItems');
		var oldIndex = this.indexOfItemInSortedList(item, listName);
		this.updateItemInSortedList(item, listName);
		var newIndex = this.indexOfItemInSortedList(item, listName);
		
		if (newIndex != oldIndex) {
			// re-insert the item because it's position has changed
			Element.remove(uiId);
			
			var rosterItemHtml = templates.process('roster.item', {
				item: item,
				uiId: uiId,
				selected: (item.jid == this.selectedJid),
				visible: this.itemVisible(item),
				classPath: this.classPath,
				showWWWs: this.showWWWs,
				showToolTips: this.showToolTips
			});
			
			if (newIndex < this[listName].length - 1) {
				new Insertion.Before(uiRosterList + '_' + this[listName][newIndex + 1].jid, rosterItemHtml);
			} else {
				new Insertion.Bottom(uiRosterList, rosterItemHtml);
			}
			
			return;
		}
	}
	
	templates.updateElement(uiId, 'roster.itemContent', {
		item: item,
		selected: (item.jid == this.selectedJid),
		visible: this.itemVisible(item),
		classPath: this.classPath,
		showWWWs: this.showWWWs,
		showToolTips: this.showToolTips
	});
}

RosterUI.prototype.addItem = function(item, showOfflineContacts, filterText) {
	
	var uiRosterList = (item.available ? 'uiRosterListOnline' : 'uiRosterListOffline');
	
	var rosterItemHtml = templates.process('roster.item', {
		item: item,
		uiId: uiRosterList + '_' + item.jid,
		selected: (item.jid == this.selectedJid),
		visible: this.itemVisible(item, showOfflineContacts, filterText),
		classPath: this.classPath,
		showWWWs: this.showWWWs,
		showToolTips: this.showToolTips
	});
	
	var listName = (item.available ? 'sortedOnlineItems' : 'sortedOfflineItems');
	this.addItemToSortedList(item, listName);
	
	var index = this.indexOfItemInSortedList(item, listName);
	if (index > 0) {
		new Insertion.After(uiRosterList + '_' + this[listName][index - 1].jid, rosterItemHtml);
	} else {
		new Insertion.Top(uiRosterList, rosterItemHtml);
	}
}

RosterUI.prototype.removeItem = function(item) {
	
	var uiId = 'uiRosterListOnline_' + item.jid;
	if ($(uiId)) {
		Element.remove(uiId);
		
		this.removeItemFromSortedList(item, 'sortedOnlineItems');
	}
	
	uiId = 'uiRosterListOffline_' + item.jid;
	if ($(uiId)) {
		Element.remove(uiId);
		
		this.removeItemFromSortedList(item, 'sortedOfflineItems');
	}
}

RosterUI.prototype.addItemToSortedList = function(item, listName) {
	
	var strippedItem = {
		jid: item.jid,
		nick: strippedNick(item.nick ? item.nick : item.jid)
	};
	
	var list = this[listName];
	var low = 0;
	var high = list.length;
	while (low < high) {
		var mid = low + ((high - low) >> 1);
		if (list[mid].nick < strippedItem.nick) {
			low = mid + 1;
		} else {
			high = mid;
		}
	}
	this[listName].splice(low, 0, strippedItem);
}

RosterUI.prototype.updateItemInSortedList = function(item, listName) {
	
	this.removeItemFromSortedList(item, listName);
	this.addItemToSortedList(item, listName);
}

RosterUI.prototype.removeItemFromSortedList = function(item, listName) {
	
	var list = this[listName];
	for (var i = 0; i < list.length; i++) {
		if (list[i].jid == item.jid) {
			list.splice(i, 1);
			return;
		}
	}
	
	this.log.error('ui.js:removeItemFromSortedList(): Non-existing item.');
}

RosterUI.prototype.indexOfItemInSortedList = function(item, listName) {
	
	var strippedItem = {
		jid: item.jid,
		nick: strippedNick(item.nick ? item.nick : item.jid)
	};
	
	var list = this[listName];
	var low = 0;
	var high = list.length;
	while (low < high) {
		var mid = low + ((high - low) >> 1);
		if (list[mid].nick < strippedItem.nick) {
			low = mid + 1;
		} else {
			high = mid;
		}
	}
	
	if (low < list.length && list[low].nick == strippedItem.nick) {
		return low;
	} else {
		this.log.debug('ui.js:indexOfItemInSortedList(): Non-existing item.');
		return -1;
	}
}

/**
 * Called whenever a setting in the SettingsManager changes.
 *
 * @param key The key of the setting that changed.
 */
RosterUI.prototype.onSettingChanged = function(key) {
	
	if (key == 'General/showOfflineFriends') {
		this.refreshRoster();
	}
}

/**
 * Updates the visibility of roster items. This is used for setting which items
 * should be visible and which shouldn't when the setting for showing of offline
 * friends has changed or when the text in the search filter has changed.
 */
RosterUI.prototype.refreshRoster = function() {
	this.log.debug('roster.js:refreshRoster()');
	
	var showOfflineContacts = this.showOfflineContacts();
	var filterText = this.filterText();
	
	var items = window.roster.itemsJSON().parseJSON();
	for (var jid in items) {
		if (this.excludedJids.indexOf(jid) != -1) {
			continue;
		}
		
		var item = items[jid];
		
		if (item.available) {
			var id = 'uiRosterListOnline_' + jid;
		} else {
			var id = 'uiRosterListOffline_' + jid;
		}
		
		templates.updateElement(id, 'roster.itemContent', {
			item: item,
			selected: (item.jid == this.selectedJid),
			visible: this.itemVisible(item, showOfflineContacts, filterText),
			classPath: this.classPath,
			showWWWs: this.showWWWs,
			showToolTips: this.showToolTips
		});
	}
}

/**
 * Returns whether a roster item should be visible.
 *
 * @param item The roster item.
 * @param showOfflineContacts Optional boolean determining whether offline
 *                            contacts are shown by default.
 * @param filterText Optional string on which friends should be filtered.
 * @return @c true if the item should be visible, @c false otherwise. 
 */
RosterUI.prototype.itemVisible = function(item, showOfflineContacts, filterText) {
	
	if (filterText === undefined) {
		filterText = this.filterText();
	}
	
	if (filterText == '') {
		if (item.available) {
			return true;
		}
		
		return (showOfflineContacts === undefined ? this.showOfflineContacts() : showOfflineContacts);
	} else {
		if (item.nick) {
			return strippedNick(item.nick).indexOf(filterText.toLowerCase()) != -1;
		} else {
			return item.jid.indexOf(filterText.toLowerCase()) != -1;
		}
	}
}

/**
 * Returns the text by which to filter the roster items.
 *
 * Override this method if you want to implement a search field that searches
 * through the roster list, for instance.
 */
RosterUI.prototype.filterText = function() {
	
	return '';
}

/**
 * Returns whether the user by default wants to see all his offline contacts.
 */
RosterUI.prototype.showOfflineContacts = function() {
	
	return window.settingsManager.boolValue('General/showOfflineFriends', false);
}

/**
 * Handles keyboard navigation through the roster list.
 */
RosterUI.prototype.onKeyPressed = function(event) {
	
	var searchbox = $('searchbox_input');
	switch (event.keyCode) {
		case 3:  // Nnumpad enter on Mac, fall...
		case 13: // regular enter
			this.selectItem();
			Event.stop(event);
			break;
		case 38: // up arrow
			this.moveSelectedRosterItem(-1);
			Event.stop(event);
			break;
		case 40: // down arrow
			this.moveSelectedRosterItem(1);
			Event.stop(event);
			break;
		case 27: // 'ESC'-key clears searchbar (if it has the focus)
			//this.log.debug("received escape key event");
			if ($(event.target) == searchbox) {
				searchbox.value = '';
				$('searchbox_close').style.visibility = 'hidden';
				event.stop();
			}
			break;
	}
}

/**
 * Selects the first (top) element in the roster list
 */
RosterUI.prototype.selectFirstItem = function() {

	//logging.debug("selectFirstItem() invoked.");
	var reference = { index: -1 };
	var elements = this.getVisibleElements('uiRosterListOnline', reference);
	if (this.showOfflineContacts() || this.filterText() != '') {
		reference.offset = elements.length;
		elements = elements.concat(this.getVisibleElements('uiRosterListOffline', reference));
	}

	if (elements.length > 0) {
		if (this.selectedJid) { // remove selection from current roster element
			var prevSelectedElement = $('uiRosterListOnline_' + this.selectedJid);
			if (!prevSelectedElement) {
				prevSelectedElement = $('uiRosterListOffline_' + this.selectedJid);
			}
			if (prevSelectedElement) {
				prevSelectedElement.down().removeClassName('selected');
			}
		}

		var selectedElement = $(elements[0]); // give selection to first visible item in the list
		selectedElement.down().addClassName('selected');
		selectedElement.scrollIntoView(false);
		this.selectedJid = selectedElement.id.substr(selectedElement.id.indexOf('_') + 1);
	}
}

RosterUI.prototype.selectItem = function() {
	
	var reference = { index: -1, visible: false };
	var elements = this.getVisibleElements('uiRosterListOnline', reference);
	if (this.showOfflineContacts() || this.filterText() != '') {
		reference.offset = elements.length;
		elements = elements.concat(this.getVisibleElements('uiRosterListOffline', reference));
	}
	if (!reference.visible) {
		this.selectedJid = null;
	}
	if (elements.length == 1) {
		this.selectedJid = elements[0].id.substr(elements[0].id.indexOf('_') + 1);
	}
	
	if (this.selectedJid) {
		this.onItemSelected(this.selectedJid);
	}
}

RosterUI.prototype.moveSelectedRosterItem = function(delta) {
	this.log.debug('moveSelectedRosterItem(' + delta + ')');
	
	var reference = { index: -1 };
	var elements = this.getVisibleElements('uiRosterListOnline', reference);
	if (this.showOfflineContacts() || this.filterText() != '') {
		reference.offset = elements.length;
		elements = elements.concat(this.getVisibleElements('uiRosterListOffline', reference));
	}
	if (elements.length == 0) {
		return;
	}
	
	var index = reference.index + delta;
	if (index < 0) {
		index = elements.length - 1;
	} else if (index >= elements.length) {
		index = 0;
	}
	
	if (this.selectedJid) {
		var previousElement = $('uiRosterListOnline_' + this.selectedJid);
		if (previousElement) {
			previousElement.down().removeClassName('selected');
		} else {
			previousElement = $('uiRosterListOffline_' + this.selectedJid);
			if (previousElement) {
				previousElement.down().removeClassName('selected');
			}
		}
	}
	
	var newElement = $(elements[index]);
	newElement.down().addClassName('selected');
	newElement.scrollIntoView(false);
	
	this.selectedJid = newElement.id.substr(newElement.id.indexOf('_') + 1);
}

RosterUI.prototype.getVisibleElements = function(container, reference) {
	
	var childNodes = document.getElementById(container).childNodes;

	var visibleElements = [];
	for (var i = 0; i < childNodes.length; i++) {
		if (childNodes[i].nodeType != Node.ELEMENT_NODE) {
			continue;
		}
		
		var element = childNodes[i];
		
		if (this.selectedJid && element.id.indexOf(this.selectedJid) != -1) {
			reference.index = visibleElements.length;
			if (reference.offset) {
				reference.index += reference.offset;
			}
			reference.visible = (element.firstChild.style.display != 'none');
		}
		if (element.firstChild.style.display != 'none') {
			visibleElements.push(element);
		}
	}

	return visibleElements;
}

/**
 * Strips the nick name of special tags for sorting.
 */
function strippedNick(nick) {
	
	return fancy_layout(nick, 'none').replace(/^\s+/, '').toLowerCase();
}

var currentToolTipElement = null;
var shouldHideToolTip = false;

/**
 * Shows a tooltip for the given roster item.
 *
 * @param jid Bare JID of the contact to which the roster item belongs.
 * @param element Top-level element of the roster item that's being hovered.
 */
function showToolTip(jid, element) {
	
	if (!window.roster.isInitialized()) {
		return; // disable tooltips until all roster info is loaded
	}
	
	if (window.currentToolTipElement == null) {
		onidle('showTooltip', 400, function() { showToolTipDelayed(jid, element); });
	} else if (window.currentToolTipElement != element) {
		showToolTipDelayed(jid, element);
	}
	
	window.shouldHideToolTip = false;
}

function showToolTipDelayed(jid, element) {
	
	element = $(element);
	
	var memberInfo = window.roster.itemJSON(jid).parseJSON();
	
	var positionedOffset = element.positionedOffset();
	
	var status = (memberInfo.available ? (memberInfo.status ? memberInfo.status : 'online') : 'offline');
	var statusText = (status == 'offline' ? tr("Offline") : aShowStatus[status].status);
	if (memberInfo.availableClients == ClientResourceBits.siteClient) {
		statusText += tr(" (Site)");
	}
	
	var wwwHtml = element.down('span.www').innerHTML;
	if (element.down('span.wwwTime')) {
		wwwHtml += ' <span class="wwwTime">' + element.down('span.wwwTime').innerHTML + '</span>';
	}
	
	window.toolTipper.showContactToolTip(ObjectToJSONString({
		left: positionedOffset.left,
		top: positionedOffset.top - $('rosterlist_chat').scrollTop,
		right: positionedOffset.left + element.offsetWidth,
		width: 370,
		jid: memberInfo.jid,
		username: memberInfo.username,
		nick: fancy_layout(memberInfo.nick ? memberInfo.nick : memberInfo.username, 'nickname,nosmilies'),
		status: status,
		statusText: statusText,
		mediaId: (memberInfo.mediaId && memberInfo.mediaSecret ? memberInfo.mediaId : ''),
		mediaSecret: (memberInfo.mediaId && memberInfo.mediaSecret ? memberInfo.mediaSecret : ''),
		wwwHtml: wwwHtml,
		userhome: memberInfo.userhome
	}));
	
	window.currentToolTipElement = element;
}

/**
 * Hides the tooltip that is shown for the given roster item.
 *
 * @param element Top-level element of the roster item that's no longer being
 *                hovered.
 */
function hideToolTip(element) {
	
	window.shouldHideToolTip = true;
	onidle('showTooltip', 20, hideToolTipDelayed);
}

function hideToolTipDelayed() {
	
	if (window.shouldHideToolTip) {
		window.toolTipper.hideToolTip();
		window.currentToolTipElement = null;
	}
}
