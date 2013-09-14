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

#include <QDesktopServices>
#include <QMap>
#include <QMessageBox>
#include <QString>
#include <QUrl>
#include <QVector>
#include <QWebFrame>
#include <QWebPage>
#include <QWidget>
#include <QMessageBox>
#include <QTextDocument>

#ifdef Q_WS_MAC
#include "GrowlNotifier.h"
#endif

#include "config.h"
#include "extender/Extender.h"
#include "tester/Tester.h"
#include "logger/Logger.h"
#include "ScriptWindow.h"
#include "SliderWindow.h"
#include "Util.h"
#include "WindowBase.h"
#include "MainWindowBase.h"
#include "WindowManager.h"

namespace WindowManager {

static const int maxSliderWindows = 5;

WindowManager *WindowManager::s_instance = 0;

struct WindowManager::Private {
	Logger::Logger *log;
	
	QMap<int, WindowBase *> registeredWindows;
	int openerWindowId; // We assume that the window with the openerid is the mainwindow
	int nextWindowId;
	
	int titleBarHeight;
	int frameWidth;
	int frameHeight;
	
	QVector<SliderWindow *> sliderWindows;
#ifdef Q_WS_MAC
	GrowlNotifier *growlNotifier;
#endif
	Private() :
		log(0),
		openerWindowId(0),
		nextWindowId(1),
		titleBarHeight(0),
		frameWidth(0),
		frameHeight(0) {
#ifdef Q_WS_MAC
		growlNotifier = new GrowlNotifier();
#endif
	}
	
