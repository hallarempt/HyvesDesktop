#ifndef MEDIACACHE_NETWORKREPLY_H
#define MEDIACACHE_NETWORKREPLY_H

#include <QNetworkReply>

namespace MediaCache {

class NetworkReply : public QNetworkReply {
	
	public:
		NetworkReply(const QNetworkRequest &request, const QString &url, QObject *parent = 0);
		virtual ~NetworkReply();
		
		virtual void abort();
		virtual qint64 bytesAvailable() const;
		
	protected:
		virtual qint64 readData(char *data, qint64 maxLength);
		
	private:
		QByteArray m_data;
};

} // MediaCache

#endif // MEDIACACHE_NETWORKREPLY_H
