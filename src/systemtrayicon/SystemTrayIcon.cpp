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

#include <QAction>
#include <QActionGroup>
#include <QCursor>
#include <QIcon>
#include <QMap>
#include <QMenu>

#include "config.h"
#include "extender/Extender.h"
#include "logger/Logger.h"
#include "plugger/Plugger.h"
#include "tester/Tester.h"
#include "systemspy/SystemSpy.h"
#include "HyvesDesktopApplication.h"
#include "SystemTrayIcon.h"
#include "Util.h"

namespace SystemTrayIcon {

SystemTrayIcon *SystemTrayIcon::s_instance = 0;

struct SystemTrayIcon::Private {
	Logger::Logger *log;
	
	QAction *quitAction;
	
	QSystemTrayIcon *trayIcon;
	QMenu *trayIconMenu;
	
	QString icon;
	QIcon highlightIcon;
	QIcon activeIcon;
	QIcon inactiveIcon;
	
	QMap<QString, QMenu *> subMenus;
	QMap<QString, QAction *> pluginActions;
	QMap<QString, QActionGroup *> actionGroups;
	
	Private() :
		log(0),
		quitAction(0),
		trayIcon(0),
		trayIconMenu(0) {
	}
	
	~Private() {
		
		delete trayIconMenu;
		delete log;
	}
};

SystemTrayIcon *SystemTrayIcon::instance() {
	
	if (s_instance == 0) {
		s_instance = new SystemTrayIcon();
	}
	
	return s_instance;
}

void SystemTrayIcon::destroy() {
	
	delete s_instance;
	s_instance = 0;
}

void SystemTrayIcon::addAction(const QString &menu, const QString &key, const QString &menuItem, const QString &group) {
	m_d->log->debug(QString("menu %1, menuItem %2, key %3").arg(menu).arg(menuItem).arg(key));
	
	if (m_d->pluginActions.contains(menu + "/" + key)) {
		// Don't add a menu item that already exists
		return;
	}
	
	QMenu *subMenu = 0;
	if (!m_d->subMenus.contains(menu)) {
		subMenu = new QMenu(menu, m_d->trayIconMenu);
		m_d->subMenus[menu] = subMenu;
		m_d->trayIconMenu->insertMenu(m_d->trayIconMenu->actions().first(), subMenu);
	} else {
		subMenu = m_d->subMenus[menu];
	}
	
	QAction *action = new QAction(menuItem, subMenu);
	action->setObjectName(menu + '/' + key);
	m_d->pluginActions[action->objectName()] = action;
	
	if (!group.isEmpty()) {
		if (!m_d->actionGroups.contains(group)) {
			m_d->actionGroups[group] = new QActionGroup(this);
			m_d->actionGroups[group]->setExclusive(true);
		}
		
		action->setActionGroup(m_d->actionGroups[group]);
		action->setCheckable(true);
	}
	
	subMenu->addAction(action);
	
	connect(action, SIGNAL(triggered()), this, SLOT(onMenuClick()));
}

void SystemTrayIcon::removePluginActions() {
	
	foreach(QString key, m_d->pluginActions.keys()) {
		removePluginAction("", key);
	}
}

void removeAction(QMenu *subMenu, QAction *pluginAction, QMenu *trayIconMenu) {
	
	if (subMenu->actions().contains(pluginAction)) {
		subMenu->removeAction(pluginAction);
		delete pluginAction; // XXX: fix this when we also put actions in toolbars
		if (subMenu->actions().isEmpty()) {
			trayIconMenu->removeAction(subMenu->menuAction());
		}
	}
}

void SystemTrayIcon::removePluginAction(const QString &menu, const QString &key) {
	
	if (!m_d->pluginActions.contains(key)) { 
		return;
	}
	
	QAction *pluginAction = m_d->pluginActions[key];
	
	if (m_d->subMenus.contains(menu)) {
		removeAction(m_d->subMenus[menu], pluginAction, m_d->trayIconMenu);
	} else {
		foreach(QMenu *menu, m_d->subMenus.values()) {
			removeAction(menu, pluginAction, m_d->trayIconMenu);
		}
	}
	m_d->pluginActions.remove(key);
}

void SystemTrayIcon::setSelectedAction(const QString &menu, const QString &key) {
	
	if (!m_d->subMenus.contains(menu)) {
		m_d->log->warning("Trying to select action in non-existing menu: " + menu);
		return;
	}
	
	QString actionName = menu + "/" + key;
	if (!m_d->pluginActions.contains(actionName)) {
		m_d->log->warning("Trying to select non-existing action: " + actionName);
		return;
	}
	
	m_d->pluginActions[actionName]->setChecked(true);
}

QString SystemTrayIcon::trayIconGeometryJSON() {
	
	return Util::variantToJSType(m_d->trayIcon->geometry());
}

const QString SystemTrayIcon::icon() const {
	
	return m_d->icon;
}

void SystemTrayIcon::setIcon(const QString &icon) {
	m_d->log->debug(QString("Setting icon: %1").arg(icon));
	
	m_d->icon = icon;
	
	if (icon == "highlight") {
		m_d->trayIcon->setIcon(m_d->highlightIcon);
	} else if (icon == "active") {
		m_d->trayIcon->setIcon(m_d->activeIcon);
	} else if (icon == "inactive") {
		m_d->trayIcon->setIcon(m_d->inactiveIcon);
	} else {
		m_d->trayIcon->setIcon(QIcon(icon));
	}
	
#ifndef Q_WS_X11
	m_d->trayIcon->hide();
	m_d->trayIcon->show();
#endif
}

SystemTrayIcon::SystemTrayIcon() :
	QObject(),
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("SystemTrayIcon");
	
