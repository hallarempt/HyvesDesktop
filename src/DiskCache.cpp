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

#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>

#include "logger/Logger.h"
#include "settingsmanager/SettingsManager.h"
#include "DiskCache.h"

struct DiskCache::Private {
	Logger::Logger *log;
	
	QString baseServer;
	
	Private() :
		log(0) {
	}
	
	~Private() {
		
		delete log;
	}
	
	static QUrl filterUrl(const QUrl &url) {
		
		if (url.host().contains("hyves-static.net")) {
			QString urlString = url.toString();
			return QUrl("http://cache.hyves-static.net" + urlString.mid(urlString.indexOf("hyves-static.net") + 16));
		}
		
		return url;
	}
};

DiskCache::DiskCache(QObject *parent) :
	QNetworkDiskCache(parent),
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("DiskCache");
	
	m_d->baseServer = SettingsManager::SettingsManager::instance()->baseServer();
	
	QString cacheLocation = QDesktopServices::storageLocation(QDesktopServices::CacheLocation)
	                        + "/HyvesDesktop";
	setCacheDirectory(cacheLocation);
}

DiskCache::~DiskCache() {
	
	delete m_d;
}

QIODevice *DiskCache::data(const QUrl &url) {
	
	if (!url.toString().contains("cache")) {
		m_d->log->debug("Data for URL: " + Private::filterUrl(url).toString());
	}
	
	return QNetworkDiskCache::data(Private::filterUrl(url));
}

void DiskCache::insert(QIODevice *device) {
	
	//m_d->log->debug("Inserting device into cache.");
	
	QNetworkDiskCache::insert(device);
}

QNetworkCacheMetaData DiskCache::metaData(const QUrl &url) {
	
	QNetworkCacheMetaData metaData = QNetworkDiskCache::metaData(Private::filterUrl(url));
	
	if (metaData.isValid()) {
		if (metaData.url().host().contains("hyves-static.net")) {
			// content on hyves-static.net uses md5 hashes to determine whether the
			// content has changed. this means if the url hasn't changed (in which case
			// the meta-data is still valid), the content hasn't expired
			metaData.setExpirationDate(QDateTime::currentDateTime().addYears(10));
		}
	}
	
	if (!metaData.url().toString().contains("cache")) {
		m_d->log->debug("Meta-data for URL: " + metaData.url().toString() +
		                " (last modified: " + metaData.lastModified().toString() + ")");
	}
	
	return metaData;
}

QIODevice *DiskCache::prepare(const QNetworkCacheMetaData &metaData) {
	
	QNetworkCacheMetaData filteredMetaData = metaData;
	filteredMetaData.setUrl(Private::filterUrl(filteredMetaData.url()));
	
	if (!filteredMetaData.url().toString().contains("cache")) {
		m_d->log->debug("Prepare for URL: " + filteredMetaData.url().toString() +
		                " (last modified: " + filteredMetaData.lastModified().toString() + ")");
	}
	
	return QNetworkDiskCache::prepare(filteredMetaData);
}

bool DiskCache::remove(const QUrl &url) {
	
	if (!url.toString().contains("cache")) {
		m_d->log->debug("Removing URL: " + url.toString());
	}
	
	return QNetworkDiskCache::remove(Private::filterUrl(url));
}

void DiskCache::updateMetaData(const QNetworkCacheMetaData &metaData) {
	
	QNetworkCacheMetaData filteredMetaData = metaData;
	filteredMetaData.setUrl(Private::filterUrl(filteredMetaData.url()));
	
	if (!filteredMetaData.url().toString().contains("cache")) {
		m_d->log->debug("Update meta-data for URL: " + filteredMetaData.url().toString() +
		                " (last modified: " + filteredMetaData.lastModified().toString() + ")");
	}
	
	QNetworkDiskCache::updateMetaData(filteredMetaData);
}
