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
#include <QDir>
#include <QFile>
#include <QBuffer>
#include <QtNetwork>
#include <QTextStream>
#include <QStringList>

#include "../settingsmanager/SettingsManager.h"
#include "../tester/Tester.h"
#include "UsageStatsUploader.h"
#include "Logger.h"
#include "config.h"

namespace Logger {

#ifdef Q_WS_MAC
QString platformToStringMac(QSysInfo::MacVersion version) {
	
	switch(version) {
		case QSysInfo::MV_9:
			return "MacOS 9";
		case QSysInfo::MV_10_0:
			return "OSX 10.0 (cheetah)";
		case QSysInfo::MV_10_1:
			return "OSX 10.1 (puma)";
		case QSysInfo::MV_10_2:
			return "OSX 10.2 (jaguar)";
		case QSysInfo::MV_10_3:	
			return "OSX 10.3 (panther)";
		case QSysInfo::MV_10_4:
			return "OSX 10.4 (tiger)";
		case QSysInfo::MV_10_5:
			return "OSX 10.5 (leopard)";
		case QSysInfo::MV_10_6:
			return "OSX 10.6 (snow leopard)";
		case QSysInfo::MV_Unknown:
		default:
			return "Unknown OSX version";
	};
	
}
#endif

#ifdef Q_WS_WIN
QString platformToStringWin(QSysInfo::WinVersion version) {
	
	switch(version) {
	case QSysInfo::WV_32s:
		return "Windows 3.1 with win32s";
	case QSysInfo::WV_95:
		return "Windows 95";
	case QSysInfo::WV_98:
		return "Windows 98";
	case QSysInfo::WV_Me:
		return "Windows Me";
	case QSysInfo::WV_NT:
		return "Windows NT";
	case QSysInfo::WV_2000:
		return "Windows 2000";
	case QSysInfo::WV_XP:
		return "Windows XP";
	case QSysInfo::WV_2003:
		return "Windows 2003";
	case QSysInfo::WV_VISTA:
		return "Windows Vista";
	//case QSysInfo::WV_WINDOWS7:
	//	return "Windows 7";
	default:
		return "Unknown Windows version";
	};
	
}
#endif

struct UsageStatsUploader::Private {
	Logger *log;
	bool uploadStarted;
	QNetworkAccessManager *networkAccessManager;
	QString removeFileWhenDone;
	
