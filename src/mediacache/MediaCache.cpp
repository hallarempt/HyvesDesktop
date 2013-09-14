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

#include <QBuffer>
#include <QByteArray>
#include <QColor>
#include <QDesktopServices>
#include <QDir>
#include <QImage>
#include <QMap>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>

#include "filer/Filer.h"
#include "extender/Extender.h"
#include "logger/Logger.h"
#include "MediaCache.h"
#include "NetworkAccessManager.h"
#include "config.h"

namespace MediaCache {

MediaCache *MediaCache::s_instance = 0;

struct MediaCache::Private {
	
	Logger::Logger *log;
	
	QMap<QString, QImage> images;
	QMap<QString, QPixmap> cachedPixmaps;
	QMap<QString, QImage> cachedImages;
	QMap<QString, QByteArray> cachedByteArrays;
	
	QString cacheLocation;
	
	Private() :
		log(0) {
	}
	
	~Private() {
		
		delete log;
	}
	
	static QString rewriteUrl(const QString &url, const QString &id) {
		
		Q_ASSERT(url.contains('/'));
		
		QString newUrl(url);
		QStringList idParts = id.split('/');
		Q_ASSERT(idParts.size() == 2);
		if(idParts[0].contains(QRegExp("[^\\d]"))) {
			Logger::Logger::instance()->warning("MediaCache.cpp:rewriteUrl(): Invalid media ID: " + idParts[0]);
			return "";
		}
		if(idParts[1].size() != 4) {
			Logger::Logger::instance()->warning("MediaCache.cpp:rewriteUrl(): Invalid media secret: " + idParts[1]);
			return "";
		}
		return newUrl.replace(QRegExp(idParts[0] + "_\\d_" + idParts[1]),
		                      idParts[0] + "_14_" + idParts[1]);
	}
	
	static QImage scaleImage(const QImage &image, const QSize &size) {
		
		QImage scaledImage = image.scaled(
			size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation
		);
		
		QRect rect;
		if (scaledImage.width() > size.width()) {
			int cutoffMargin = (scaledImage.width() - size.width()) / 2;
			int cutoffRest = (scaledImage.width() - size.width()) % 2;
			rect.setCoords(cutoffMargin, 0, scaledImage.width() - cutoffMargin - cutoffRest - 1, size.height() - 1);
		} else {
			int cutoffMargin = (scaledImage.height() - size.height()) / 2;
			int cutoffRest = (scaledImage.height() - size.height()) % 2;
			rect.setCoords(0, cutoffMargin, size.width() - 1, scaledImage.height() - cutoffMargin - cutoffRest - 1);
		}
		return scaledImage.copy(rect);
	}
	
	static QByteArray imageToByteArray(const QImage &image) {
		
		QByteArray byteArray;
		QBuffer buffer(&byteArray);
		buffer.open(QIODevice::WriteOnly);
		image.save(&buffer, "PNG");
		buffer.close();
		
		return byteArray;
	}
};

MediaCache *MediaCache::instance() {
	
	if (s_instance == 0) {
		s_instance = new MediaCache();
	}
	
	return s_instance;
}

void MediaCache::destroy() {
	
	delete s_instance;
	s_instance = 0;
}

void MediaCache::clear() {
	
	QDir dir;
	if (!dir.exists(m_d->cacheLocation)) {
		return;
	}
	
	bool result = Filer::removeDirectory(m_d->cacheLocation);
	if (result) {
		m_d->log->debug(QString("Media cache at %1 is cleared").arg(m_d->cacheLocation));
	} else {
		m_d->log->error(QString("Could not clear media cache at %1").arg(m_d->cacheLocation));
	}
	
	dir.mkpath(m_d->cacheLocation);
}


QPixmap MediaCache::pixmap(const QString &id, const QSize &size) {
	
	if (!isInCache(id)) {
		return emptyPixmap(size);
	}
	
	QString key = id + "_" + size.width() + "_" + size.height();
	
	if (!m_d->cachedPixmaps.contains(key)) {
		Q_ASSERT(!m_d->images[id].isNull());
		m_d->cachedPixmaps[key] = QPixmap::fromImage(Private::scaleImage(m_d->images[id], size));
		Q_ASSERT(m_d->cachedPixmaps[key].size() == size);
	}
	
	return m_d->cachedPixmaps[key];
}

QPixmap MediaCache::pixmap(const QString &id, int width, int height) {
	
	return pixmap(id, QSize(width, height));
}

QImage MediaCache::image(const QString &id, const QSize &size) {
	
	if (!isInCache(id)) {
		return emptyImage(size);
	}
	
	QString key = id + "_" + size.width() + "_" + size.height();
	
	if (!m_d->cachedImages.contains(key)) {
		Q_ASSERT(!m_d->images[id].isNull());
		m_d->cachedImages[key] = Private::scaleImage(m_d->images[id], size);
		Q_ASSERT(m_d->cachedImages[key].size() == size);
	}
	
	return m_d->cachedImages[key];
}

QImage MediaCache::image(const QString &id, int width, int height) {
	
	return image(id, QSize(width, height));
}

QByteArray MediaCache::get(const QString &url) {
	
	static QRegExp regExp("^http://localhost/media/(\\d+/[\\w-]+)/(\\d+)x(\\d+)\\.png$");
	if (!regExp.exactMatch(url)) {
		m_d->log->warning("Invalid URL given to get(): " + url);
		return QByteArray();
	}
	
	QString id(regExp.cap(1));
	QSize size(regExp.cap(2).toInt(), regExp.cap(3).toInt());
	
	if (!isInCache(id)) {
		m_d->log->debug("So we gave out a URL to a non-cached object?");
		return emptyByteArray(size);
	}
	
	QString key = id + "_" + size.width() + "_" + size.height();
	
	if (!m_d->cachedByteArrays.contains(key)) {
		Q_ASSERT(!m_d->images[id].isNull());
		m_d->cachedByteArrays[key] = Private::imageToByteArray(Private::scaleImage(m_d->images[id], size));
	}
	
	return m_d->cachedByteArrays[key];
}

bool MediaCache::isInCache(const QString &id) {
	
	if (m_d->images.contains(id)) {
		return !m_d->images[id].isNull();
	}
	
	return readImageFromDiskCache(id);
}

QString MediaCache::mediaUrl(const QString &id, int width, int height) {
	
	return QString("http://localhost/media/%1/%2x%3.png").arg(id).arg(width).arg(height);
}

void MediaCache::insertUrl(const QString &id, const QString &url) {
	
	if (m_d->images.contains(id)) {
		return;
	}
	
	m_d->images[id] = QImage();
	
	QNetworkReply *reply = NetworkAccessManager::instance()->get(QNetworkRequest(Private::rewriteUrl(url, id)));
	reply->setProperty("id", id);
	
	connect(reply, SIGNAL(finished()), SLOT(handleReply()));
}

MediaCache::MediaCache() :
	QObject(),
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("MediaCache");
	
