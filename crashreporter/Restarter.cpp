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

#include "config.h"
#include "Restarter.h"

#include <cstdlib>

#include <QtCore>
#include <QtGui>

#ifdef Q_WS_WIN
#include <cstring>
#include <windows.h>

/**
 * Native Win32 method for starting a process. This is required in order to
 * launch the installer with User Account Control enabled.
 * 
 * @param path Path to the process to start.
 * @param parameters Parameters for the process.
 * @return @c true if the process is started successfully, @c false otherwise.
 */
bool startProcess(char *path, char *parameters = 0) {
	
	Q_ASSERT(path != 0);
	
	SHELLEXECUTEINFO info;
	memset(&info, '\0', sizeof(info));
	
	info.cbSize = sizeof(info);
	info.fMask = 0;
	info.hwnd = 0;
	info.lpVerb = TEXT("open");
	info.lpFile = path;
	info.lpParameters = parameters;
	info.lpDirectory = 0;
	info.nShow = SW_SHOWNORMAL;
	return ShellExecuteEx(&info);
}

void Restarter::restart() {
	
	if (!startProcess("HyvesDesktop")) {
		QMessageBox::warning(0, HD_PRODUCT_NAME,
			QObject::tr("Could not restart %1. Please try to restart %1 manually.").arg(HD_PRODUCT_NAME)
		);
	}
}

#else // Q_WS_WIN

void Restarter::restart() {
	
#ifdef Q_WS_MAC
	// We put the crash reporter bundle inside the hyves desktop bundle
	QDir bundleDir(qApp->applicationDirPath());
	
	bundleDir.cdUp();
	bundleDir.cdUp();
	bundleDir.cdUp();
	
	QString restartCommand;
	if (QDir(QString(bundleDir.absolutePath() + "/Hyves Desktop.app")).exists()) {
		restartCommand = QString("open \"") + QString(bundleDir.absolutePath() + "/Hyves Desktop.app\"");
	} else {
		restartCommand = QString("open \"") + QString(bundleDir.absolutePath() + "/HyvesDesktop.app\"");
	}
#else // Q_WS_MAC
	QString restartCommand = "sh -c \"" + qApp->applicationDirPath().replace(' ', "\\ ") + "/HyvesDesktop.shell \"";
#endif // Q_WS_MAC
	
	QProcess restartProcess;
	if (!restartProcess.startDetached(restartCommand)) {
		QMessageBox::warning(0, HD_PRODUCT_NAME,
			QObject::tr("Could not restart %1. Please try to restart %1 manually.").arg(HD_PRODUCT_NAME)
		);
	}
}

#endif // Q_WS_WIN
