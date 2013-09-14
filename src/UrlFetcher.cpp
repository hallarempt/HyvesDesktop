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

#include <QNetworkReply>
#include <QTemporaryFile>

#include "NetworkAccessManager.h"
#include "UrlFetcher.h"

struct UrlFetcher::Private {
	QUrl url;
	QNetworkAccessManager *accessManager;
	QNetworkReply *reply;
	QByteArray bytes;
	QNetworkReply::NetworkError errorCode;
	UrlFetcher::FetchType fetchType;
};

UrlFetcher::UrlFetcher(const QUrl &url, UrlFetcher::FetchType type, QObject *parent) :
	QObject(parent),
	m_d(new Private) {
	
	m_d->reply = 0;
	m_d->fetchType = type;
	m_d->url = url;
	m_d->accessManager = NetworkAccessManager::instance();
	refetch();
}

UrlFetcher::~UrlFetcher() {
	
	delete m_d;
}

void UrlFetcher::refetch() {
	
	QNetworkRequest request(m_d->url);
	m_d->reply = m_d->accessManager->get(request);
	connect(m_d->reply, SIGNAL(finished()), SLOT(finished()));
	connect(m_d->reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(error()));
}

void UrlFetcher::finished() {
	
	if (m_d->reply->error()) {
		if (m_d->fetchType == FetchString) {
			emit replyReady("");
		} else {
			emit replyReady(0);
		}
	} else {
		if (m_d->fetchType == FetchString) {
			// given that we're retrieving only relatively small resources,
			// letting the QNetworkReply object do the caching should be ok
			m_d->bytes = m_d->reply->readAll();
			QString string = QString::fromUtf8(m_d->bytes);
			emit replyReady(string);
		} else {
			QTemporaryFile *temporaryFile = new QTemporaryFile();
			temporaryFile->open();
			temporaryFile->write(m_d->reply->readAll());
			temporaryFile->flush();
			temporaryFile->setPermissions(QFile::ExeOther | QFile::ReadOther);
			emit replyReady(temporaryFile);
		}
	}
	m_d->reply->deleteLater();
}

void UrlFetcher::error() {
	
	// error is handled in finished()
}
