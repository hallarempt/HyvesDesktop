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
#include <limits.h>

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QKeyEvent>
#include <QStringList>
#include <QTime>
#include <QTimer>
#include <QWebFrame>
#include <QWebPage>

#include "extender/Extender.h"
#include "jabber/JabberMock.h"
#include "windowmanager/WindowManager.h"
#include "logger/Logger.h"
#include "settingsmanager/SettingsManager.h"
#include "Tester.h"
#include "WebPage.h"

/*
 * Please leave the commented debug statements in when doing cleanups.
 * Otherwise, next time we've got a problem with the tester we have to
 * spend another ten minutes putting them all back in.
 */

namespace Tester {

Tester *Tester::s_instance = 0;

struct Tester::Private {
	
	Logger::Logger *log;
	
	QWebPage *testPage;
	QWebPage *restoreTestPage;
	
	QStringList testFiles;
	QString currentTestFile;
	
	QStringList testMethods;
	int currentMethodIndex;
	
	int numPassedTests;
	int numFailedTests;
	int totalNumFailedTests;
	
	QString currentTestCode;
	bool waitForFinishedBeforeProceeding;
	bool testPassed;
	bool isTestRunning;
	bool isTestScheduled;
	
	// keeps track of where the test framework has been already injected, to avoid injecting it again.
	QMap<QWebPage *, bool> isTestFrameworkInjected;
	// keeps track of which test files have been loaded in each webpage, to avoid reloading them.
	QMap<QWebPage *, QStringList> injectedTestFiles;
	
	Private() :
		log(0),
		testPage(0),
		restoreTestPage(0),
		currentMethodIndex(INT_MAX),
		numPassedTests(0),
		numFailedTests(0),
		totalNumFailedTests(0),
		isTestRunning(false),
		isTestScheduled(false) {
	}
	
	~Private() {
		
		delete log;
	}
};

bool Tester::isTesting() {
	
	//qDebug() << "Tester::isTesting()";
	return (s_instance != 0);
}

Tester *Tester::instance() {
	
	if (s_instance == 0) {
		s_instance = new Tester();
	}
	
	return s_instance;
}

void Tester::destroy() {
	
	//qDebug() << "Tester::destroy";
	s_instance->deleteLater();
	s_instance = 0;
}

void Tester::setTestPage(QWebPage *page) {
	
	//qDebug() << "Tester::setTestPage " << page;
	if (m_d->testPage) {
		m_d->testPage->disconnect(this);		
	}
	
	m_d->testPage = page;
	m_d->isTestFrameworkInjected[page] = false;
	
	connect(m_d->testPage, SIGNAL(loadStarted()), SLOT(loadStarted()));
	connect(m_d->testPage, SIGNAL(loadFinished(bool)), SLOT(loadFinished(bool)));
}

void Tester::scheduleTestFile(const QString &path) {
	
	//qDebug() << "Tester::scheduleTestFile " << path;
	m_d->testFiles << path;
}

void Tester::runAllTests(Tester::Options options) {
	
	//qDebug() << "Tester::runAllTests";
	
	Q_ASSERT(m_d->testPage != 0);
	
	m_d->totalNumFailedTests = 0;
	
	m_d->waitForFinishedBeforeProceeding = true;
	if (options & DontWaitForLoad) {
		loadFinished(true);
	}
}

bool Tester::executeTestCode(const QString &code) {
	
	//qDebug() << "Tester::executeTestCode " << code; 
	Q_ASSERT(m_d->testPage != 0);
	
	m_d->currentTestCode = code;
	
	m_d->testPassed = true;
	m_d->testPage->mainFrame()->evaluateJavaScript(code);
	
	m_d->currentTestCode = "";
	
	return m_d->testPassed;
}

int Tester::numFailedTests() const {
	
	return m_d->totalNumFailedTests;
}

Tester::Tester() :
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("Tester");
	
	SettingsManager::SettingsManager *settingsManager = SettingsManager::SettingsManager::instance();
	settingsManager->clearSettings();
	settingsManager->setValue("General/allowMultipleClients", true, false); // makes sure multiple test runs don't clash
	
	Jabber::Jabber::setInstance(new Jabber::JabberMock());
	
	Extender::Extender::instance()->registerObject("tester", this);
}

Tester::~Tester() {
	
	Extender::Extender::instance()->unregisterObject("tester");
	
	delete m_d;
}

