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

#include <cstring>

#include <QtGui>
#include <QtCore>

#include "config.h"
#include "logger/Logger.h"
#include "player/Player.h"
#include "updater/Updater.h"
#include "HyvesDesktopApplication.h"
#ifndef BUILD_64BIT
#include "CrashHandler.h"
#endif

QTextStream cout(stdout, QIODevice::WriteOnly);

void setupLibraryPaths(char *argv[]) {
	
	QString executablePath = QFileInfo(argv[0]).path();
	QStringList paths;
	paths << "."
	      << executablePath + "/plugins"
	      << executablePath + "/../plugins";
#ifndef Q_WS_MAC
	paths << QCoreApplication::libraryPaths();
#endif
	QCoreApplication::setLibraryPaths(paths);
}

/**
 * Main starting point for the Hyves Desktop client. All other code goes
 * into src.
 */
int main(int argc, char *argv[]) {
	
#ifndef BUILD_64BIT
#ifndef NO_BREAKPAD
	CrashHandler::CrashHandler crashHandler;
#endif
#endif
	
	if (argc > 1 && strcmp(argv[1], "--version") == 0) {
		cout << HD_PRODUCT_NAME << " " << HD_VERSION.toString() << endl
		     << "Copyright (C) 2008-2009 Hyves. All rights reserved." << endl;
		return 0;
	}

	
	// For use in scripts
	if (argc > 1 && strcmp(argv[1], "--version-number") == 0) {
		cout << HD_VERSION.toString();
		return 0;
	}
	
	Q_INIT_RESOURCE(HyvesDesktop);
	
	setupLibraryPaths(argv);
	
	// A per-user unique string, without /, because QLocalServer cannot use names with a / in it
	QString key = "HyvesDesktop" +
	              QDesktopServices::storageLocation(QDesktopServices::HomeLocation).replace("/", "_");
	key = key.replace(":", "_").replace("\\","_");
	
	if (getenv("BASE_SERVER")) {
		key += getenv("BASE_SERVER");
	}	
	
	HyvesDesktopApplication *application = new HyvesDesktopApplication(argc, argv, key);

        QStringList arguments = application->arguments();
	
	if (arguments.contains("--test-sound")) {
		Player::Player::instance()->playSound(":/plugins/kwekker/sounds/newsession");
	}
	
	if (arguments.contains("--crash")) {
		int* x = 0;
		*x = 1;
	}
	
	int exitCode = application->exec();
	
	if (exitCode == Updater::InstallUpdateExitCode) {
		Updater::Updater::instance()->installAvailableUpdate();
	}
	
	delete application;
	return exitCode;
}
