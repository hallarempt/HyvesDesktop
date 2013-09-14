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

/**
 * Tests the existance of the roster items and some of their properties.
 */
function testRosterItems() {
	
	assertEqual('$("dock_roster_item_kilian").style.display', '');
	assertEqual('$("dock_roster_item_kilian").down(".www").textContent', '<blink>JOEHOE</blink>">" @<blink>XSSland</blink>');
	assertEqual('$("dock_roster_item_kilian").down(".wwwTime").textContent', '16 okt, 11:37');
	
	assertEqual('$("dock_roster_item_optroodt").style.display', '');
	
	assertEqual('$("dock_roster_item_toymachine").style.display', '');
}

/**
 * Tests the order of the roster items.
 */
function testItemsOrder() {
	
	assertEqual('$("uiRosterListOnline_toymachine@hyves.nl").next().id', 'uiRosterListOnline_kilian@hyves.nl');
	
	if ($("uiRosterListOnline_salsel@hyves.nl")) {
		assertEqual('$("uiRosterListOnline_kilian@hyves.nl").next().id', 'uiRosterListOnline_salsel@hyves.nl');
		assertEqual('$("uiRosterListOnline_salsel@hyves.nl").next().id', 'uiRosterListOnline_optroodt@hyves.nl');
	} else {
		assertEqual('$("uiRosterListOnline_kilian@hyves.nl").next().id', 'uiRosterListOnline_optroodt@hyves.nl');
	}
}
