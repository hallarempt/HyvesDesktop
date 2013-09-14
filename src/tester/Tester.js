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
 * Asserts that the given statement, when executed, evaluates to true.
 */
function assert(statement) {
	
	var value = eval(statement);
	if (!value) {
		var message = "Assertion failed: " + statement + " = " + value;
		window.tester.failTest(message);
		throw message;
	}
}

/**
 * Asserts that the given statement, when executed, evaluates to the given
 * expected value.
 */
function assertEqual(statement, expectedValue) {
	
	var value = eval(statement);
	if (value === undefined) {
		var message = "Assertion failed: Value is undefined: " + statement
		              + " (expected: " + expectedValue + ")";
		window.tester.failTest(message);
		throw message;
	}
	if (value != expectedValue) {
		var message = "Assertion failed: Values not the same: " + statement + " = " + value
		              + " (expected: " + expectedValue + ")";
		window.tester.failTest(message);
		throw message;
	}
}

/**
 * Asserts that the given statement, when executed, does not evaluate to the
 * given unexpected value.
 */
function assertNotEqual(statement, notExpectedValue) {
	
	var value = eval(statement);
	if (value === undefined) {
		var message = "Assertion failed: Value is undefined: " + statement;
		window.tester.failTest(message);
		throw message;
	}
	if (value == notExpectedValue) {
		var message = "Assertion failed: Values are the same: " + statement + " = " + value
		              + " (not expected)";
		window.tester.failTest(message);
		throw message;
	}
}

/**
 * Asserts that the given statement, when executed, is undefined.
 */
function assertUndefined(statement) {
	
	var value = eval(statement);
	if (value !== undefined) {
		var message = "Assertion failed: Value is defined: " + statement + " = " + value
		              + " (expected undefined)";
		window.tester.failTest(message);
		throw message;
	}
}

/**
 * Asserts that the given statement, when executed, evaluates to less than the
 * given reference value.
 */
function assertLessThan(statement, referenceValue) {
	
	var value = eval(statement);
	if (value === undefined) {
		var message = "Assertion failed: Value is undefined: " + statement
		              + " (expected less than: " + referenceValue + ")";
		window.tester.failTest(message);
		throw message;
	}
	if (value >= referenceValue) {
		var message = "Assertion failed: Value too high: " + statement + " = " + value
		              + " (expected less than: " + referenceValue + ")";
		window.tester.failTest(message);
		throw message;
	}
}

/**
 * Asserts that the given statement, when executed, evaluates to less than or
 * equal to the given reference value.
 */
function assertLessThanOrEqual(statement, referenceValue) {
	
	var value = eval(statement);
	if (value === undefined) {
		var message = "Assertion failed: Value is undefined: " + statement
		              + " (expected less than or equal: " + referenceValue + ")";
		window.tester.failTest(message);
		throw message;
	}
	if (value > referenceValue) {
		var message = "Assertion failed: Value too high: " + statement + " = " + value
		              + " (expected less than or equal: " + referenceValue + ")";
		window.tester.failTest(message);
		throw message;
	}
}

/**
 * Asserts that the given statement, when executed, evaluates to more than the
 * given reference value.
 */
function assertMoreThan(statement, referenceValue) {
	
	var value = eval(statement);
	if (value === undefined) {
		var message = "Assertion failed: Value is undefined: " + statement
		              + " (expected more than: " + referenceValue + ")";
		window.tester.failTest(message);
		throw message;
	}
	if (value <= referenceValue) {
		var message = "Assertion failed: Value too low: " + statement + " = " + value
		              + " (expected more than: " + referenceValue + ")";
		window.tester.failTest(message);
		throw message;
	}
}

/**
 * Asserts that the given statement, when executed, evaluates to less more or
 * equal to the given reference value.
 */
function assertMoreThanOrEqual(statement, referenceValue) {
	
	var value = eval(statement);
	if (value === undefined) {
		var message = "Assertion failed: Value is undefined: " + statement
		              + " (expected more than or equal: " + referenceValue + ")";
		window.tester.failTest(message);
		throw message;
	}
	if (value < referenceValue) {
		var message = "Assertion failed: Value too low: " + statement + " = " + value
		              + " (expected more than or equal: " + referenceValue + ")";
		window.tester.failTest(message);
		throw message;
	}
}

/**
 * Returns the first element found with the given name.
 */
function getFirstElementByName(name) {
	
	var elements = document.getElementsByName(name);
	if (elements.length == 0) {
		var message = "Test failed: No element with name \"" + name + "\"";
		window.tester.failTest(message);
		throw message;
	}
	
	return elements[0];
}

/**
 * Sleeps for the given amount of milliseconds.
 */
function msleep(milliseconds) {
	
	window.tester.msleep(milliseconds);
}

/**
 * Sleeps for the given amount of seconds.
 */
function sleep(seconds) {
	
	window.tester.msleep(1000 * seconds);
}

/**
 * Fakes a mouse click on a given element.
 */
function simulateClick(elementOrId) {
	
	var event = document.createEvent('MouseEvents');
	event.initMouseEvent('click', true, true, window,
			     0, 0, 0, 0, 0, false, false, false, false, 0, null);
	var element = $(elementOrId);
	assert(element);
	element.dispatchEvent(event);
}
