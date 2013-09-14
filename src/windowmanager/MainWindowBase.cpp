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

#include <QDebug>
#include <QEvent>
#include <QAction>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QMenuBar>
#include <QMenu>
#include <QString>
#include <QDebug>

#include "config.h"
#include "logger/Logger.h"
#include "settingsmanager/SettingsManager.h"
#include "AboutDialog.h"
#include "HyvesDesktopApplication.h"
#include "MainWindowBase.h"
#include "Util.h"
#include "WindowManager.h"
#include "WindowInfo.h"

namespace WindowManager {

struct MainWindowBase::Private {
	
	Private() :
		log(0),
		menuBar(0),
		fileMenu(0),
		editMenu(0),
//		windowMenu(0),
		helpMenu(0) {
		
		SettingsManager::SettingsManager *settingsManager = SettingsManager::SettingsManager::instance();
		onlineHelpUrl = "http://www." + settingsManager->baseServer() + "/help";
		privacyUrl = "http://www." + settingsManager->baseServer() + "/index.php?l1=ut&l2=pr&l3=hm";
		giveFeedbackUrl = "http://www." + settingsManager->baseServer() + "/help/feedback?faqid=816&topicid=301";
		userConditionsUrl = "http://www." + settingsManager->baseServer() + "/index.php?l1=ut&l2=gv&l3=hm";
	}
	
	~Private() {
		
		delete log;
	}
	
	Logger::Logger *log;
	
	QMenuBar *menuBar;
	QMenu *fileMenu;
	QMenu *editMenu;
	//QMenu *windowMenu;
	QMenu *helpMenu;
	
	QString pluginTitle;
	
	QUrl onlineHelpUrl;
	QUrl privacyUrl;
	QUrl giveFeedbackUrl;
	QUrl userConditionsUrl;
};
	
MainWindowBase::MainWindowBase(const QString &pluginTitle, QWidget *parent, Qt::WindowFlags flags) : 
	QMainWindow(parent, flags),
	WindowBase(),
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("MainWindowBase");
	
	m_d->pluginTitle = pluginTitle;
	
	QMainWindow::setAttribute(Qt::WA_DeleteOnClose, false);
	
	m_d->menuBar = menuBar();
	populateMenuBar();
	
	QAction *closeAction = new QAction(tr("&Close"), this);
	closeAction->setShortcut(QKeySequence::Close);
	connect(closeAction, SIGNAL(triggered()), SLOT(hide()));
	addAction(closeAction);
}

MainWindowBase::~MainWindowBase() {
	
	delete m_d;
}

QMenu *MainWindowBase::menu(const QString &title) {
	
	QMenu *menu = new QMenu(title);
	m_d->menuBar->insertMenu(m_d->helpMenu->menuAction(), menu);
	
	return menu;
}

void MainWindowBase::populateMenuBar() {
	
	m_d->fileMenu = new QMenu(m_d->pluginTitle);
	m_d->fileMenu->addAction(tr("&Sign out"), this, SLOT(signOut()));
	m_d->fileMenu->addSeparator();
	m_d->fileMenu->addAction(tr("&Close"), this, SLOT(hide()));
	m_d->fileMenu->addAction(tr("&Quit"), app(), SLOT(quit()));
	m_d->menuBar->addMenu(m_d->fileMenu);
	
	/*
	m_d->editMenu = new QMenu(tr("&Edit"));
	m_d->menuBar->addMenu(m_d->editMenu);
	*/
	
//	m_d->windowMenu = new QMenu(tr("&Window"));
//	m_d->menuBar->addMenu(m_d->windowMenu);
	
	m_d->helpMenu = new QMenu("&Help");
	
	m_d->helpMenu->addAction(tr("&Introduction"), this, SLOT(introduction()));
	m_d->helpMenu->addAction(tr("&Online Help"), this, SLOT(onlineHelp()));
	m_d->helpMenu->addAction(tr("&Privacy"), this, SLOT(privacy()));
	m_d->helpMenu->addAction(tr("Give &Feedback"), this, SLOT(giveFeedback()));
	m_d->helpMenu->addAction(tr("&User Conditions"), this, SLOT(userConditions()));
	
	m_d->helpMenu->addSeparator();
	
	QAction *aboutAction = m_d->helpMenu->addAction(tr("&About %1").arg(HD_PRODUCT_NAME), this, SLOT(about()));
	aboutAction->setMenuRole(QAction::AboutRole);
	
	m_d->menuBar->addMenu(m_d->helpMenu);
}

void MainWindowBase::introduction() {
	
	m_d->log->usage("IntroductionMenuItemClicked");
	showLandingPage();
}	

void MainWindowBase::onlineHelp() {
	
	m_d->log->usage("OnlineHelpMenuItemClicked");
	QDesktopServices::openUrl(m_d->onlineHelpUrl);
}

void MainWindowBase::privacy() {
	
	m_d->log->usage("PrivacyMenuItemClicked");
	QDesktopServices::openUrl(m_d->privacyUrl);
}

void MainWindowBase::giveFeedback() {
	
	m_d->log->usage("GiveFeedbackMenuItemClicked");
	QDesktopServices::openUrl(m_d->giveFeedbackUrl);
}

void MainWindowBase::userConditions() {
	
	m_d->log->usage("UserConditionsMenuItemClicked");
	QDesktopServices::openUrl(m_d->userConditionsUrl);
}

void MainWindowBase::about() {
	
	m_d->log->usage("AboutMenuItemClicked");
	AboutDialog aboutDialog(this);
	aboutDialog.exec();
}

void MainWindowBase::showLandingPage() {
	
	WindowManager *windowManager = WindowManager::instance();
	WindowInfo windowInfo = windowManager->windowInfo(windowId());
	
	QVariantMap arguments;
	arguments["caption"] = tr("Introduction");
	arguments["visible"] = true;
	arguments["width"] = 800 + 2 * windowManager->frameWidth();
	arguments["height"] = 600 + windowManager->titleBarHeight() + windowManager->frameHeight();
	arguments["left"] = (windowInfo.availableGeometry().width() / 2) - (arguments["width"].toInt() / 2);
	arguments["top"] = (windowInfo.availableGeometry().height() / 2) - (arguments["height"].toInt() / 2);
	arguments["minimumSize"] = true;
	arguments["openLinksInUserBrowser"] = false;
	
	int windowId = windowManager->createWindow("landingPage", Util::variantMapToJSON(arguments));
	windowManager->windowExecute(windowId, "navigate", "url=http://localhost/statics/intro/intro1.html");
	windowManager->windowExecute(windowId, "bringToFront");
}

void MainWindowBase::updateWindowMenu() {

//	m_d->windowMenu->clear();
//	foreach (QWidget *window, WindowManager::instance()->windows()) {
//		qDebug() << window->windowTitle();
//		//QAction *action = m_d->windowMenu->addAction();
//	}

}

} // namspace WindowManager
