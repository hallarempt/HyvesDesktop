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

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPixmap>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QUiLoader>

#include "logger/Logger.h"
#include "historykeeper/HistoryKeeper.h"
#include "settingsmanager/SettingsManager.h"
#include "windowmanager/WindowManager.h"
#include "SettingsDialog.h"

namespace Kwekker {

struct SettingsDialog::Private {
	Logger::Logger *log;
	
	QListWidget *contentsWidget;
	QStackedWidget *pagesWidget;
	QDialogButtonBox *buttonBox;
	
	QWidget *generalPage;
	QWidget *notificationsPage;
	
	Private() :
		log(0),
		generalPage(0),
		notificationsPage(0) {
	}
	
	~Private() {
		
		delete log;
	}
};

SettingsDialog::SettingsDialog(QWidget *parent) :
	QDialog(parent),
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("SettingsDialog");
	m_d->log->usage("SettingsDialogOpened");
	
	setWindowTitle(tr("Settings"));
	createLayout();
	
	m_d->generalPage = createPage(tr("General"), ":/plugins/kwekker/preferences%1",
	                              ":/plugins/kwekker/GeneralSettingsPage.ui");
	m_d->notificationsPage = createPage(tr("Sound & Notifications"), ":/plugins/kwekker/sound%1",
	                                    ":/plugins/kwekker/NotificationsSettingsPage.ui");
	
	loadSettings();
	
	connect(m_d->generalPage->findChild<QPushButton *>("deleteHistoryButton"), SIGNAL(clicked()), SLOT(deleteHistory()));
	connect(this, SIGNAL(accepted()), SLOT(saveSettings()));
}

SettingsDialog::~SettingsDialog() {
	
	delete m_d;
}

