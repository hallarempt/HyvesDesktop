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

#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QtDebug>
#include <QFile>
#include <QMoveEvent>
#include <QObject>
#include <QPoint>
#include <QWebFrame>
#include <QWidget>
#include <QLibrary>

#ifdef Q_WS_WIN
#include <windows.h>
#endif

#include "logger/Logger.h"
#include "settingsmanager/SettingsManager.h"
#include "Util.h"
#include "WindowBase.h"
#include "WindowManager.h"
#include "WebPage.h"

namespace WindowManager {

struct WindowBase::Private {
	
	Logger::Logger *log;
	
	bool initialized;
	int windowId;
	QString windowClass;
	bool trackWindowActivation;
	bool trackWindowPosition;
	
	Private() :
		log(0),
		initialized(false) {
	}
	
	~Private() {
		
		delete log;
	}
};

WindowBase::WindowBase(const QString &windowClass) :
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("WindowBase");
	
	m_d->windowId = WindowManager::instance()->registerWindow(this);
	m_d->windowClass = windowClass;
	m_d->trackWindowActivation = false;
	m_d->trackWindowPosition = false;
}

WindowBase::~WindowBase() {
	
	QVariantMap map;
	map["name"] = "windowClose";
	map["id"] = windowId();
	map["class"] = windowClass();
	
	WindowManager::instance()->raiseEvent(map);
	WindowManager::instance()->unregisterWindow(m_d->windowId);
	
	delete m_d;
}

void WindowBase::setWebPage(QWebPage *webPage) {
	
	Q_UNUSED(webPage);
}

int WindowBase::windowId() const {
	
	return m_d->windowId;
}

QString WindowBase::windowClass() const {
	
	return m_d->windowClass;
}

void WindowBase::initialize(const QString &argumentsJSON) {
	
	update(Util::jsonToVariantMap(argumentsJSON));
	
	m_d->initialized = true;
}

void WindowBase::executeCommand(const QString &command,
                                const QString &arguments) {
	
	if (command == "bringToFront") {
		bringToFront();
	} else if (command == "minimize") {
		minimize();
	} else if (command == "close") {
		close();
	} else if (command == "documentWrite") {
		documentWrite(arguments);
	} else if (command == "flash") {
		flash(argumentStringToMap(arguments));
	} else if (command == "loadJsFile") 	{
		loadJsFile(arguments);
	} else if (command == "executeJs") 	{
		executeJs(arguments);
	} else if (command == "navigate") 	{
		navigate(argumentStringToMap(arguments));
	} else if (command == "update") {
		update(Util::jsonToVariantMap(arguments));
		QApplication::processEvents();
	} else {
		m_d->log->debug(QString("executeCommand(): Unrecognized command: %1.").arg(command));
	}
}

void WindowBase::bringToFront() {
	
	QWidget *widget = dynamic_cast<QWidget *>(this);
	Q_ASSERT(widget != 0);
	
	if (widget->isMinimized()) {
		widget->showNormal();
	}
	widget->activateWindow();
#ifdef Q_WS_MAC
	// On mac, the previous lines are not enough to give the focus to the selected window. 
	// We need to explicitly raise it on top of all the other windows.
	widget->raise();
#endif
	QApplication::alert(widget, 1);
}

void WindowBase::minimize() {
	
	QWidget *widget = dynamic_cast<QWidget *>(this);
	Q_ASSERT(widget != 0);
	
	widget->showMinimized();
}

void WindowBase::close() {
	
	QWidget *widget = dynamic_cast<QWidget *>(this);
	Q_ASSERT(widget != 0);
	
	widget->close();
}

void WindowBase::documentWrite(const QString &arguments) {
	
	QWebFrame *frame = mainFrame();
	Q_ASSERT(frame != 0);
	
	frame->setHtml(arguments);
}

#ifdef Q_WS_WIN
// mostly stolen from qapplication_win.cpp. The key difference is that this
// does not check if the window is active before flashing it.
// On Windows, there is a possibility that minimized Windows can still be 
// active and even when another window is receiving keyboard input. This is
// easily reproduced by just minimizing a window, not clicking anywhere and 
// checking the result of GetActiveWindow() or QApplication::activeWindow().

typedef BOOL (WINAPI *PtrFlashWindowEx)(PFLASHWINFO pfwi);
static PtrFlashWindowEx pFlashWindowEx = 0;

