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
#ifndef CRASH_REPORTER
#define CRASH_REPORTER

#include "ui_CrashReporter.h"

#include <QNetworkReply>
#include <QObject>
#include <QWidget>

// truncate logfiles if larger than 200kB
const qint64 MAX_LOGUPLOAD_SIZE = 200 * 1024;

class QString;

class CrashReporter : public QWidget, public Ui::CrashReporter {
	
	Q_OBJECT
	
	public:
		CrashReporter(const QString &dumpPath, const QString &id);
		virtual ~CrashReporter();

	private slots:
		void restart();
		void close();
		
		void startUpload();
		
		void uploadDone(QNetworkReply *reply);
		void uploadProgress(qint64 received, qint64 total);
		void uploadError(QNetworkReply::NetworkError);
		
	private:
		struct Private;
		Private *const m_d;
};

#endif
