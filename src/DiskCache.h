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

#ifndef DISKCACHE_H
#define DISKCACHE_H

#include <QNetworkDiskCache>

#include "hyveslib_export.h"

class HYVESLIB_EXPORT DiskCache : public QNetworkDiskCache {
	
	Q_OBJECT
	
	public:
		DiskCache(QObject *parent = 0);
		virtual ~DiskCache();
		
		virtual QIODevice *data(const QUrl &url);
		virtual void insert(QIODevice *device);
		virtual QNetworkCacheMetaData metaData(const QUrl &url);
		virtual QIODevice *prepare(const QNetworkCacheMetaData &metaData);
		virtual bool remove(const QUrl &url);
		virtual void updateMetaData(const QNetworkCacheMetaData &metaData);
		
	private:
		struct Private;
		Private *const m_d;
};

#endif // DISKCACHE_H
