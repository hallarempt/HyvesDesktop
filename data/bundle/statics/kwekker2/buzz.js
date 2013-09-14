
var FEED_REFRESH_INTERVAL = 300000; // refresh every 5 minutes

templates.addFromString('buzz.notificationItem', '<div class="notification"><span class="date">${dateFormat.formatTimeSmall(date)}</span> <a href="${action}" onclick="window.windowManager.openUrl(\'${action}\'); return false;">${subject}</a></div>');

function BuzzManager(component) {
	
	this.component = component;
	
	this.numHeadlines = 0;
	window.mainWindow.setNumNotifications(this.numHeadlines)
	
	this.cleanHTML = $('notificationsContent').innerHTML;
	
	window.extender.connect('windowManager', 'eventRaised(QVariantMap)', 'window.buzzManager.onEventRaised');
	
	windowResized();
	
	var self = this;
	Event.observe('deleteNotifications', 'click', function(event) {
		self.clearNotifications();
	});
	
	Event.observe('closeButton', 'click', function(event) {
		self.hideNotificationView();
	});
	
	setInterval(function() {
		self.refreshFeed();
	}, FEED_REFRESH_INTERVAL);
}

BuzzManager.prototype.onEventRaised = function(event) {
	
	if (event.name == 'headlineReceived') {
		this.onHeadline(event);
	}
}

BuzzManager.prototype.onHeadline = function(event) {
	
	try {
		this.numHeadlines++;
		window.mainWindow.setNumNotifications(this.numHeadlines);
		
		templates.insertTop('notificationsContent', 'buzz.notificationItem', {
			subject: event.subject,
			action: event.url,
			date: parseDateString14(event.date)
		});
	} catch(e) {
		logging.error('buzz.js:onHeadline(): ' + e, e);								
	}
}

BuzzManager.prototype.toggleNotificationView = function() {
	
	if ($('notifications').style.display == 'none') {
		this.showNotificationView();
	} else {
		this.hideNotificationView();
	}
}

BuzzManager.prototype.showNotificationView = function() {
	
	$('notifications').show();
}

BuzzManager.prototype.hideNotificationView = function() {
	
	$('notifications').hide();
}

BuzzManager.prototype.clearNotifications = function() {
	
	$('notificationsContent').innerHTML = this.cleanHTML;
	
	var self = this;
	Event.observe('deleteNotifications', 'click', function(event) {
		self.clearNotifications();
	});
	
	this.numHeadlines = 0;
	window.mainWindow.setNumNotifications(this.numHeadlines);
}

BuzzManager.prototype.refreshFeed = function() {
	
	new Ajax.Updater('contentCell',	'?module=DesktopBuzz&action=showFeed&component=' + this.component);
}

function initBuzz(component) {
	
	if (component === undefined) {
		component = 'overview';
	}
	
	window.buzzManager = new BuzzManager(component);
}

function windowResized() {
	
	// this beautiful hack makes the horizontal scrollbar disappear by
	// explicitly setting the width of the paragraphs containing WWW's.
	setTimeout(function() {
		var styleSheet = document.styleSheets[document.styleSheets.length - 1];
		if (window.buzzRuleAdded) {
			styleSheet.removeRule(styleSheet.cssRules.length - 1);
		}
		
		styleSheet.addRule('.feed .buzz-descr p', 'width: ' + ($('contentCell').offsetWidth - 34) + 'px');
		window.buzzRuleAdded = true;
	}, 50);
}
