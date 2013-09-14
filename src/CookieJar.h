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

#ifndef COOKIEJAR_H
#define COOKIEJAR_H

#include <QNetworkCookieJar>
#include <QVariantMap>

#include "hyveslib_export.h"

class HYVESLIB_EXPORT CookieJar : public QNetworkCookieJar {
	
	Q_OBJECT
	
	public:
		CookieJar(QObject *parent = 0);
		virtual ~CookieJar();
		
		/**
		 * Removes all cookies currently in the jar.
		 */
		void clearJar();
		
		bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList,
		                       const QUrl &url);
		
	private:
		void saveConfig() const;
		void loadConfig();
		
	private slots:
		void eventRaised(const QVariantMap &event) const;
};

#endif // COOKIEJAR_H
