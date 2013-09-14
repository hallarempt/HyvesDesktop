
templates.addFromString('history.dateItem', '<li id="dateItem_${date}" class="${%active ? \'active\' : \'\'%}">${dateFormat.formatDateLong(parseHistoryDateString(date))}</div>');
templates.addFromString('history.message', '<li class="${%ownMessage ? \'ownMessage\' : \'otherMessage\'%}"><cite><img src="http://localhost/images/nixel.gif" alt="" onload="window.mediaLocator.setMedia(this, \'${mediaId}\', \'${mediaSecret}\')"></cite><blockquote><p>${msg|softHyphen:20}</p></blockquote></li>');
templates.addFromString('history.toMemberInfo',
        '<div id="usr-optns">' +
                '<div class="avatar small">' +
                        '<a href="http://${item.userhome}">' +
                                '<img src="http://localhost/images/nixel.gif" alt="" onclick="" onload="window.mediaLocator.setMedia(this, \'${item.mediaId}\', \'${item.mediaSecret}\');" class="square-img">' +
                        '</a>' +
                '</div>' +
                '<p style="padding: 9px 0px 0px">${item.nick|fancyLayout:"nickname,nosmilies"}</p>' +
                '<br />' +
        '</div>'
);

function HistoryManager() {
	
	this.log.usage('HistoryWindowOpened');
	
	this.jid = null;
	this.windowId = 0;
	
	this.initLanguage();
	
	window.extender.connect('windowManager', 'eventRaised(QVariantMap)', 'window.historyManager.onEventRaised');
	
	setTimeout(function() {
		window.windowManager.raiseEvent(ObjectToJSONString({
			name: 'historyWindowAvailable'
		}));
	}, 10);
}

HistoryManager.prototype.log = logging.getLogger('HistoryManager');

HistoryManager.prototype.initLanguage = function() {
	
	try {
		$('deleteButton').down().innerHTML = tr("Delete history");
		$('searchInput').setAttribute('placeholder', tr("Search in messages"));
	} catch(e) {
		this.log.error('initLanguage(): ' + e, e);
	}
}

HistoryManager.prototype.onEventRaised = function(event) {
	
	if (event.name == 'openHistory') {
		this.openHistory(event);
	}
}

HistoryManager.prototype.openHistory = function(event) {
	
	this.initGlobalVars(event.globalVars);
	
	this.jid = jidbare(event.jid);
	this.windowId = event.windowId;
	this.currentDate = null;
	
	this.fromMemberInfo = event.fromMemberInfo;
	this.toMemberInfo = event.toMemberInfo;
	
        var data = { item: this.toMemberInfo };
        templates.updateElement('toMemberInfo', 'history.toMemberInfo', data);
	
	this.search('');
}

/**
 * Searches for all dates containing a specific keyword. If the keyword is
 * empty, all dates on which messages were sent are found.
 *
 * @param keyword Keyword to search for, may be empty.
 */
HistoryManager.prototype.search = function(keyword) {
	
	var historyDates = window.historyKeeper.historyDates(this.jid, keyword);
	var dates = (historyDates.length > 0 ? historyDates.split(',') : []);
	
	var datesList = $('datesList');
	datesList.innerHTML = '';
	var self = this;
	for (var i = 0; i < dates.length; i++) {
		var date = dates[i];
		templates.insertBottom(datesList, 'history.dateItem', {
			date: date,
			active: (i == 0)
		});
		
		$('dateItem_' + date).observe('click', function(event) {
			self.openDate(event.element().id.substr(9), keyword);
		});
	}
	
	this.currentDate = null;
	if (dates.length > 0) {
		this.openDate(dates[0]);
	} else {
		var messageContainer = $('messageContainer');
		messageContainer.innerHTML = tr("(No messages found)");
	}
}

HistoryManager.prototype.initGlobalVars = function(globalVars) {
	
	try {
		for (i in globalVars) {
			window[i] = globalVars[i];
		}
	} catch(e) {
		this.log.error('initGlobalVars(): ' + e, e);
	}
}

/**
 * Opens all messages for a particular date.
 *
 * @param date Date to get the messages for, formatted like "YYYY-MM-DD".
 */
HistoryManager.prototype.openDate = function(date) {
		
	var messageContainer = $('messageContainer');
	messageContainer.innerHTML = '';
	
	var highlight = $('searchInput').value;
	var regExp = new RegExp(highlight, 'gi');
	
	var historyMessages = window.historyKeeper.messagesFromDayJSON(this.jid, date).parseJSON().messages;
	for (var i = 0; i < historyMessages.length; i++) {
		var historyMessage = historyMessages[i];
		
		historyMessage.message = fancy_layout(historyMessage.message, 'body|newtarget');
		if (highlight != '') {
			historyMessage.message = historyMessage.message.replace(regExp, '<span style="background-color: #ff0">$&</span>');
		}
		
		var ownMessage = (historyMessage.direction == 'to');
		templates.insertBottom(messageContainer, 'history.message', {
			msg: filterMessageForDisplay(historyMessage.message),
			ownMessage: ownMessage,
			mediaId: (ownMessage ? this.fromMemberInfo.mediaId : this.toMemberInfo.mediaId),
			mediaSecret: (ownMessage ? this.fromMemberInfo.mediaSecret : this.toMemberInfo.mediaSecret)
		});
	}
	
	if (this.currentDate) {
		$('dateItem_' + this.currentDate).removeClassName('active');
	}
	$('dateItem_' + date).addClassName('active');
	this.currentDate = date;
}

/**
 * Deletes all history for the current contact.
 */
HistoryManager.prototype.deleteHistory = function() {
	
	if (!window.windowManager.askConfirmation(this.windowId, tr('Delete history'),
	                                          tr('This will delete all history of conversations with %1. Are you sure you want to delete the history?').arg(fancy_layout(this.toMemberInfo.nick, 'none')))) {
		return;
	}
	
	window.historyKeeper.clear(this.jid);
	
	this.search('');
}

function initHistory() {
	
	window.historyManager = new HistoryManager();
}

function updateSearch() {
	
	onidle('historySearch', 300, function() {
		window.historyManager.search($('searchInput').value);
	});
}
