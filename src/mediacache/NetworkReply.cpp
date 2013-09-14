#include <QTimer>

#include "MediaCache.h"
#include "NetworkReply.h"

namespace MediaCache {
	
NetworkReply::NetworkReply(const QNetworkRequest &request, const QString &url, QObject *parent) :
	QNetworkReply(parent) {
	
	m_data = MediaCache::instance()->get(url);
	
	setRequest(request);
	setOpenMode(QIODevice::ReadOnly);
	
	setHeader(QNetworkRequest::ContentTypeHeader, "image/png");
	setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(m_data.length()));
	QTimer::singleShot(0, this, SIGNAL(metaDataChanged()));
	QTimer::singleShot(0, this, SIGNAL(readyRead()));
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

} // namespace MediaCache
