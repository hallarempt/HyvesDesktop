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

#include <QtGui>
#include <QtCore>

#include "config.h"
#include "CrashReporter.h"
#include "Restarter.h"

int main(int argc, char *argv[]) {
	
	// Must be the same as the hyves desktop so we can read settings
	// and access the data directory.
	QCoreApplication::setApplicationName(HD_PRODUCT_NAME);
	QCoreApplication::setOrganizationName(HD_ORGANIZATION_NAME);
	QCoreApplication::setOrganizationDomain(HD_PRODUCT_URL);
	QCoreApplication::setApplicationVersion(HD_VERSION);
	
	QApplication app(argc, argv);
	
	QIcon appIcon;
	appIcon.addFile(":/data/appicon_16_r.png", QSize(16, 16), QIcon::Normal);
	appIcon.addFile(":/data/appicon_22_r.png", QSize(22, 22), QIcon::Normal);
	appIcon.addFile(":/data/appicon_24_r.png", QSize(24, 24), QIcon::Normal);
	appIcon.addFile(":/data/appicon_32_r.png", QSize(32, 32), QIcon::Normal);
	appIcon.addFile(":/data/appicon_64_r.png", QSize(64, 64), QIcon::Normal);
	appIcon.addFile(":/data/appicon_128_r.png", QSize(128, 128), QIcon::Normal);
	app.setWindowIcon(appIcon);

	QSettings settings(HD_DOMAIN_NAME, HD_PRODUCT_NAME);
	QString language = settings.value("language", "NLD").toString();
	if (language == "NLD") {
                QDir dir = QDir(app.applicationDirPath());
                QStringList nameFilters;
                nameFilters << QString("*_nl.qm");
                dir.setNameFilters(nameFilters);
                QFileInfoList languageFiles = dir.entryInfoList();
                foreach (QFileInfo languageFile, languageFiles) {
                        QTranslator *translator = new QTranslator();
                        translator->load(languageFile.completeBaseName(), dir.path());
                        app.installTranslator(translator);
                }

	}
	
	QStringList arguments = app.arguments();
	
	// Something went wrong, whatever: we restart the Hyves Desktop
	if (arguments.size() != 3)  {
		Restarter::restart();
		return 0;
	}
	
	QString dumpPath = arguments[1];
	QString dumpId = arguments[2];
	
	CrashReporter reporter(dumpPath, dumpId);
	reporter.show();
	
	return app.exec();
}
