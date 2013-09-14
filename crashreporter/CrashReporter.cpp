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

#include <QtCore>
#include <QtGui>
#include <QtNetwork>

#include "config.h"
#include "CrashReporter.h"
#include "Restarter.h"

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

struct CrashReporter::Private {
	QString dumpPath;
	QString id;
	QNetworkAccessManager *networkAccessManager;
	
	bool doRestart;
	bool uploadStarted;
	
	Private() :
			doRestart(true),
			uploadStarted(false) {
	}
};

CrashReporter::CrashReporter(const QString &dumpPath, const QString &id) :
		QWidget(0),
		m_d(new Private()) {	


	setupUi(this);
	
	progressBar->hide();
	
	setWindowFlags(Qt::WindowStaysOnTopHint | windowFlags());
	
	m_d->networkAccessManager = new QNetworkAccessManager(this);
	
	connect(m_d->networkAccessManager, SIGNAL(finished(QNetworkReply *)), SLOT(uploadDone(QNetworkReply *)));
	
	connect(restartButton, SIGNAL(clicked()), this, SLOT(restart()));
	connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
	
	m_d->dumpPath = dumpPath;
	m_d->id = id;
}

CrashReporter::~CrashReporter() {
	
	delete m_d;
}

void CrashReporter::restart() {
	
	if (uploadCheckbox->isChecked()) {
		startUpload();
	} else {
		Restarter::restart();
		qApp->quit();
	}
}

void CrashReporter::close() {
	
	m_d->doRestart = false;
	if (!m_d->uploadStarted && uploadCheckbox->isChecked()) {
		startUpload();
	} else {
		qApp->quit();
	}
}

void CrashReporter::startUpload() {
	
	restartButton->setEnabled(false);
	progressBar->show();
	
	m_d->uploadStarted = true;
	
	// Upload minidump
	QNetworkRequest request(QUrl("http://" + HD_CRASH_URL + "/submit"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=9876543210"); 
	
	QString boundary = "--9876543210";
	
	typedef QPair<QByteArray, QByteArray> Field;
	QList<Field> fields;
	fields << Field("BuildID", HD_VERSION.toAscii())
			<< Field("ProductName", HD_PRODUCT_NAME.toAscii())
			<< Field("Version", HD_VERSION.toAscii())
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
	
	QByteArray body;
	foreach(Field const field, fields) {
		body += boundary + "\r\n";
		body += "Content-Disposition: form-data; name=\"" + field.first + "\"\r\n\r\n";
		body += field.second + "\r\n";
	}
	body += boundary + "\r\n";
	
	// add minidump file
	QString dumpfile = m_d->dumpPath + "/" + m_d->id + ".dmp";
	body += "Content-Disposition: form-data; name=\"upload_file_minidump\"; filename=\""
		 + QFileInfo(dumpfile).fileName().toAscii() + "\"\r\n";
	body += "Content-Type: application/octet-stream\r\n\r\n";
	QFile file(dumpfile);
	if (file.exists()) {	
		file.open(QFile::ReadOnly);
		body += file.readAll();
		file.remove();
	}
	body += "\r\n";	
	
	// add logfile
	body += boundary + "\r\n";
	body += "Content-Disposition: form-data; name=\"logfile\"\r\n";
	body += "\r\n";
	QString dataLocation = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
	QString logFileLocation = dataLocation + "/log/session.txt";
	QFile logFile(logFileLocation);
	if (logFile.exists() && logFile.open(QFile::ReadOnly)) {
		// truncate logfile if necessary, only submit whole lines
		if (logFile.size() > MAX_LOGUPLOAD_SIZE) {
			logFile.seek(logFile.size() - (MAX_LOGUPLOAD_SIZE + 1));
			logFile.readLine();
		}
		body +=	logFile.read(MAX_LOGUPLOAD_SIZE);
		logFile.close();
	}
	body += "\r\n";
	body += boundary + "--" + "\r\n";
	
	
	QNetworkReply *reply = m_d->networkAccessManager->post(request, body);
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(uploadError(QNetworkReply::NetworkError)));
	connect(reply, SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(uploadProgress(qint64,qint64)));
}

void CrashReporter::uploadDone(QNetworkReply *reply) {
	
	if (reply && reply->error() != QNetworkReply::NoError) {	
		qCritical() << "uploadDone: Error uploading crash report: " << reply->errorString();
	}
	
	if (m_d->doRestart) {
		Restarter::restart();
	}
	qApp->quit();
}

void CrashReporter::uploadProgress(qint64 received, qint64 total) {
	
	progressBar->setMaximum(total);
	progressBar->setValue(received);
}

void CrashReporter::uploadError(QNetworkReply::NetworkError error) {
	
	// Fake success...
	progressBar->setRange(0, 100);
	progressBar->setValue(100);
	qCritical() << "UploadError: Error uploading crash report: " << error;
	
	uploadDone(0);
}
