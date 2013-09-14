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

#include <QMenu>
#include <QWebFrame>
#include <QWebPage>

#include "config.h"
#include "extender/Extender.h"
#include "logger/Logger.h"
#include "windowmanager/WindowBase.h"
#include "windowmanager/WindowManager.h"
#include "ContextMenuManager.h"
#include "Util.h"

namespace ContextMenuManager {

struct ContextMenuManager::Private {
	Logger::Logger *log;
	
	Private() :
		log(0) {
	}
	
	~Private() {
		
		delete log;
	}
};

ContextMenuManager *ContextMenuManager::s_instance = 0;

ContextMenuManager *ContextMenuManager::instance() {
	
	if (s_instance == 0) {
		s_instance = new ContextMenuManager();
	}
	
	return s_instance;
}

void ContextMenuManager::destroy() {
	
	delete s_instance;
	s_instance = 0;
}

void ContextMenuManager::constructMenu(QMenu *menu, const QVariantMap &menuDefinition) {
	
	foreach(QString key, menuDefinition.keys()) {
		QVariant val = menuDefinition[key];
		if (val.type() == QVariant::Map) {
			// Create a submenu and call constructMenu again
			QVariantMap subMenuDefinition = val.toMap();
			QMenu *subMenu = menu->addMenu(key);
			constructMenu(subMenu, subMenuDefinition);
		} else if (val.type() == QVariant::StringList) {
			QStringList menuItem = val.toStringList();
			Q_ASSERT(menuItem.count() == 2);
			if (menuItem[0] == "separator" || menuItem[0] == "-") {
				menu->addSeparator();
			} else {
				QAction *action = menu->addAction(menuItem[0]); // visible text
				action->setObjectName(menuItem[1]);  // key to return to javascript
			}
		} else {
			m_d->log->debug(QString("Could not convert value associated with %1 to either a variant map or a stringlist").arg(key));
		}
	}
}
	
QString ContextMenuManager::showTemporaryMenu(int windowId, const QVariantMap &menuDefinition, const QPoint &pos) {
	
	QMenu *menu = new QMenu();
	QWidget *widget = WindowManager::WindowManager::instance()->window(windowId);
	WindowManager::WindowBase* windowBase = dynamic_cast<WindowManager::WindowBase *>(widget);
	if (!windowBase) {
		m_d->log->debug(QString("No window with id %1").arg(windowId));
		return QString();
	}
	
	constructMenu(menu, menuDefinition);
#ifdef DEBUG_BUILD
	QWebPage *page = windowBase->mainFrame()->page();
	menu->addSeparator();
	menu->addAction(page->action(QWebPage::InspectElement));
#endif
	
	QAction *action = menu->exec(pos);
	QString key = action->objectName();
	delete menu;
	emit contextMenuOptionClicked(key);
	return key;
}

QString ContextMenuManager::showTemporaryMenuJSON(int windowId, const QString &menuDefinitionJSON, int x, int y) {
	
	return showTemporaryMenu(windowId, Util::jsonToVariantMap(menuDefinitionJSON), QPoint(x, y));
}

ContextMenuManager::ContextMenuManager(QObject *parent) :
	QObject(parent),
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("ContextMenuManager");
	
#ifdef DEBUG_BUILD	
	QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
#endif	
	Extender::Extender::instance()->registerObject("contextMenuManager", this);
}

ContextMenuManager::~ContextMenuManager() {
	
	Extender::Extender::instance()->unregisterObject("contextMenuManager");
	delete m_d;
}

} // namespace ContextMenuManager
