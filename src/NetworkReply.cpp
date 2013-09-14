#include <QApplication>
#include <QDesktopServices>
#include <QFile>
#include <QTimer>

#include "logger/Logger.h"
#include "zipper/Zipper.h"
#include "NetworkReply.h"
#include "NetworkAccessManager.h"

NetworkReply::NetworkReply(const QNetworkRequest &request, const QString &path, QObject *parent) :
	QNetworkReply(parent) {
	
	if (path.startsWith("bundle://")) {
		Zipper::ZippedFile file("bundle.zip", path.mid(9));
		if (!file.open(QFile::ReadOnly)) {
			Logger::Logger::instance()->warning("Cannot open file in bundle: " + path);
			return;
		}
		
		m_data = file.readAll();
	} else {
		if (!path.startsWith(NetworkAccessManager::instance()->bundlePath()) &&
		    !path.startsWith(QDesktopServices::storageLocation(QDesktopServices::DataLocation))) {
			Logger::Logger::instance()->warning("Tried to open path outside of bundle: " + path);
			return;
		}
		
		QFile file(path);
		if (!file.open(QFile::ReadOnly)) {
			Logger::Logger::instance()->warning("Cannot open file in bundle: " + path);
			return;
		}
		
		m_data = file.readAll();
	}
	
	setHeader(QNetworkRequest::ContentTypeHeader, guessMimeType(path));
	init(request);
}

NetworkReply::NetworkReply(const QNetworkRequest &request, const QByteArray &data, QObject *parent) :
	QNetworkReply(parent),
	m_data(data) {
	
	setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
	init(request);
}

NetworkReply::~NetworkReply() {
}

void NetworkReply::abort() {
}

qint64 NetworkReply::bytesAvailable() const {
	
	return m_data.length() + QNetworkReply::bytesAvailable();
}

qint64 NetworkReply::readData(char *buffer, qint64 maxLength) {
	
	qint64 length = qMin(qint64(m_data.length()), maxLength);
	if (length) {
		qMemCopy(buffer, m_data.constData(), length);
		m_data.remove(0, length);
	}
	if (!m_data.length()) {
		QTimer::singleShot(0, this, SIGNAL(finished()));
	}
	return length;
}

void NetworkReply::init(const QNetworkRequest &request) {
	
	setRequest(request);
	setOpenMode(QIODevice::ReadOnly);
	
	setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(m_data.length()));
	QTimer::singleShot(0, this, SIGNAL(metaDataChanged()));
	QTimer::singleShot(0, this, SIGNAL(readyRead()));
}

const char *NetworkReply::guessMimeType(const QString &path) {
	
	QString extension = path.mid(path.lastIndexOf('.') + 1);
	if (extension == "js") {
		return "text/javascript";
	} else if (extension == "css") {
		return "text/css";
	} else if (extension == "html") {
		return "text/html";
	} else if (extension == "png") {
		return "image/png";
	} else if (extension == "gif") {
		return "image/gif";
	} else if (extension == "jpg") {
		return "image/jpeg";
	}
	return "application/octet-stream";
}
