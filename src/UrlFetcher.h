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

#ifndef URLFETCHER_H
#define URLFETCHER_H

#include <QObject>

class QTemporaryFile;
class QUrl;
class QString;

#include "hyveslib_export.h"

/**
 * UrlFetcher fetches a resource from a particular URL.
 * It is asynchronous, you create the UrlFetcher with a particular
 * URL and when it is done, a signal is sent with the requested
 */
class  HYVESLIB_EXPORT UrlFetcher : public QObject {
	
	Q_OBJECT
	
	public:
		enum FetchType {
			FetchString = 1,
			FetchTemporaryFile
		};
		
		/**
		 * Starts fetching a URL.
		 *
		 * @param url URL to fetch.
		 * @param type Determines whether the reply is returned as string or
		 *             placed in a temporary file.
		 * @param parent Parent object.
		 */
		UrlFetcher(const QUrl &url, FetchType type = FetchString, QObject *parent = 0);
		virtual ~UrlFetcher();
		
		/**
		 * Starts refetching the URL using the same parameters.
		 */
		void refetch();
		
	signals:
		/**
		 * Emitted when a reply is available. This signal is only emitted when
		 * fetch was initiated with fetch type FetchString.
		 *
		 * @param reply String content of the reply. May be an empty string in
		 *              case of an error.
		 */
		void replyReady(const QString &reply);

		/**
		 * Emitted when a reply is available. This signal is only emitted when
		 * fetch was initiated with fetch type FetchTemporaryFile.
		 *
		 * @param temporaryFile Temporary file containing the reply. The caller
		 *                      should delete this instance. May be a null
		 *                      pointer in case of an error.
		 */
		void replyReady(QTemporaryFile *temporaryFile);
		
	private slots:
		void finished();
		void error();
		
	private:
		struct Private;
		Private *const m_d;
};

#endif // URLFETCHER_H
