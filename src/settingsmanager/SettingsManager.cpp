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

#include <cstdlib>

#include <QApplication>
#include <QSettings>

#include "CookieJar.h"
#include "NetworkAccessManager.h"
#include "SettingsManager.h"
#include "config.h"
#include "extender/Extender.h"
#include "logger/Logger.h"
#include "mediacache/MediaCache.h"
#include "historykeeper/HistoryKeeper.h"

namespace SettingsManager {

const QString BASE_SERVER = "hyves.nl";

SettingsManager *SettingsManager::s_instance = 0;

struct SettingsManager::Private {
	QSettings* settings;
	Logger::Logger *log;
	
	Private() :
		settings(0),
		log(0) {
	}
	
	~Private() {
		delete log;
	}
};

SettingsManager *SettingsManager::instance() {
	
	if (s_instance == 0) {
		s_instance = new SettingsManager();
	}
	
	return s_instance;
}

void SettingsManager::destroy() {
	
	delete s_instance;
	s_instance = 0;
}

void SettingsManager::clearSettings() {
	
	CookieJar *cookieJar = dynamic_cast<CookieJar *>(NetworkAccessManager::instance()->cookieJar());
	Q_ASSERT(cookieJar != 0);
	cookieJar->clearJar();
	
	MediaCache::MediaCache::instance()->clear();
	
	HistoryKeeper::HistoryKeeper::instance()->clear();
	
	m_d->settings->clear();
}

QSettings *SettingsManager::settings() {
	
	return m_d->settings;
}

QVariant SettingsManager::variantValue(const QString &key, const QVariant &defaultValue) {
	
	return m_d->settings->value(key, defaultValue);
}

void SettingsManager::setVariantValue(const QString &key, const QVariant &value, const QVariant &defaultValue) {
	
	if (m_d->settings->value(key).toByteArray() != value) {
		m_d->settings->setValue(key, value);
		emit settingChanged(key);
	}
}

bool SettingsManager::boolValue(const QString &key, bool defaultValue) {
	
	return m_d->settings->value(key, defaultValue).toBool();
}

int SettingsManager::intValue(const QString &key, int defaultValue) {
	
	return m_d->settings->value(key, defaultValue).toInt();
}

QString SettingsManager::stringValue(const QString &key, const QString &defaultValue) {
	
	return m_d->settings->value(key, defaultValue).toString();
}

void SettingsManager::setValue(const QString &key, bool value, bool defaultValue) {
	
	if (m_d->settings->value(key, defaultValue).toBool() != value) {
		m_d->settings->setValue(key, value);
		emit settingChanged(key);
	}
}

void SettingsManager::setValue(const QString &key, int value, int defaultValue) {
	
	if (m_d->settings->value(key, defaultValue).toInt() != value) {
		m_d->settings->setValue(key, value);
		emit settingChanged(key);
	}
}

void SettingsManager::setValue(const QString &key, const QString &value) {
	
	if (m_d->settings->value(key).toString() != value) {
		m_d->settings->setValue(key, value);
		emit settingChanged(key);
	}
}

void SettingsManager::setValue(const QString &key, const QByteArray &value) {
	
	if (m_d->settings->value(key).toByteArray() != value) {
		m_d->settings->setValue(key, value);
		emit settingChanged(key);
	}
}

QString SettingsManager::baseServer() const {
	
	QString baseServer = BASE_SERVER;
	if (!qgetenv("BASE_SERVER").isEmpty()) {
		baseServer = QString::fromLatin1(qgetenv("BASE_SERVER"));
	}
	return baseServer;
}

SettingsManager::SettingsManager() :
	QObject(),
	m_d(new Private()) {
	m_d->settings = new QSettings(HD_DOMAIN_NAME, HD_PRODUCT_NAME, this);
	m_d->log = new Logger::Logger("SettingsManager");
	
	Extender::Extender::instance()->registerObject("settingsManager", this);
}

SettingsManager::~SettingsManager() {
	
	Extender::Extender::instance()->unregisterObject("settingsManager");
	
	delete m_d;
}

} // namespace SettingsManager