	Private() :
		log(0),
		uploadStarted(false),
		networkAccessManager(0) {
	}
};

UsageStatsUploader::UsageStatsUploader(QObject *parent) :
	m_d(new Private()) {
	
	m_d->log = new Logger("UsageStatsUploader");
	m_d->networkAccessManager = new QNetworkAccessManager(this);
	
	connect(m_d->networkAccessManager, SIGNAL(finished(QNetworkReply *)), SLOT(uploadDone(QNetworkReply *)));
}

UsageStatsUploader::~UsageStatsUploader() {
	delete m_d->log;
	delete m_d;
}

bool UsageStatsUploader::isUploading() const {
	return m_d->uploadStarted;
}

void UsageStatsUploader::uploadUsageStats(const QString &filename, bool removeFile) {
	
	// do not upload anything if the user hasn't agreed (except when in test-mode)
	if (!Tester::Tester::isTesting() && !SettingsManager::SettingsManager::instance()->boolValue("Statistics/submitUsageStats", true)) {
		m_d->log->notice("Not uploading usage statistics (disabled by user)");
		emit failed();
		return;
	}
	
	if (m_d->uploadStarted) {
		m_d->log->debug("An upload is already in progress");
		emit failed();
		return;
	}
	m_d->uploadStarted = true;
	m_d->removeFileWhenDone = removeFile ? filename : "";

	
	// Upload usage statistics
	QNetworkRequest request(QUrl(Tester::Tester::isTesting() ? HD_STATS_TEST_URL : HD_STATS_PROD_URL));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=9876543210"); 
	
	QString boundary = "--9876543210";
	
	typedef QPair<QByteArray, QByteArray> Field;
	QList<Field> fields;
	fields << Field("BuildID", HD_VERSION.toString().toAscii())
			<< Field("ProductName", HD_PRODUCT_NAME.toAscii())
			<< Field("Version", HD_VERSION.toString().toAscii())
			<< Field("Vendor", "Hyves")
			<< Field("timestamp", QByteArray::number(QDateTime::currentDateTime().toTime_t())); 

#ifdef Q_WS_WIN
	fields << Field("Platform", platformToStringWin(QSysInfo::WindowsVersion).toAscii());
#endif
#ifdef Q_WS_X11
	fields << Field("Platform", "Linux/X11");
#endif
#ifdef Q_WS_MAC
	fields << Field("Platform", platformToStringMac(QSysInfo::MacintoshVersion).toAscii());
#endif
	
	QBuffer data;
	data.open(QIODevice::WriteOnly);
	QTextStream body(&data);
	foreach(Field const field, fields) {
		body << boundary << "\r\n";
		body << "Content-Disposition: form-data; name=\"" << field.first << "\"\r\n\r\n";
		body << field.second << "\r\n";
	}
	
	// fake dump file... (server requires attribute 'upload_file_minidump')
	body << boundary << "\r\n";
	body << "Content-Disposition: form-data; name=\"upload_file_minidump\"; filename=\"not_a_file\"\r\n";
	body << "Content-Type: application/octet-stream\r\n\r\n";
	// <content of the dumpfile would go here>
	body << "\r\n";	
	
	// add log file
	QFile logfile(filename);
	body << boundary << "\r\n";
	body << "Content-Disposition: form-data; name=\"logfile\"\r\n\r\n";
	if (logfile.exists() && logfile.open(QFile::ReadOnly)) {
		if (logfile.size() > HD_STATS_MAX_FILESIZE) {
			logfile.seek(logfile.size() - (HD_STATS_MAX_FILESIZE + 1));
			logfile.readLine();	// throw away cut off rest from the first line
		}
		body <<	logfile.read(HD_STATS_MAX_FILESIZE);
		logfile.close();
	} else {
		m_d->log->debug(QString("Failed to open file %1").arg(Logger::anonymize(filename, File)));
	}
	
	// add settings
	dumpSettings(&data);
	
	body << "\r\n";
	
	data.close();
	QNetworkReply *reply = m_d->networkAccessManager->post(request, data.buffer());
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(uploadError(QNetworkReply::NetworkError)));
	m_d->log->notice(QString("Uploading statistics to %1").arg(request.url().toString()));
}

void UsageStatsUploader::uploadDone(QNetworkReply *reply) {
	
	m_d->uploadStarted = false;
	
	if (reply && reply->error() == QNetworkReply::NoError) {	
		m_d->log->notice("Successfully uploaded statistics");
		// cleanup log file
		if (!m_d->removeFileWhenDone.isEmpty() && QFile::exists(m_d->removeFileWhenDone)) {
			m_d->log->debug(QString("removing log file %1").arg(Logger::anonymize(m_d->removeFileWhenDone, File)));
			QDir::root().remove(m_d->removeFileWhenDone);
		}
		emit finished();
	} else {
		m_d->log->warning(QString("Failed to upload statistics - server reply: %1").arg(reply ? reply->errorString() : "unknown"));
		emit failed();
	}
}

void UsageStatsUploader::uploadError(QNetworkReply::NetworkError error) {
	
	m_d->log->debug(QString("Network error: %1 (see QNetworkReply::NetworkError for details)").arg(error));
}

void UsageStatsUploader::dumpSettings(QIODevice *outputDevice) {
	
	QTextStream ostream(outputDevice);
	SettingsManager::SettingsManager *settings = SettingsManager::SettingsManager::instance();
	ostream << "<= start settings dump =>" << endl;
	
	QStringList keys = HD_STATS_UPLOAD_SETTINGS.split(';');
	foreach (QString key, keys) {
		QVariant data = settings->variantValue(key, -1);
		ostream << key << "=";
		switch (data.type()) {
			case QVariant::String:
				ostream << data.toString();
				break;
			case QVariant::Int:
				ostream << data.toInt();
				break;
			case QVariant::Bool:
				ostream << data.toBool();
				break;
			default:
				ostream << "unknown_type";
				m_d->log->warning(QString("Failed to dump value for key %1 due to unsupported/unknown type").arg(key));
				break;
		}
		ostream << endl;
	}
	ostream << "<= end settings dump =>";
}

} // namespace Logger