void Tester::failTest(const QString &message) {
	
	//qDebug() << "Tester::failTest " << message;
	QString errorMessage;
	if (!m_d->currentTestCode.isEmpty()) {
		errorMessage = m_d->currentTestCode + " failed: ";
	}
	errorMessage += message;
	
	m_d->log->warning(errorMessage);
	m_d->testPassed = false;
}

void Tester::msleep(int milliseconds) {
	
	//qDebug() << Tester::"msleep " << milliseconds;
	QTime time;
	time.start();
	do {
		qApp->processEvents(QEventLoop::WaitForMoreEvents, milliseconds / 10);
	} while (time.elapsed() < milliseconds);
}

void Tester::setTestWindowForNextMethod(int windowId) {
	
	m_d->log->notice(QString("Switching to window %1").arg(windowId));
	
	if (!m_d->restoreTestPage) {
		m_d->restoreTestPage = m_d->testPage;
	}
	
	WindowManager::WindowManager *windowManager = WindowManager::WindowManager::instance();
	QWebFrame *frame = windowManager->frame(windowId);
	QWebPage *webPage = frame->page();
	setTestPage(webPage);
	
	injectJavaScriptTesterFileIntoPage();
	
	if (!m_d->isTestScheduled) {
		m_d->waitForFinishedBeforeProceeding = true;
		QTimer::singleShot(1000, this, SLOT(continueRunningTests()));
		m_d->isTestScheduled = true;
	}
}

bool Tester::loadNextTestFile() {
	
	//qDebug() << "Tester::loadNextTestFile " << m_d->testFiles.size();
	
	if (m_d->testFiles.isEmpty()) {
		return false;
	}
	
	m_d->currentTestFile = m_d->testFiles.takeFirst();
	
	if (m_d->currentTestFile.isEmpty()) {
		return false;
	}
	
	if (m_d->restoreTestPage) {
		setTestPage(m_d->restoreTestPage);
		m_d->restoreTestPage = 0;
	}
	
	m_d->log->notice(QString("\n==========\nEXECUTING NEW TEST  %1 ...\n==========\n").arg(m_d->currentTestFile));	
	
	QFile file(m_d->currentTestFile);
	if (!file.open(QIODevice::ReadOnly)) {
		m_d->log->error(QString("Cannot open test file %1.").arg(m_d->currentTestFile));
		m_d->totalNumFailedTests++;
		return false;
	}
	
	m_d->testMethods.clear();
	m_d->currentMethodIndex = 0;
	
	//loading next text file means :
	//we have to clear map of webpages where we injected tester javascript code
	//(since we will have to reload it again for all webpages in this context)
	m_d->isTestFrameworkInjected.clear();
	
	while (!file.atEnd()) {
		QString line = file.readLine();
		
		if (line.startsWith("function test")) {
			int indexOfEndOfMethod = line.indexOf("(");
			m_d->testMethods.append(line.mid(9, indexOfEndOfMethod - 9).trimmed() + "()");
		} else if (line.startsWith("window.tester.")) {
			m_d->testMethods.append(line.trimmed());
		}
	}
	file.close();
	return true;
}

void Tester::injectJavaScriptFileIntoPage(const QString &path) {
	
	//qDebug() << "Tester::injectJavaScriptFileIntoPage " << path;
	
	if (m_d->injectedTestFiles.contains(m_d->testPage) &&
	    m_d->injectedTestFiles[m_d->testPage].contains(path)) {
		return; // file already injected
	}
	
	m_d->log->notice(QString("injecting test file %1 into web page\n").arg(path));
	
	Q_ASSERT(m_d->testPage != 0);
	Q_ASSERT(!path.isEmpty());	

	QFile file(path);
	if (!file.open(QIODevice::ReadOnly)) {
		m_d->log->error(QString("Cannot open JavaScript file %1.").arg(path));
		return;
	}
	QString content = file.readAll();
	file.close();
	
	m_d->testPage->mainFrame()->evaluateJavaScript(content);
	
	m_d->injectedTestFiles[m_d->testPage].append(path);
}


void Tester::injectJavaScriptTesterFileIntoPage() {
	
	injectJavaScriptFileIntoPage(":/src/tester/Tester.js");
}