	m_d->quitAction = new QAction(tr("&Quit"), this);
	connect(m_d->quitAction, SIGNAL(triggered()), app(), SLOT(quit()));
	
	m_d->trayIconMenu = new QMenu();
	
	m_d->trayIconMenu->addSeparator();
	m_d->trayIconMenu->addAction(m_d->quitAction);
	
	m_d->trayIcon = new QSystemTrayIcon(this);
	m_d->trayIcon->setContextMenu(m_d->trayIconMenu);
	
#ifdef Q_WS_MAC
	m_d->highlightIcon.addFile(":/data/appicon_16.png", QSize(16, 16), QIcon::Normal);
	m_d->activeIcon.addFile(":/data/appicon_16_mac_inactive.png", QSize(16, 16), QIcon::Normal);
	m_d->inactiveIcon.addFile(":/data/appicon_16_mac_inactive.png", QSize(16, 16), QIcon::Normal);
#else
	m_d->highlightIcon.addFile(":/data/appicon_16_notify.png", QSize(16, 16), QIcon::Normal);
	m_d->highlightIcon.addFile(":/data/appicon_22_notify.png", QSize(22, 22), QIcon::Normal);
	m_d->activeIcon.addFile(":/data/appicon_16.png", QSize(16, 16), QIcon::Normal);
	m_d->activeIcon.addFile(":/data/appicon_22_active.png", QSize(22, 22), QIcon::Normal);
	m_d->inactiveIcon.addFile(":/data/appicon_16_inactive.png", QSize(16, 16), QIcon::Normal);
	m_d->inactiveIcon.addFile(":/data/appicon_22_inactive.png", QSize(22, 22), QIcon::Normal);
#endif
	
	setIcon("inactive");
	
#ifndef Q_WS_MAC
	m_d->trayIcon->setToolTip(tr("%1 -- Always in touch with your friends").arg(HD_PRODUCT_NAME));
#endif
	
	m_d->trayIcon->show();
	
	connect(SystemSpy::SystemSpy::instance(), SIGNAL(networkRecovered()), SLOT(updateIcon()));
	connect(SystemSpy::SystemSpy::instance(), SIGNAL(networkDown()), SLOT(updateIcon()));
	
	connect(m_d->trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
	        SLOT(onActivated(QSystemTrayIcon::ActivationReason)));
	connect(m_d->trayIcon, SIGNAL(messageClicked()), SIGNAL(messageClicked()));
	
	connect(Plugger::Plugger::instance(), SIGNAL(registerEntryPoint(const QString &, const QString &)),
	        SLOT(registerPluginEntryPoint(const QString &, const QString &)));
	
	Extender::Extender::instance()->registerObject("systemTrayIcon", this);
}

SystemTrayIcon::~SystemTrayIcon() {
	
	Extender::Extender::instance()->unregisterObject("systemTrayIcon");
	
	delete m_d;
}

void SystemTrayIcon::onMenuClick() {
	
	emit menuOptionClicked(sender()->objectName());
}

void SystemTrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason) {
	
#ifndef Q_WS_MAC
	switch(reason) {
		case QSystemTrayIcon::DoubleClick:
			Plugger::Plugger::instance()->runPlugin("kwekker", QStringList());
			break;
		case QSystemTrayIcon::MiddleClick:
		case QSystemTrayIcon::Context:
#ifdef Q_WS_X11
			m_d->trayIconMenu->exec(QCursor::pos());
			break;
#endif // Q_WS_X11
		case QSystemTrayIcon::Trigger:
		default:
			break;
	};
#endif // !Q_WS_MAC
}

void SystemTrayIcon::onPluginEntryPointClicked() {
	
	QObject *action = sender();
	Q_ASSERT(action);
	Q_ASSERT(action->objectName().startsWith("plugin/"));
	Plugger::Plugger::instance()->runPlugin(action->objectName().mid(7), QStringList());
}

void SystemTrayIcon::registerPluginEntryPoint(const QString &pluginName, const QString &displayName) {
	
	QAction *action = new QAction(displayName, m_d->trayIconMenu);
	action->setObjectName("plugin/" + pluginName);
	m_d->trayIconMenu->insertAction(m_d->trayIconMenu->actions().first(), action);
	connect(action, SIGNAL(triggered()), this, SLOT(onPluginEntryPointClicked()));
}

void SystemTrayIcon::showMessage(const QString &title, const QString &message, QSystemTrayIcon::MessageIcon icon) {
	
	m_d->trayIcon->showMessage(title, message, icon);
}

void SystemTrayIcon::updateIcon() {
	
	SystemSpy::NetworkState state = SystemSpy::SystemSpy::instance()->networkState();
	
	switch(state) {
		case SystemSpy::NetworkOk:
			m_d->trayIcon->setIcon(m_d->activeIcon);
			break;
		case SystemSpy::BrokenDown:
		default:
			m_d->trayIcon->setIcon(m_d->inactiveIcon);
			break;
	}
}

} // namespace SystemTrayIcon
