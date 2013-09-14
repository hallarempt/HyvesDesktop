#ifndef NETWORKREPLY_H
#define NETWORKREPLY_H

#include <QByteArray>
#include <QNetworkReply>

#include "hyveslib_export.h"

class HYVESLIB_EXPORT NetworkReply : public QNetworkReply {
	
	Q_OBJECT

	public:
		NetworkReply(const QNetworkRequest &request, const QString &path, QObject *parent = 0);
		NetworkReply(const QNetworkRequest &request, const QByteArray &data, QObject *parent = 0);
		virtual ~NetworkReply();
		
		virtual void abort();
		virtual qint64 bytesAvailable() const;
		
	protected:
		virtual qint64 readData(char *data, qint64 maxLength);
		
	private:
		QByteArray m_data;
		
		void init(const QNetworkRequest &request);
		const char *guessMimeType(const QString &path);
};

#endif // NETWORKREPLY_H