static void alert_widget(QWidget *widget, int duration)
{
    bool stopFlash = duration < 0;
    if (!pFlashWindowEx) {
        QLibrary themeLib(QLatin1String("user32"));
        pFlashWindowEx  = (PtrFlashWindowEx)themeLib.resolve("FlashWindowEx");
    }
    if (pFlashWindowEx && widget) {
        DWORD timeOut = GetCaretBlinkTime();
        if (timeOut <= 0)
            timeOut = 250;
        UINT flashCount;
        if (duration == 0)
            flashCount = 10;
        else
            flashCount = duration/timeOut;
        FLASHWINFO info;
        info.cbSize = sizeof(info);
        info.hwnd = widget->window()->winId();
        info.dwFlags = stopFlash ? FLASHW_STOP : FLASHW_TRAY;
        info.dwTimeout = stopFlash ? 0 : timeOut;
        info.uCount = stopFlash ? 0 : flashCount;
        pFlashWindowEx(&info);
    }
}
#endif

void WindowBase::flash(const QMap<QString, QString> &arguments) {
	
	QWidget *widget = dynamic_cast<QWidget *>(this);
	Q_ASSERT(widget != 0);
	
#ifdef Q_WS_MAC
	int time = 1;
#else
	int time = 0;
	if (arguments.contains("time")) {
		time = 1000 * arguments["time"].toInt();
	}
#endif
	
#ifdef Q_OS_WIN
	alert_widget(widget, time);
#else
	QApplication::alert(widget, time);
#endif
}

void WindowBase::navigate(const QMap<QString, QString> &arguments) {
	
	QWebFrame *frame = mainFrame();
	Q_ASSERT(frame != 0);
	
	if (!arguments.contains("url")) {
		m_d->log->debug("navigate(): No URL to navigate given.");
		return;
	}
	
	QString url = arguments["url"];
	if (url.startsWith("http://") || url.startsWith("https://")) {
		frame->load(url);
	} else {
		frame->load("http://" + SettingsManager::SettingsManager::instance()->baseServer() + url);
	}
}