	QString dataLocation = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#if QT_VERSION == 0x040500
#ifdef Q_WS_MAC
	dataLocation = QDesktopServices::storageLocation(QDesktopServices::HomeLocation) + "/Library/Application Support/Hyves Desktop";
#endif
#endif
	m_d->cacheLocation = dataLocation + "/MediaCache";
	QDir dir;
	dir.mkpath(m_d->cacheLocation);
	
	Extender::Extender::instance()->registerObject("mediaCache", this);
}

MediaCache::~MediaCache() {
	
	Extender::Extender::instance()->unregisterObject("mediaCache");
	
	delete m_d;
}

QPixmap MediaCache::emptyPixmap(const QSize &size) {
	
	QString key = "0/0_" + QString::number(size.width()) + "_" + QString::number(size.height());
	
	if (!m_d->cachedPixmaps.contains(key)) {
		QPixmap pixmap(size);
		pixmap.fill(QColor::fromRgb(0xff, 0xff, 0xff));
		m_d->cachedPixmaps[key] = pixmap;
	}
	
	return m_d->cachedPixmaps[key];
}

QImage MediaCache::emptyImage(const QSize &size) {
	
	QString key = "0/0_" + QString::number(size.width()) + "_" + QString::number(size.height());
	
	if (!m_d->cachedImages.contains(key)) {
		QImage image(size, QImage::Format_Mono);
		image.fill(1);
		m_d->cachedImages[key] = image;
	}
	
	return m_d->cachedImages[key];
}

QByteArray MediaCache::emptyByteArray(const QSize &size) {
	
	QString key = "0/0_" + QString::number(size.width()) + "_" + QString::number(size.height());
	
	if (!m_d->cachedByteArrays.contains(key)) {
		QImage image(size, QImage::Format_Mono);
		image.fill(1);
		m_d->cachedByteArrays[key] = Private::imageToByteArray(image);
	}
	
	return m_d->cachedByteArrays[key];
}

void MediaCache::writeImageToDiskCache(const QString &id, const QImage &image) {
	
	QString replacedId(id);
	QString fileName = m_d->cacheLocation + "/" + replacedId.replace('/', '_') + ".png";
	bool writeSucceeded = image.save(fileName, "PNG");
	if (!writeSucceeded) {
		m_d->log->warning("Could not write media object (" + id + ") to disk cache.");
	}
}

bool MediaCache::readImageFromDiskCache(const QString &id) {
	
	QString replacedId(id);
	QString fileName = m_d->cacheLocation + "/" + replacedId.replace('/', '_') + ".png";
	QImage image;
	bool readSucceeded = image.load(fileName);
	if (!readSucceeded) {
		return false;
	}
	
	m_d->images[id] = image;
	return true;
}

void MediaCache::handleReply() {
	
	QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
	Q_ASSERT(reply);
	
	reply->deleteLater();
	
	if (reply->error() != QNetworkReply::NoError) {
		m_d->log->debug("Error requesting image from: " + reply->url().toString());
		return;
	}
	
	QString id = reply->property("id").toString();
	if (!m_d->images.contains(id)) {
		m_d->log->debug("Can't remember I requested an image from: " + reply->url().toString());
		return;
	}
	
	bool isValidImage = m_d->images[id].loadFromData(reply->readAll());
	if (!isValidImage) {
		m_d->log->debug("Downloaded an invalid image from: " + reply->url().toString());
		return;
	}
	
	writeImageToDiskCache(id, m_d->images[id]);
	
	emit mediaLoaded(id);
}

} // namespace MediaCache
