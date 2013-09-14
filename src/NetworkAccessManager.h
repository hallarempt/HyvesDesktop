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

#ifndef NETWORKACCESSMANAGER_H
#define NETWORKACCESSMANAGER_H

#include <QNetworkAccessManager>

#include "hyveslib_export.h"

class HYVESLIB_EXPORT NetworkAccessManager : public QNetworkAccessManager {
	
	Q_OBJECT
	
	public:
		static NetworkAccessManager *instance();
		static void destroy();
		
		/**
		 * Returns whether the local content bundle is used.
		 */
		bool useBundle() const;
		
		/**
		 * Sets whether the local content bundle should be used.
		 * 
		 * Note that other factors may cause this setting to be
		 * overwritten.
		 */
		void setUseBundle(bool useBundle);
		
		/**
		 * Sets the local bundle path.
		 *
		 * The default value is QApplication::applicationDirPath()+"/bundle"
		 */
		void setBundlePath(const QString &bundlePath);
		
		/**
		 * Returns the bundle path.
		 */
		QString bundlePath() const;
		
		/**
		 * Returns whether a file exists in the bundle.
		 *
		 * @param path The path of the file in the bundle.
		 */
		bool existsInBundle(const QString &path);
		
		/**
		 * Adds the handler for the given URL prefix to be the handler
		 * method provided by the receiver object.
		 */
		void addRequestHandler(const QString &prefix, QObject *receiver, const char *method);
		
		/**
		 * Removes the request handler for the given prefix.
		 */
		void removeRequestHandler(const QString &prefix);
		
	public slots:
		void loadSettings();
		
	protected:
		virtual QNetworkReply *createRequest(Operation operation, const QNetworkRequest &request,
		                                     QIODevice *outgoingData = 0);
			
		NetworkAccessManager();
		virtual ~NetworkAccessManager();
	
	private:
		static NetworkAccessManager *s_instance;
		
		struct Private;
		Private *const m_d;
		
	private slots:
		void sslErrors(QNetworkReply *reply, const QList<QSslError> &error);
		
		QNetworkReply *createRemoteRequest(QNetworkAccessManager::Operation operation,
		                                   const QNetworkRequest &originalRequest, const QString &path,
		                                   QIODevice *outgoingData = 0);
};

#endif // NETWORKACCESSMANAGER_H