void Tester::continueRunningTests() {

	//qDebug() << "Tester::continueRunningTests " << m_d->isTestScheduled << ", " << m_d->isTestRunning;	
	m_d->isTestScheduled = false;
	m_d->isTestRunning = true;

	//qDebug() << m_d->currentMethodIndex << ", " << m_d->testMethods.size();
	if (m_d->currentMethodIndex >= m_d->testMethods.size()) {
		if (!loadNextTestFile()) {
			QTimer::singleShot(3000, this, SIGNAL(testsFinished()));
			m_d->isTestRunning = false;
			return;
		}
		
		m_d->numPassedTests = 0;
		m_d->numFailedTests = 0;
	}
	
	Q_ASSERT(!m_d->currentTestFile.isEmpty());
	
	injectJavaScriptFileIntoPage(m_d->currentTestFile);
	
	m_d->waitForFinishedBeforeProceeding = false;
	while (!m_d->waitForFinishedBeforeProceeding) {
		
		if (m_d->testMethods[m_d->currentMethodIndex].startsWith("window.tester.")) {
			if (!executeTestCode(m_d->testMethods[m_d->currentMethodIndex])) {
				m_d->log->error(QString("FAIL: Error while executing statement: %1").arg(m_d->testMethods[m_d->currentMethodIndex]));
				m_d->numFailedTests++;
			}
		} else {
			m_d->log->notice(QString("=====> EXECUTING TEST METHOD %1 ...").arg(m_d->testMethods[m_d->currentMethodIndex]));
			
			if (executeTestCode(m_d->testMethods[m_d->currentMethodIndex])) {
				m_d->numPassedTests++;
			} else {
				m_d->numFailedTests++;
			}
		}
		
		qApp->processEvents(QEventLoop::DeferredDeletion);
		
		m_d->currentMethodIndex++;
		if (m_d->currentMethodIndex >= m_d->testMethods.size()) {
			if (m_d->numFailedTests == 0) {
				m_d->log->notice(QString("PASS %1: %2 passed, 0 failed.").arg(m_d->currentTestFile)
				                 .arg(m_d->numPassedTests));
			} else {
				m_d->log->error(QString("FAIL %1: %2 passed, %3 failed.").arg(m_d->currentTestFile)
				                 .arg(m_d->numPassedTests).arg(m_d->numFailedTests));
				m_d->totalNumFailedTests += m_d->numFailedTests;
			}
			if (!m_d->isTestScheduled) {
				QTimer::singleShot(1000, this, SLOT(continueRunningTests()));
				m_d->isTestScheduled = true;
			}
			break;
		}
	}
	m_d->isTestRunning = false;
}

void Tester::markPageAboutToLoad() {
	
	m_d->waitForFinishedBeforeProceeding = true;
}

void Tester::sendKeyPressToWindow(int windowId, const QString &key) {
	
	WindowManager::WindowManager *windowManager = WindowManager::WindowManager::instance();
	QWebFrame *frame = windowManager->frame(windowId);
	QWebPage *webPage = frame->page();
	QWidget *view = webPage->view();
	
	int keyCode;
	switch (key[0].toAscii()) {
		case 27:
			keyCode = Qt::Key_Escape;
			m_d->log->debug("Sending escape key event");
			break;
		// add more special keys here...
		default:
			keyCode = key[0].toAscii();
			m_d->log->debug(QString("Sending key event for: %1").arg(keyCode));
			break;
	}

	QKeyEvent keyPress(QEvent::KeyPress, keyCode, Qt::NoModifier, key[0]);
	QKeyEvent keyRelease(QEvent::KeyRelease, keyCode, Qt::NoModifier, key[0]);
	QCoreApplication::sendEvent(view, &keyPress);
	QCoreApplication::sendEvent(view, &keyRelease);
}

void Tester::loadStarted() {
	
	//qDebug() << "Tester::loadStarted";
}

void Tester::loadFinished(bool ok) {
	
	//qDebug() << "Tester::loadFinished " << ok;
	
	if (!ok) {
		m_d->log->error("An error occurred while loading the page. Trying to continue anyway.");
	}
	
	if (!m_d->waitForFinishedBeforeProceeding) {
		return;
	}
	
	// once a page is loaded, pre-loaded javascript tests will need to be loaded again.
	m_d->injectedTestFiles[m_d->testPage].clear();
	// the page is loaded, injects the tester engine script file before executing the tests.
	injectJavaScriptTesterFileIntoPage();
	
	if (m_d->isTestScheduled || m_d->isTestRunning) {
		return;
	}
	
	QTimer::singleShot(3000, this, SLOT(continueRunningTests()));
	m_d->isTestScheduled = true;
}

} // namespace Tester
