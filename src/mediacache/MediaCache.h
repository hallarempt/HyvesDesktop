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

#ifndef MEDIACACHE_H
#define MEDIACACHE_H

#include <QObject>
#include <QImage>
#include <QPixmap>
#include <QString>

class QNetworkReply;
class QSize;

namespace MediaCache {

/**
 * Caches media from the Hyves website.
 * 
 * Throughout this class, media are identified using IDs. These IDs consist of
 * the mediaId and mediaSecret from the Hyves media architecture, combined like
 * this: "<mediaId>/<mediaSecret>". 
 */
class MediaCache : public QObject {
	
	Q_OBJECT
	
	public:
		static MediaCache *instance();
		static void destroy();
		
		/**
		 * clears the media cache
		 */
		void clear();
		
		/**
		 * Returns an image in a QPixmap representation.
		 * 
		 * @param id ID of the image.
		 * @param size Size in which to return the image.
		 * @return The image. If the image is not in the cache. A white pixmap
		 *         of the specified size is returned.
		 */
		QPixmap pixmap(const QString &id, const QSize &size);
		
		/**
		 * Returns an image in a QPixmap representation.
		 * 
		 * @param id ID of the image.
		 * @param width Width of the image to return.
		 * @param height Height of the image to return.
		 * @return The image. If the image is not in the cache. A white pixmap
		 *         of the specified size is returned.
		 */
		QPixmap pixmap(const QString &id, int width, int height);
		
		/**
		 * Returns an image in a QImage representation.
		 * 
		 * @param id ID of the image.
		 * @param size Size in which to return the image.
		 * @return The image. If the image is not in the cache. A white pixmap
		 *         of the specified size is returned.
		 */
		QImage image(const QString &id, const QSize &size);
		
		/**
		 * Returns an image in a QImage representation.
		 * 
		 * @param id ID of the image.
		 * @param width Width of the image to return.
		 * @param height Height of the image to return.
		 * @return The image. If the image is not in the cache. A white pixmap
		 *         of the specified size is returned.
		 */
		QImage image(const QString &id, int width, int height);
		
		/**
		 * Returns the contents of the media object from the specified URL. The
		 * URL should be a fake URL as returned mediaUrl().
		 * 
		 * @param url URL for the media object.
		 * @return Contents of the media object, which will be a PNG file.
		 */
		QByteArray get(const QString &url);
		
	public slots:
		/**
		 * Returns whether the given ID is already in the cache.
		 * 
		 * @param id ID of the image to check for. 
		 * @return true if the ID is already in the cache, false otherwise.
		 */
		bool isInCache(const QString &id);
		
		/**
		 * Returns the URL from which media object can be downloaded. This URL
		 * is a fake URL which is handled by the NetworkAccessManager which
		 * will in turn return the actual image object. This is useful for
		 * referencing the object from WebKit. Note that this is *not* the URL
		 * that's been insertede using insertUrl().
		 * 
		 * @param id ID of the media.
		 * @param size Size in which the media should be retrieved.
		 * @return The URL from which to retrieve the media.
		 *
		 * @sa download()
		 */
		QString mediaUrl(const QString &id, int width, int height);
		
		/**
		 * Inserts the URL for an image into the cache.
		 * 
		 * The image will be downloaded from the given URL automatically. Once
		 * the image is downloaded and loaded into the cache, the mediaLoaded()
		 * signal will be emitted.
		 * 
		 * @param id ID of the image.
		 * @param url URL where to find the image. The URL should have the
		 *                form: "http://<ip>/<id-range>/<id-range>/<mediaId>_<size>_<mediaSecret>.<extension>".
		 */
		void insertUrl(const QString &id, const QString &url);
		
	signals:
		/**
		 * Emitted when a new media object is loaded into the cache.
		 */
		void mediaLoaded(const QString &id);
		
	private:
		static MediaCache *s_instance;
		
		struct Private;
		Private *const m_d;
		
		MediaCache();
		virtual ~MediaCache();
		
		QPixmap emptyPixmap(const QSize &size);
		QImage emptyImage(const QSize &size);
		QByteArray emptyByteArray(const QSize &size);
		
		void writeImageToDiskCache(const QString &id, const QImage &image);
		bool readImageFromDiskCache(const QString &id);
		
	private slots:
		void handleReply();
};

} // namespace MediaCache

#endif // MEDIACACHE_H
