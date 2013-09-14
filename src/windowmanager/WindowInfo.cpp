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
#include <QDesktopWidget>
#include <QWidget>

#include "Util.h"
#include "WindowInfo.h"

namespace WindowManager {

WindowInfo::WindowInfo(int id, QWidget *window) :
	m_id(id),
	m_window(window) {
}

WindowInfo::WindowInfo(const WindowInfo &info) :
	m_id(info.m_id),
	m_window(info.m_window) {
}

WindowInfo::~WindowInfo() {
}

int WindowInfo::id() const {
	
	return m_id;
}

bool WindowInfo::active() const {
	
	Q_ASSERT(m_window != 0);
#ifdef Q_OS_WIN
	// On Windows, a minimized window can still be active
	return !(m_window->windowState() & Qt::WindowMinimized) && m_window->isActiveWindow();
#else
	return m_window->isActiveWindow();
#endif
}

int WindowInfo::left() const {
	
	Q_ASSERT(m_window != 0);
	return m_window->frameGeometry().left();
}

int WindowInfo::top() const {
	
	Q_ASSERT(m_window != 0);
	return m_window->frameGeometry().top();
}

int WindowInfo::width() const {
	
	Q_ASSERT(m_window != 0);
	return m_window->frameGeometry().width();
}

int WindowInfo::height() const {
	
	Q_ASSERT(m_window != 0);
	return m_window->frameGeometry().height();
}

int WindowInfo::innerWidth() const {
	
	Q_ASSERT(m_window != 0);
	return m_window->geometry().width();
}

int WindowInfo::innerHeight() const {
	
	Q_ASSERT(m_window != 0);
	return m_window->geometry().height();
}

bool WindowInfo::visible() const {
	
	Q_ASSERT(m_window != 0);
	return m_window->isVisible();
}

QString WindowInfo::windowState() const {
	
	Q_ASSERT(m_window != 0);
	return (m_window->isMinimized() ? "minimized" : "normal");
}

QRect WindowInfo::screenGeometry() const {
	
	Q_ASSERT(m_window != 0);
	QDesktopWidget *desktop = QApplication::desktop();
	return desktop->screenGeometry(m_window);
}

QRect WindowInfo::availableGeometry() const {
	
	Q_ASSERT(m_window != 0);
	QDesktopWidget *desktop = QApplication::desktop();
	return desktop->availableGeometry(m_window);
}

QString WindowInfo::toJson() const {
	
	QRect screen = screenGeometry();
	
	QVariantMap map;
	map["id"] = id();
	map["active"] = active();
	map["left"] = left();
	map["top"] = top();
	map["width"] = width();
	map["height"] = height();
	map["innerWidth"] = innerWidth();
	map["innerHeight"] = innerHeight();
	map["visible"] = visible();
	map["windowState"] = windowState();
	map["screenWidth"] = screen.width();
	map["screenHeight"] = screen.height();
	map["availableGeometry"] = availableGeometry();
	
	return Util::variantMapToJSON(map);
}

WindowInfo &WindowInfo::operator=(const WindowInfo &info) {
	
	m_id = info.m_id;
	m_window = info.m_window;
	return *this;
}

} // namespace WindowManager
