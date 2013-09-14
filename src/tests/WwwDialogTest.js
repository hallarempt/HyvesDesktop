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

var globalMainWindowId = 1;

function testOpenWwwDialog() {
	
	simulateClick('addWwwLink');
	
	sleep(2);
	
	assertMoreThan('window.xmppClient.ui.wwwWindowId', 1);
	
	window.tester.setTestWindowForNextMethod(window.xmppClient.ui.wwwWindowId);
}

function testSubmitPlainWww() {
	
	$('richtextcontrol__code').value = 'test1';
	$('www_box').value = 'hyves hq';
	
	simulateClick('submitButton');
	
	window.tester.setTestWindowForNextMethod(globalMainWindowId);
}

function testWwwSubmitted() {
	
	sleep(10);
	
	assertEqual('window.xmppClient.ui.wwwWindowId', 0);
	
	assert("$('userWwwLabel').innerHTML.indexOf('<strong>www: </strong>test1@hyves hq') > 0");
}

function testOpenWwwDialog2() {
	
	simulateClick('addWwwLink');
	
	sleep(2);
	
	assertMoreThan('window.xmppClient.ui.wwwWindowId', 1);
	
	window.tester.setTestWindowForNextMethod(window.xmppClient.ui.wwwWindowId);
}

function testSubmitSpotWww() {
	
	$('richtextcontrol__code').value = 'test2';
	$('www_favoritelocation').value = '29_9143596';
	
	simulateClick('submitButton');
	
	window.tester.setTestWindowForNextMethod(globalMainWindowId);
}

function testSpotWwwSubmitted() {
	
	sleep(10);
	
	assertEqual('window.xmppClient.ui.wwwWindowId', 0);
	
	assert("$('userWwwLabel').innerHTML.indexOf('<strong>www: </strong>test2@Hyves HQ') > 0");
}
