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

function testInitialization() {
	
	assert("window.contextMenuManager");
	assert("window.extender");
	assert("window.hyvesApi");
	assert("window.informer");
	assert("window.jabber");
	assert("window.mainWindow");
	assert("window.player");
	assert("window.plugger");
	assert("window.roster");
	assert("window.settingsManager");
	assert("window.systemSpy");
	assert("window.systemTrayIcon");
	assert("window.translator");
	assert("window.updater");
	assert("window.windowManager");
}

function testIntroductionPageOpened() {
	
	assertNotEqual('window.windowManager.windowInfoJSON(2)', 'false');
	
	window.windowManager.closeWindow(2);
}
