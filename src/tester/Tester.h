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

#ifndef TESTER_H
#define TESTER_H

#include <QObject>
#include <QPair>
#include <QString>
#include <QVariant>

#include "hyveslib_export.h"

class QWebPage;

namespace Tester {

/**
 * The Tester provides with methods that aid unit testing inside the QtWebKit
 * environment.
 */
class HYVESLIB_EXPORT Tester : public QObject {
	
	Q_OBJECT
	
	public:
		enum Options {
			NoOption = 0,
			DontWaitForLoad = 1
		};
		
		/**
		 * Returns whether tests are currently being executed.
		 *
		 * This is the case if an instance of the Tester has been created.
		 */
		static bool isTesting();
		
		/**
		 * Returns the global Tester instance.
		 */
		static Tester *instance();
		
		/**
		 * Destroys the global Tester instance.
		 */
		static void destroy();
		
		/**
		 * Sets the web page in which we will execute the tests.
		 *
		 * @param page The page that will provide the context in which to run
		 *             the tests.
		 */
		void setTestPage(QWebPage *page);
		
		/**
		 * Schedules a test file for execution. The file is supposed to contain
		 * one or more functions whose names start with "test". Test functions
		 * are executed in order of their definitions.
		 *
		 * If a page reload occurs, the Tester will wait for the page to have
		 * finished loading before executing the next function.
		 *
		 * @param path Path to the file containing the test functions.
		 *
		 * To start executing all scheduled test files, call runAllTests().
		 */
		void scheduleTestFile(const QString &path);
		
		/**
		 * Start testing of all scheduled tests.
		 *
		 * Once all tests have finished, the signal testsFinished() is emitted.
		 * 
		 * @param options Options for running the tests. Use DontWaitForLoad to
		 *                start running the tests immediately, rather than
		 *                waiting until the web page is loaded.
		 */
		void runAllTests(Options options = NoOption);
		
		/**
		 * Executes a single block of JavaScript code that performs tests. The
		 * test code will pass if none of the assertions done by the test code
		 * fail, and no unhandled exceptions are caught.
		 *
		 * @param code JavaScript code to execute.
		 * @return Returns @c true if the test code passed, otherwise @c false.
		 */
		bool executeTestCode(const QString &code);
		
		/**
		 * Returns the number of tests that have failed after a test run
		 * executed using runAllTests().
		 */
		int numFailedTests() const;
		
	public slots:
		/**
		 * Marks the test currently being executed as failed. Called from the
		 * JavaScript assert*() functions.
		 *
		 * @param message Message specifying the reason for the failure.
		 */
		void failTest(const QString &message);
		
		/**
		 * Lets the test script sleep for the given amount of milliseconds.
		 */
		void msleep(int milliseconds);
		
		/**
		 * Sets the web page in which we will execute the next tests.
		 *
		 * @param windowId windowId identifying the window where we want to execute javascript code.
		 */
		void setTestWindowForNextMethod(int windowId);
		
		/**
		 * Tells the tester that a new page is about to be loaded in the webpage
		 */
		void markPageAboutToLoad();
		
		/**
		 * Sends a key press event to a window.
		 *
		 * @param windowId ID of the window to send the key press to.
		 * @param key Single character for the key to send.
		 */
		void sendKeyPressToWindow(int windowId, const QString &key);
		
	signals:
		/**
		 * Emitted when all scheduled tests are finished.
		 * 
		 * You can get the number of failed tests using numFailedTests();
		 */
		void testsFinished();
		
	private:
		static Tester *s_instance;
		
		struct Private;
		Private *const m_d;
		
		Tester();
		virtual ~Tester();
		
		bool loadNextTestFile();
		void injectJavaScriptFileIntoPage(const QString &path);
		void injectJavaScriptTesterFileIntoPage();
		
	private slots:
		void continueRunningTests();
		
		void loadStarted();
		void loadFinished(bool ok);
};

} // namespace Tester

#endif // TESTER_H
