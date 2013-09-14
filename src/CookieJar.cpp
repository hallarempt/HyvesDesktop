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

#include <QByteArray>
#include <QDebug>
#include <QSettings>
#include <QUrl>

#include "CookieJar.h"
#include "windowmanager/WindowManager.h"
#include "settingsmanager/SettingsManager.h"

CookieJar::CookieJar(QObject *parent) :
	QNetworkCookieJar(parent) {
	
	loadConfig();
	
	connect(WindowManager::WindowManager::instance(), SIGNAL(eventRaised(QVariantMap)),
	        SLOT(eventRaised(QVariantMap)));
}

CookieJar::~CookieJar() {
	
	saveConfig();
}

void CookieJar::clearJar() {
	
	setAllCookies(QList<QNetworkCookie>());
}

bool CookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &cookieList,
                                  const QUrl &url) {
	
	QList<QNetworkCookie> cookies = cookieList;
	for (QList<QNetworkCookie>::Iterator it = cookies.begin();
	     it != cookies.end(); ++it) {
		
		QString domain = (*it).domain();
		if (domain.startsWith("hyves")) {
			(*it).setDomain(QString(".") + domain);
		}
	}
	
	return QNetworkCookieJar::setCookiesFromUrl(cookies, url);
}

void CookieJar::saveConfig() const {
	
	QList<QNetworkCookie> cookies = allCookies();
	
	QSettings *settings = SettingsManager::SettingsManager::instance()->settings();
	settings->beginWriteArray("Cookies/cookie");
	int i = 0;
	foreach (QNetworkCookie cookie, cookies) {
		if (cookie.isSessionCookie()) {
			continue;
		}
		QByteArray cookieString = cookie.toRawForm();
		settings->setArrayIndex(i);
		settings->setValue("content", cookieString);
		i++;
	}
	settings->endArray();
}

void CookieJar::loadConfig() {
	
	QList<QNetworkCookie> cookies;
	
	QSettings *settings = SettingsManager::SettingsManager::instance()->settings();
	int size = settings->beginReadArray("Cookies/cookie");
	for (int i = 0; i < size; i++) {
		settings->setArrayIndex(i);
		QByteArray cookieString = settings->value("content").toByteArray();
		QList<QNetworkCookie> cookieList = QNetworkCookie::parseCookies(cookieString);
		cookies += cookieList;
	}
	settings->endArray();
	
	setAllCookies(cookies);
}

void CookieJar::eventRaised(const QVariantMap &event) const {
	
	if (event["name"] == "userLoggedIn" || event["name"] == "userLoggedOut") {
		saveConfig();
	}
}