void SettingsDialog::createLayout() {
	
	m_d->contentsWidget = new QListWidget(this);
	m_d->contentsWidget->setViewMode(QListView::IconMode);
	m_d->contentsWidget->setIconSize(QSize(64, 64));
	m_d->contentsWidget->setMovement(QListView::Static);
	m_d->contentsWidget->setMaximumWidth(100);
	m_d->contentsWidget->setCurrentRow(0);
	m_d->contentsWidget->setUniformItemSizes(true);
	
	m_d->pagesWidget = new QStackedWidget(this);
	
	QFrame *line = new QFrame(this);
	line->setFrameShadow(QFrame::Sunken);
	line->setLineWidth(1);
	line->setMidLineWidth(0);
	
	m_d->buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	
	connect(m_d->buttonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(m_d->buttonBox, SIGNAL(rejected()), SLOT(reject()));
	
	QHBoxLayout *layout2 = new QHBoxLayout();
	layout2->addWidget(m_d->contentsWidget);
	layout2->addWidget(m_d->pagesWidget);
	
	QVBoxLayout *layout1 = new QVBoxLayout();
	layout1->addLayout(layout2);
	layout1->addWidget(line);
	layout1->addWidget(m_d->buttonBox);
	
	setLayout(layout1);
	
	connect(m_d->contentsWidget, SIGNAL(currentRowChanged(int)),
	        m_d->pagesWidget, SLOT(setCurrentIndex(int)));
}

QWidget *SettingsDialog::createPage(const QString &title, const QString &iconResource, const QString &uiResource) {
	
	QFile uiFile(uiResource);
	if (!uiFile.open(QFile::ReadOnly)) {
		m_d->log->debug(QString("Cannot open UI resource %1.").arg(uiResource));
		return 0;
	}
	
	QFont font;
	font.setBold(true);
	
	QListWidgetItem *item = new QListWidgetItem(m_d->contentsWidget);
	item->setIcon(QIcon(iconResource.arg("64")));
	item->setText(title);
	item->setTextAlignment(Qt::AlignHCenter); 
	item->setFont(font);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	
	QRect visualItemRect = m_d->contentsWidget->visualItemRect(item);
	if (visualItemRect.width() + 2 * m_d->contentsWidget->frameWidth() > m_d->contentsWidget->maximumWidth()) {
		m_d->contentsWidget->setFixedWidth(visualItemRect.width() + 2 * m_d->contentsWidget->frameWidth());
	}
	
	QWidget *page = new QWidget(this);
	
	QLabel *titleLabel = new QLabel(page);
	titleLabel->setText(title);
	titleLabel->setFont(font);
	
	QLabel *iconLabel = new QLabel(page);
	iconLabel->setPixmap(QPixmap(iconResource.arg("22")));
	
	QUiLoader uiLoader;
	QWidget *uiWidget = uiLoader.load(&uiFile, page);
	
	QHBoxLayout *layout2 = new QHBoxLayout();
	layout2->setSpacing(4);
	layout2->addWidget(titleLabel);
	layout2->addStretch();
	layout2->addWidget(iconLabel);
	
	QVBoxLayout *layout1 = new QVBoxLayout();
	layout1->addLayout(layout2);
	layout1->addWidget(uiWidget);
	layout1->addStretch();
	
	page->setLayout(layout1);
	m_d->pagesWidget->addWidget(page);
	
	return uiWidget;
}

void SettingsDialog::loadSettings() {
	
	SettingsManager::SettingsManager *settingsManager = SettingsManager::SettingsManager::instance();
	
	m_d->generalPage->findChild<QCheckBox *>("startAutomaticallyCheckbox")->setChecked(settingsManager->boolValue("General/Autostart", true));
	m_d->generalPage->findChild<QCheckBox *>("disconnectOtherLocationsCheckbox")->setChecked(!settingsManager->boolValue("General/allowMultipleClients", false));
	m_d->generalPage->findChild<QCheckBox *>("enableLocalHistoryCheckbox")->setChecked(settingsManager->boolValue("General/enableLocalHistory", true));
	m_d->generalPage->findChild<QCheckBox *>("alwaysDisplayOfflineFriendsCheckbox")->setChecked(settingsManager->boolValue("General/showOfflineFriends", false));
	m_d->generalPage->findChild<QCheckBox *>("submitUsageStatsCheckbox")->setChecked(settingsManager->boolValue("Statistics/submitUsageStats", true));
	
	m_d->notificationsPage->findChild<QCheckBox *>("friendOnlineSoundCheckbox")->setChecked(settingsManager->boolValue("Notifications/newOnlineSound", false));
	m_d->notificationsPage->findChild<QCheckBox *>("newConversationSoundCheckbox")->setChecked(settingsManager->boolValue("Notifications/newSessionSound", true));
	m_d->notificationsPage->findChild<QCheckBox *>("newNotificationSoundCheckbox")->setChecked(settingsManager->boolValue("Notifications/headlineSound", true));
	m_d->notificationsPage->findChild<QCheckBox *>("friendOnlinePopupCheckbox")->setChecked(settingsManager->boolValue("Notifications/newOnlineAlert", true));
	m_d->notificationsPage->findChild<QCheckBox *>("newConversationPopupCheckbox")->setChecked(settingsManager->boolValue("Notifications/newSessionAlert", true));
	m_d->notificationsPage->findChild<QCheckBox *>("newNotificationPopupCheckbox")->setChecked(settingsManager->boolValue("Notifications/headlineAlert", true));
}

void SettingsDialog::saveSettings() {
	
	SettingsManager::SettingsManager *settingsManager = SettingsManager::SettingsManager::instance();
	
	settingsManager->setValue("General/Autostart", m_d->generalPage->findChild<QCheckBox *>("startAutomaticallyCheckbox")->isChecked(), true);
	settingsManager->setValue("General/allowMultipleClients", !m_d->generalPage->findChild<QCheckBox *>("disconnectOtherLocationsCheckbox")->isChecked(), false);
	settingsManager->setValue("General/enableLocalHistory", m_d->generalPage->findChild<QCheckBox *>("enableLocalHistoryCheckbox")->isChecked(), true);
	settingsManager->setValue("General/showOfflineFriends", m_d->generalPage->findChild<QCheckBox *>("alwaysDisplayOfflineFriendsCheckbox")->isChecked(), false);
	settingsManager->setValue("Statistics/submitUsageStats", m_d->generalPage->findChild<QCheckBox *>("submitUsageStatsCheckbox")->isChecked(), true);
	
	settingsManager->setValue("Notifications/newOnlineSound", m_d->notificationsPage->findChild<QCheckBox *>("friendOnlineSoundCheckbox")->isChecked(), false);
	settingsManager->setValue("Notifications/newSessionSound", m_d->notificationsPage->findChild<QCheckBox *>("newConversationSoundCheckbox")->isChecked(), true);
	settingsManager->setValue("Notifications/headlineSound", m_d->notificationsPage->findChild<QCheckBox *>("newNotificationSoundCheckbox")->isChecked(), true);
	settingsManager->setValue("Notifications/newOnlineAlert", m_d->notificationsPage->findChild<QCheckBox *>("friendOnlinePopupCheckbox")->isChecked(), true);
	settingsManager->setValue("Notifications/newSessionAlert", m_d->notificationsPage->findChild<QCheckBox *>("newConversationPopupCheckbox")->isChecked(), true);
	settingsManager->setValue("Notifications/headlineAlert", m_d->notificationsPage->findChild<QCheckBox *>("newNotificationPopupCheckbox")->isChecked(), true);
}

void SettingsDialog::deleteHistory() {
	
	WindowManager::WindowManager *windowManager = WindowManager::WindowManager::instance();
	
	if (!windowManager->askConfirmation(this, tr("Delete history"),
	                                    tr("This will delete all your saved histories. Are you sure you want to delete the history?"))) {
		return;
	}
	
	HistoryKeeper::HistoryKeeper::instance()->clear();
	
	m_d->generalPage->findChild<QPushButton *>("deleteHistoryButton")->setEnabled(false);
}

} // namespace Kwekker
