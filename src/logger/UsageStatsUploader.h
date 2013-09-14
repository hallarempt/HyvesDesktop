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

#ifndef USAGESTATSUPLOADER_H
#define USAGESTATSUPLOADER_H

#include <QNetworkReply>
#include <QObject>
#include <QString>

#include "hyveslib_export.h"

namespace Logger {
	
class HYVESLIB_EXPORT UsageStatsUploader : public QObject {
	
	Q_OBJECT
	
	public:
		UsageStatsUploader(QObject *parent = 0);
		virtual ~UsageStatsUploader();
		
		bool isUploading() const;
		
	public slots:
		/*
		 * Collects and uploads usage statistics from the current session
		 *
		 * @param logfile full path to the log file to be be uploaded
		 * @param removeFile if true the logfile will be removed when the upload is complete
		 * Connect to finished()/failed() if you need notification about the result
		 */
		void uploadUsageStats(const QString &logfile, bool removeFile);
		
	signals:
		/*
		 * Emitted when the stats-upload finished sucessfully
		 */
		void finished();

		/*
		 * Emmitted when the stats-upload failed
		 * (e.g. uploading disabled by user, network trouble,... etc.)
		 */
		void failed();
		
	private slots:
		void uploadDone(QNetworkReply *reply);
		void uploadError(QNetworkReply::NetworkError);
		
	private:
		struct Private;
		Private *const m_d;
		
		void dumpSettings(QIODevice *outputDevice);
};

} // namespace Logger

#endif // USAGESTATSUPLOADER_H
