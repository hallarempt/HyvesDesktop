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

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMovie>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>

#include "config.h"
#include "updater/Updater.h"
#include "AboutDialog.h"

struct AboutDialog::Private {
	QVBoxLayout *verticalLayout;
	QLabel *logoLabel;
	QLabel *versionLabel;
	QHBoxLayout *horizontalLayout;
	QPushButton *checkForUpdatesButton;
	QMovie *busyMovie;
	QLabel *busyLabel;
	QLabel *statusLabel;
	QSpacerItem *horizontalSpacer;
	QLabel *copyrightLabel;
	QDialogButtonBox *buttonBox;
};

AboutDialog::AboutDialog(QWidget *parent) :
	QDialog(parent),
	m_d(new Private()) {
	
	setupUi();
	
	Updater::Updater *updater = Updater::Updater::instance();
	connect(updater, SIGNAL(downloadingUpdate()), SLOT(downloadingUpdate()));
	connect(updater, SIGNAL(updateAvailable()), SLOT(updateAvailable()));
	connect(updater, SIGNAL(noUpdateAvailable()), SLOT(noUpdateAvailable()));
	connect(updater, SIGNAL(errorDownloadingUpdate()), SLOT(errorDownloadingUpdate()));
}

AboutDialog::~AboutDialog() {
	
	delete m_d;
}

void AboutDialog::setupUi() {
	
	m_d->logoLabel = new QLabel(this);
	m_d->logoLabel->setPixmap(QPixmap(":/data/about_bg.png"));
	m_d->logoLabel->setAlignment(Qt::AlignCenter);
	
	m_d->versionLabel = new QLabel(this);
	m_d->versionLabel->setWordWrap(true);
	
	m_d->checkForUpdatesButton = new QPushButton(this);
	
	m_d->busyMovie = new QMovie(this);
	m_d->busyMovie->setFileName(":/data/bundle/images/ajax_action.gif");
	m_d->busyLabel = new QLabel(this);
	m_d->busyLabel->setMovie(m_d->busyMovie);
	m_d->busyLabel->setVisible(false);
	
	m_d->statusLabel = new QLabel(this);
	
	m_d->horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	
	m_d->copyrightLabel = new QLabel(this);
	m_d->copyrightLabel->setWordWrap(true);
	
	m_d->buttonBox = new QDialogButtonBox(this);
	m_d->buttonBox->setOrientation(Qt::Horizontal);
	m_d->buttonBox->setStandardButtons(QDialogButtonBox::Ok);
	
	m_d->horizontalLayout = new QHBoxLayout();
	m_d->horizontalLayout->addWidget(m_d->checkForUpdatesButton);
	m_d->horizontalLayout->addWidget(m_d->busyLabel);
	m_d->horizontalLayout->addWidget(m_d->statusLabel);
	m_d->horizontalLayout->addItem(m_d->horizontalSpacer);
	
	m_d->verticalLayout = new QVBoxLayout(this);
	m_d->verticalLayout->addWidget(m_d->logoLabel);
	m_d->verticalLayout->addWidget(m_d->versionLabel);
	m_d->verticalLayout->addLayout(m_d->horizontalLayout);
	m_d->verticalLayout->addWidget(m_d->copyrightLabel);
	m_d->verticalLayout->addWidget(m_d->buttonBox);
	
	retranslateUi();
	
	connect(m_d->checkForUpdatesButton, SIGNAL(clicked()), SLOT(checkForUpdates()));
	connect(m_d->buttonBox, SIGNAL(accepted()), SLOT(accept()));
	
	QMetaObject::connectSlotsByName(this);
}

void AboutDialog::retranslateUi() {
	
	setWindowTitle(HD_PRODUCT_NAME);
	
	m_d->versionLabel->setText(QString("<p><b>%1</b></p>\n").arg(HD_PRODUCT_NAME) + 
		tr("<p>Version %1</p>").arg(HD_VERSION.toString())
	);
	m_d->checkForUpdatesButton->setText(tr("&Check for updates"));
	m_d->copyrightLabel->setText(tr("<br>\n"
		"<p>Copyright (C) 2008-2009 Hyves. All rights reserved.</p>\n"
		"<p>This program is provided as AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.</p>"
	));
}

void AboutDialog::checkForUpdates() {
	
	Updater::Updater::instance()->checkForUpdates();
	
	m_d->checkForUpdatesButton->setEnabled(false);
	m_d->busyMovie->start();
	m_d->busyLabel->setVisible(true);
	m_d->statusLabel->setVisible(true);
	m_d->statusLabel->setText(tr("Checking for updates..."));
}

void AboutDialog::downloadingUpdate() {
	
	m_d->checkForUpdatesButton->setEnabled(false);
	m_d->busyMovie->start();
	m_d->busyLabel->setVisible(true);
	m_d->statusLabel->setVisible(true);
	m_d->statusLabel->setText(tr("Downloading update..."));	
}

void AboutDialog::updateAvailable() {
	
	m_d->checkForUpdatesButton->setEnabled(true);
	m_d->busyMovie->stop();
	m_d->busyLabel->setVisible(false);
	m_d->statusLabel->setVisible(false);
}

void AboutDialog::noUpdateAvailable() {
	
	m_d->checkForUpdatesButton->setEnabled(true);
	m_d->busyMovie->stop();
	m_d->busyLabel->setVisible(false);
	m_d->statusLabel->setVisible(true);
	m_d->statusLabel->setText(tr("No update available."));	
}

void AboutDialog::errorDownloadingUpdate() {
	
	m_d->checkForUpdatesButton->setEnabled(true);
	m_d->busyMovie->stop();
	m_d->busyLabel->setVisible(false);
	m_d->statusLabel->setVisible(true);
	m_d->statusLabel->setText(tr("Error downloading update."));	
}
