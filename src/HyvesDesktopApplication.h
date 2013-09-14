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

#ifndef HYVESDESKTOPAPPLICATION_H
#define HYVESDESKTOPAPPLICATION_H

#include "hyveslib_export.h"
#include "jabber/Jabber.h"
#include <QtSingleApplication>

class QString;

class HYVESLIB_EXPORT HyvesDesktopApplication : public QtSingleApplication {
	
	Q_OBJECT
	
	public:
		HyvesDesktopApplication(int &argc, char *argv[], const QString &uniqueKey);
		virtual ~HyvesDesktopApplication();
		
	public slots:
		static void quit();
		void setNextStatsUpload();
		void sendArgs(const QStringList &args);
		
	private slots:
		void writeSettings(const QString &key);
		void startPlugin(const QPair<QString, QStringList> &plugin);
		void setLanguage(const QStringList &args);
		void parseMessage(const QString &message);
	private:
		struct Private;
		Private *const m_d;
		
		bool doUploadTest(const QStringList &args);
		void doStatsUpload();
		void init(const QStringList &args);
		void initApplicationIcons();
		void doSystemChecks();
		bool event(QEvent *event);
		
		void enableAutostart();
		void disableAutostart();
		
	private slots:
		void updateAvailable();
};

inline HyvesDesktopApplication *app() {
	return (HyvesDesktopApplication *) qApp;
}

#endif // HYVESDESKTOPAPPLICATION_H