	~Private() {
		delete log;
#ifdef Q_WS_MAC
		delete growlNotifier;
#endif
		for (int i = 0; i < sliderWindows.size(); i++) {
			delete sliderWindows[i];
		}
	}
};

WindowManager *WindowManager::instance() {
	
	if (s_instance == 0) {
		s_instance = new WindowManager();
	}
	
	return s_instance;
}

void WindowManager::destroy() {
	
	delete s_instance;
	s_instance = 0;
}

void WindowManager::setOpenerWindowId(int windowId) {
	
	m_d->openerWindowId = windowId;
}

void WindowManager::setFrameDecorationGeometry(int titleBarHeight, int frameWidth, int frameHeight) {

	m_d->log->debug(QString("setFrameDecorationGeometry(%1, %2, %3)").arg(titleBarHeight).arg(frameWidth).arg(frameHeight));
	
	m_d->titleBarHeight = titleBarHeight;
	m_d->frameWidth = frameWidth;
	m_d->frameHeight = frameHeight;
}

int WindowManager::registerWindow(WindowBase *window) {
	
	int windowId = m_d->nextWindowId++;
	m_d->registeredWindows[windowId] = window;
	return windowId;
}

bool WindowManager::unregisterWindow(int windowId) {
	
	m_d->log->debug(QString("Unregistering window with ID %1.").arg(windowId));
	
	if (!m_d->registeredWindows.contains(windowId)) {
		return false;
	}
	
	m_d->registeredWindows.remove(windowId);
	
	return true;
}

QWidget *WindowManager::window(int windowId) const {
	
	if (!m_d->registeredWindows.contains(windowId)) {
		return 0;
	}
	
	QWidget *window = dynamic_cast<QWidget *>(m_d->registeredWindows[windowId]);
	Q_ASSERT(window != 0);
	return window;
}

QList<QWidget*> WindowManager::windows() const {
	
	QList<QWidget*> windowList;
	foreach(WindowBase* windowBase, m_d->registeredWindows) {
		QWidget* w = dynamic_cast<QWidget*>(windowBase);
		if (w) {
			windowList << w;
		}
	}
	return windowList;
}

WindowInfo WindowManager::windowInfo(int windowId) const {
	
	if (!m_d->registeredWindows.contains(windowId)) {
		return WindowInfo(0, 0);
	}
	
	QWidget *window = dynamic_cast<QWidget *>(m_d->registeredWindows[windowId]);
	Q_ASSERT(window != 0);
	return WindowInfo(windowId, window);
}

void WindowManager::raiseEvent(const QVariantMap &event) {
	
	emit eventRaised(event);
}

int WindowManager::titleBarHeight() const {
	
	return m_d->titleBarHeight;
}

int WindowManager::frameWidth() const {
	
	return m_d->frameWidth;
}

int WindowManager::frameHeight() const {
	
	return m_d->frameHeight;
}

QString WindowManager::windowInfoJSON(int windowId) const {
	
	if (!m_d->registeredWindows.contains(windowId)) {
		return "false";
	}
	
	return windowInfo(windowId).toJson();
}

int WindowManager::createWindow(QString windowClass, QString arguments) {
	
	m_d->log->debug(QString("Creating window of class %1.").arg(windowClass));
	
	QWidget *parent = dynamic_cast<QWidget *>(m_d->registeredWindows[m_d->openerWindowId]);
	Q_ASSERT(parent);
	ScriptWindow *window = new ScriptWindow(windowClass, parent);
	
	int windowId = window->windowId();
	window->initialize(arguments);
	
	connect(window, SIGNAL(loadFinished(int)), this, SIGNAL(windowLoadFinished(int)));
	
	m_d->log->debug(QString("Created window with ID %1.").arg(windowId));
	return windowId;
}

bool WindowManager::windowExecute(int windowId, QString command, QString arguments) {
	
	if (!m_d->registeredWindows.contains(windowId)) {
		m_d->log->debug(QString("Tried to execute in non-existing window with ID %1.").arg(windowId));
		return false;
	}
	
	m_d->registeredWindows[windowId]->executeCommand(command, arguments);
	return true;
}

QWebFrame *WindowManager::frame(int windowId){
	
	QWebFrame *frame = m_d->registeredWindows[windowId]->mainFrame();
	Q_ASSERT(frame != 0);
	
	return frame;
}

bool WindowManager::askConfirmation(QWidget *parent, const QString &title, const QString &question) {
	
	if (Tester::Tester::isTesting()) {
		return true;
	}
	
	QString dialogTitle(title);
	if (dialogTitle.isEmpty()) {
		dialogTitle = HD_PRODUCT_NAME;
	}
	
	int selectedButton = QMessageBox::question(parent, dialogTitle, question,
	                                           QMessageBox::Yes, QMessageBox::No);
	return (selectedButton == QMessageBox::Yes);
}

void WindowManager::showMessage(QWidget *parent, const QString &title, const QString &message) {
	
	if (Tester::Tester::isTesting()) {
		return;
	}
	
	QString dialogTitle(title);
	if (dialogTitle.isEmpty()) {
		dialogTitle = HD_PRODUCT_NAME;
	}
	
	QMessageBox::information(parent, dialogTitle, message);
}

void WindowManager::closeWindow(int windowId) {
	
	if (!m_d->registeredWindows.contains(windowId)) {
		m_d->log->debug(QString("Tried to close non-existing window with ID %1.").arg(windowId));
		return;
	}
	emit windowClosed(windowId);
	delete window(windowId);
}

void WindowManager::closeAllWindows() {
	
	foreach (WindowBase *windowBase, m_d->registeredWindows) {
		if (windowBase->windowId() > 1 && dynamic_cast<MainWindowBase*>(windowBase) == 0) {
			QWidget *window = dynamic_cast<QWidget *>(windowBase);
			Q_ASSERT(window != 0);
			window->close();
		}
	}
}

void WindowManager::raiseEvent(const QString &eventJSON) {
	
	bool error;
	QVariantMap event = Util::jsonToVariantMap(eventJSON, &error);
	if (error) {
		m_d->log->error("Could not parse event as JSON: " + eventJSON);
	} else {
		raiseEvent(event);
	}
}

void WindowManager::openUrl(const QString &url) {
	
	QDesktopServices::openUrl(QUrl(url));
}

void WindowManager::showHtmlSlider(const QString &htmlContent) {
	
	for (int i = 0; i < maxSliderWindows; i++) {
		if (i == m_d->sliderWindows.size()) {
			SliderWindow *slider = new SliderWindow();
			slider->setIndex(i);
			m_d->sliderWindows.append(slider);
		}
		
		Q_ASSERT(i < m_d->sliderWindows.size());
		if (!m_d->sliderWindows[i]->isAnimating()) {
			m_d->sliderWindows[i]->setContent(htmlContent);
			m_d->sliderWindows[i]->startAnimation();
			return;
		}
	}
}

void WindowManager::showSlider(const QString &htmlContent, const QString &textContent,
                               const QString &title, const QString &method, const QString &parameter,
			       const QString& mediaId, const QString& mediaSecret ) {
	
#ifdef Q_WS_MAC
	if (m_d->growlNotifier->isOperational()) {
	  m_d->growlNotifier->showNotification(textContent, method, parameter, title, mediaId, mediaSecret);
		return;
	}
#endif
	showHtmlSlider(htmlContent);
}

void WindowManager::closeSlider(int index) {
	
	if (index < 0 || index >= m_d->sliderWindows.size()) {
		m_d->log->debug(QString("Request to close non-existing slider with index %1.").arg(index));
		return;
	}
	
	m_d->sliderWindows[index]->stopAnimation();
}

bool WindowManager::askConfirmation(int windowId, const QString &title, const QString &question) {
	
	QWidget *parent = 0;
	if (windowId) {
		parent = window(windowId);
	}
	
	return askConfirmation(parent, title, question);
}

void WindowManager::showMessage(int windowId, const QString &title, const QString &message) {
	
	QWidget *parent = 0;
	if (windowId) {
		parent = window(windowId);
	}
	
	showMessage(parent, title, message);
}

WindowManager::WindowManager() :
	QObject(),
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("WindowManager");
	
	Extender::Extender::instance()->registerObject("windowManager", this);
}

WindowManager::~WindowManager() {
	
	closeAllWindows();
	
	Extender::Extender::instance()->unregisterObject("windowManager");
	delete m_d;
}

} // namespace WindowManager
