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

function testNotifications() {
	
	assertEqual('window.mainWindow.numNotifications()', 0);
	assertEqual('window.systemTrayIcon.icon()', 'active');
	
	window.jabber.incomingXml(
		"<message type='headline' to='arendjr@hyves.nl' from='boudewijnrempt@hyves.nl'>" +
			"<notification xmlns='http://hyves.net/xmpp' url='http://arendjr.hyves.nl/?ref=newscrap' from='7787013' jid='boudewijnrempt@hyves.nl' mask='15' datetime='20090318134505'/>" +
			"<subject>boud krabbelt: scratch</subject>" +
			"<body>boud krabbelt: scratch http://arendjr.hyves.nl/?ref=newscrap</body>" +
		"</message>"
	);
	
	sleep(1);
	
	assertEqual('window.mainWindow.numNotifications()', 1);
	assertEqual('window.systemTrayIcon.icon()', 'highlight');
	
	window.jabber.incomingXml(
		"<message type='headline' to='arendjr@hyves.nl' from='boudewijnrempt@hyves.nl'>" +
			"<notification xmlns='http://hyves.net/xmpp' url='http://www.hyves.nl/berichten/inbox/berichten/746338801/Onbekend/' from='7787013' jid='boudewijnrempt@hyves.nl' mask='15' datetime='20090318134505'/>" +
			"<subject>Nieuw bericht van boud</subject>" +
			"<body>Nieuw bericht van boud http://www.hyves.nl/berichten/inbox/berichten/746338801/Onbekend/</body>" +
		"</message>"
	);
	
	sleep(1);
	
	assertEqual('window.mainWindow.numNotifications()', 2);
	assertEqual('window.systemTrayIcon.icon()', 'highlight');
	
	window.mainWindow.toggleNotificationView();
	
	sleep(1);
	
	assertEqual("$('notificationsContent').style.display", '');
	
	assertMoreThan("$('notificationsContent').innerHTML.indexOf('boud krabbelt: scratch')", 0);
	assertMoreThan("$('notificationsContent').innerHTML.indexOf('Nieuw bericht van boud')", 0);
	
	simulateClick('deleteNotifications');
	
	sleep(1);
	
	assertEqual("$('notificationsContent').innerHTML.indexOf('boud krabbelt: scratch')", -1);
	assertEqual("$('notificationsContent').innerHTML.indexOf('Nieuw bericht van boud')", -1);
	window.mainWindow.toggleNotificationView();
	
	sleep(1);
	
	assertEqual('window.mainWindow.numNotifications()', 0);
	assertEqual('window.systemTrayIcon.icon()', 'active');

	window.jabber.incomingXml(
		"<message type='headline' to='arendjr@hyves.nl' from='boudewijnrempt@hyves.nl'>" +
			"<notification xmlns='http://hyves.net/xmpp' url='http://www.hyves.nl/berichten/inbox/berichten/746338801/Onbekend/' from='7787013' jid='boudewijnrempt@hyves.nl' mask='15' datetime='20090318134505'/>" +
			"<subject>Nieuw bericht van boud</subject>" +
			"<body>Nieuw bericht van boud http://www.hyves.nl/berichten/inbox/berichten/746338801/Onbekend/</body>" +
		"</message>"
	);
	
	sleep(1);
	
	assertEqual('window.mainWindow.numNotifications()', 1);
	assertEqual('window.systemTrayIcon.icon()', 'highlight');
	
	window.mainWindow.toggleNotificationView();
	
	sleep(1);
	
	simulateClick('deleteNotifications');
	
	sleep(1);
	
	assertEqual('window.mainWindow.numNotifications()', 0);
	assertEqual('window.systemTrayIcon.icon()', 'active');
	
	window.mainWindow.toggleNotificationView();
}

/**
 * Tests that the buzz view is refreshed when new content is posted.
 */
function testBuzzRefresh() {
	// in order not to pollute with fake content, we are not testing against
	// the real live feed. instead, we use two pages :
	// - news_before.html is a fictional live feed for moment t.
	// - news_after.html is a fictional live feed for moment t+1 (one item
	// has been added to news_before.html).
	// both test data files contain include directives to the buzz.js
	// script, which manages the refresh. tester instructs the
	// networkmanager to make an exception for the buzz url by providing a
	// custom url handler (pointing to news.html)
	window.mainWindow.setFakeBuzzHtml('news_before.html');
	
	// proceeds to delete and recreate buzzmanager so that we can set our
	// own FEED_REFRESH_INTERVAL. For tests purpose this interval is much
	// smaller than the production one - that way the buzz manager will
	// refresh the contents while we are still inside the test
	delete(window.buzzManager);
	FEED_REFRESH_INTERVAL = 800;
	window.buzzManager = new BuzzManager('overview');
	sleep(1);
	
	var itemNumberBefore = $('contentCell').getElementsByTagName('a').length;
	
	window.mainWindow.setFakeBuzzHtml('news_after.html');
	sleep(1);
	
	// the new content frame should have more items (we added one item).
	assertMoreThan("$('contentCell').getElementsByTagName('a').length", itemNumberBefore);
	
	window.mainWindow.clearFakeBuzzHtml();
}
