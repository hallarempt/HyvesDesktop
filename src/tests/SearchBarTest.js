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

/*
 * Test for the '/'-key shortcut (should give focus to searchbox)
 */
function testFocusToSearchBar() {

	window.tester.sendKeyPressToWindow(1, '/');
	window.tester.sendKeyPressToWindow(1, 'k');
	window.tester.sendKeyPressToWindow(1, 'i');
	window.tester.sendKeyPressToWindow(1, 'l');
	assertEqual('$("searchbox_input").value', 'kil');
}

/*
 * Test the searchbox's close button:
 * 1. button should become visible once text is entered into the searchbox
 * 2. check whether the button works
 * 3. button should disappear (hide) when the searchbox is empty
 */
function testSearchbarClosebutton() {

	var searchbox = $('searchbox_input');
	var closebutton = $('searchbox_close');

	searchbox.value='';
	searchbox.focus();
	window.tester.sendKeyPressToWindow(1, 't');
	assertEqual('$("searchbox_close").style.visibility', 'visible');
	simulateClick('searchbox_close');
	assertEqual('$("searchbox_close").style.visibility', 'hidden');
	assertEqual('$("searchbox_input").value', '');
}

/*
 * Check whether the search/filter actually works
 * Note: this test assumes that at least the following friends are online:
 *       -> Kilian
 *       -> Blaatpuk
 *       -> Youri
 *       ...other friends will be ignored.
 */
function testSearchResult() {

	var searchbox = $('searchbox_input');
	window.tester.sendKeyPressToWindow(1, '/');	//searchbox needs a keypress event to refresh roster
	searchbox.value = '';

	// check whether everyone is online and visible
	assertEqual('$("dock_roster_item_kilian").style.display', ''); // Kilian
	assertEqual('$("dock_roster_item_toymachine").style.display', ''); // Blaak
	assertEqual('$("dock_roster_item_optroodt").style.display', ''); //Youri

	window.tester.sendKeyPressToWindow(1, '/');
	searchbox.value = 'kilian'; // set filter for kilian
	sleep(1);

	// only kilian should be visible now
	assertEqual('$("dock_roster_item_kilian").style.display', '');
	assertEqual('$("dock_roster_item_toymachine").style.display', 'none');
	assertEqual('$("dock_roster_item_optroodt").style.display', 'none');
}

/*
 * Test if the first search result (top most item) is being selected
 * (of course for this to work, the search filter must be working, too)
 */
function testSearchResultSelection() {

	var searchbox = $('searchbox_input');

	window.tester.sendKeyPressToWindow(1, '/');	//searchbox needs a keypress event to refresh roster
	searchbox.value = 'kilian';
	sleep(1); // wait for roster to refresh

	assertMoreThan(($("dock_roster_item_kilian").className).indexOf("selected", 0), -1); //kilian shoud be in class 'selected'
	assertLessThan(($("dock_roster_item_toymachine").className).indexOf("selected", 0), 0); // everyone else shouldn't
	assertLessThan(($("dock_roster_item_optroodt").className).indexOf("selected", 0), 0);
	searchbox.value = '';
	sleep(1);

	window.tester.sendKeyPressToWindow(1, '/');
	searchbox.value = 'youri';	// run another test for youri... just in case
	sleep(1); // wait for roster to refresh

	assertMoreThan(($("dock_roster_item_optroodt").className).indexOf("selected", 0), -1); // only Youri should be selected this time
	assertLessThan(($("dock_roster_item_kilian").className).indexOf("selected", 0), 0);
	assertLessThan(($("dock_roster_item_toymachine").className).indexOf("selected", 0), 0);
}

/*
 * Test for the 'Escape'-key shortcut (should clear the searchbox)
 */
function testEscapeSearch() {

	var searchbox = $('searchbox_input');

	searchbox.focus();
	searchbox.value = 'test';
	window.tester.sendKeyPressToWindow(1, '\u001B'); // send ESC-key
	sleep(1);
	assertEqual('$("searchbox_input").value', ''); // escape should have cleared the searchbox
}