void WindowBase::update(const QVariantMap &arguments) {
	
	QWidget *widget = dynamic_cast<QWidget *>(this);
	Q_ASSERT(widget != 0);
	
	bool positionChanged = false;
	bool sizeChanged = false;
	bool visibilityChanged = false;
	
	QPoint position = widget->pos();
	QSize size = widget->frameSize();
	bool visible = widget->isVisible();
	QString windowTitle = widget->windowTitle();
	Qt::WindowFlags flags = widget->windowFlags();
	bool minimumSize = false;
	QString borderStyle;
	bool noActivate = false;
	bool openLinksInUserBrowser = true;
	QString windowState;
	QString z;
	
	for (QVariantMap::ConstIterator it = arguments.begin();
	     it != arguments.end(); ++it) {
		if (it.key() == "borderStyle") {
			borderStyle = it.value().toString();
		} else if (it.key() == "caption") {
			windowTitle = it.value().toString();
		} else if (it.key() == "height") {
			int height = it.value().toInt();
			size.setHeight(height >= 0 ? height : 0);
			sizeChanged = true;
		} else if (it.key() == "left") {
			int left = it.value().toInt();
			position.setX(left >= 0 ? left : 0);
			positionChanged = true;
		} else if (it.key() == "minimumSize") {
			minimumSize = it.value().toBool();
		} else if (it.key() == "noactivate") {
			noActivate = it.value().toBool();
		} else if (it.key() == "openLinksInUserBrowser") {
			openLinksInUserBrowser = it.value().toBool();
		} else if (it.key() == "top") {
			int top = it.value().toInt();
			position.setY(top >= 0 ? top : 0);
			positionChanged = true;
		} else if (it.key() == "trackWindowActivation") {
			m_d->trackWindowActivation = it.value().toBool();
		} else if (it.key() == "trackWindowPosition") {
			m_d->trackWindowPosition = it.value().toBool();
		} else if (it.key() == "visible") {
			visible = it.value().toBool();
			visibilityChanged = true;
		} else if (it.key() == "width") {
			int width = it.value().toInt();
			size.setWidth(width >= 0 ? width : 0);
			sizeChanged = true;
		} else if (it.key() == "windowState") {
#ifndef Q_WS_MAC
			// this is intentionally ignored on Mac (#6431)
			windowState = it.value().toString();
#endif
		} else if (it.key() == "z") {
			z = it.value().toString();
		} else {
			m_d->log->debug(QString("update(): Unrecognized window property: %1.").arg(it.key()));
		}
	}
	
	if (borderStyle == "none") {
		flags |= Qt::FramelessWindowHint;
	}
	
	if (noActivate) {
		widget->setAttribute(Qt::WA_ShowWithoutActivating, true);
		widget->setFocusPolicy(Qt::NoFocus);
	}
	
	if (!openLinksInUserBrowser) {
		QWebFrame *frame = mainFrame();
		Q_ASSERT(frame != 0);
		WebPage *webPage = dynamic_cast<WebPage *>(frame->page());
		Q_ASSERT(webPage != 0);
		webPage->setOpenLinksInUserBrowser(false);
	}
	
	if (windowState == "minimized") {
		widget->showMinimized();
	} else {
		if (z == "topmost" && !noActivate) {
			widget->raise();
		}
		
		if (positionChanged) {
			widget->move(position);
			qApp->processEvents();
		}
	}
	
	if (sizeChanged) {
		WindowManager *windowManager = WindowManager::instance();
		
		if (!(flags & Qt::FramelessWindowHint)) {
			if (m_d->initialized) {
				size -= (widget->frameSize() - widget->size());
			} else {
				size.setWidth(size.width() - 2 * windowManager->frameWidth());
				size.setHeight(size.height() - (windowManager->titleBarHeight() + windowManager->frameHeight()));
			}
		}
		
		WindowInfo windowInfo = windowManager->windowInfo(m_d->windowId);
		if (size.width() > windowInfo.availableGeometry().width()) {
			size.setWidth(windowInfo.availableGeometry().width());
		}
		if (size.height() > windowInfo.availableGeometry().height()) {
			size.setHeight(windowInfo.availableGeometry().height());
		}
		
		widget->resize(size);
		if (minimumSize) {
			widget->setMinimumSize(size);
		}
	}
	if (visibilityChanged) {
		visible ? widget->show() : widget->hide();
	}
	if (windowState == "minimized" && positionChanged) {
		widget->move(position);
	}
	
	widget->setWindowTitle(windowTitle);
	widget->setWindowFlags(flags);
}

void WindowBase::loadJsFile(const QString &path) {
	
	QWebFrame *frame = mainFrame();
	Q_ASSERT(frame != 0);
	
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly)) {
		m_d->log->error(QString("Cannot open JavaScript file %1.").arg(path));
		return;
	}
	QString content = file.readAll();
	file.close();
	
	frame->evaluateJavaScript(content);
}

void WindowBase::executeJs(const QString &statement) {
	
	QWebFrame *frame = mainFrame();
	Q_ASSERT(frame != 0);
	
	frame->evaluateJavaScript(statement);
}

void WindowBase::changeEvent(QEvent *event) {
	
	if (event->type() == QEvent::WindowActivate && m_d->trackWindowActivation) {
		QVariantMap map;
		map["name"] = "windowActivate";
		map["id"] = windowId();
		map["class"] = windowClass();
		map["active"] = true;
		
		WindowManager::instance()->raiseEvent(map);
	}
}

void WindowBase::moveEvent(QMoveEvent *event) {
	
	if (m_d->trackWindowPosition) {
		QWidget *widget = dynamic_cast<QWidget *>(this);
		Q_ASSERT(widget != 0);
		
		QVariantMap map;
		map["name"] = "windowPositionChanged";
		map["id"] = windowId();
		map["class"] = windowClass();
		map["x"] = event->pos().x();
		map["y"] = event->pos().y();
		map["cx"] = widget->width();
		map["cy"] = widget->height();
		
		WindowManager::instance()->raiseEvent(map);
	}
}

QMap<QString, QString> WindowBase::argumentStringToMap(const QString &arguments) {
	
	QMap<QString, QString> argumentMap;
	QStringList argumentList = arguments.split(",");
	foreach(QString argument, argumentList) {
		QStringList parts = argument.split("=");
		QString key = parts.takeFirst();
		QString value = parts.join("=");
		argumentMap[key] = value;
	}
	return argumentMap;
}

} // namespace WindowManager
