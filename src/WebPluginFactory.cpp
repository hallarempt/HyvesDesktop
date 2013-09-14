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

#include <QMap>
#include <QUrl>

#include "logger/Logger.h"

#include "WebPluginFactory.h"

WebPluginFactory *WebPluginFactory::s_instance = 0;

struct WebPluginFactory::Private {
	Logger::Logger *log;
	
	QMap<QString, QObject *> plugins;
};

WebPluginFactory *WebPluginFactory::instance() {
	
	if (s_instance == 0) {
		s_instance = new WebPluginFactory();
	}
	
	return s_instance;
}

void WebPluginFactory::destroy() {
	
	delete s_instance;
	s_instance = 0;
}

QObject *WebPluginFactory::create(const QString &mimeType, const QUrl &url,
                                  const QStringList &argumentNames, const QStringList &argumentValues) const {
	
	if (!m_d->plugins.contains(mimeType)) {
		m_d->log->warning(QString("Request for plugin for unsupported MIME type: %1.").arg(mimeType));
		return 0;
	}
	
	return m_d->plugins[mimeType];
}

QList<QWebPluginFactory::Plugin> WebPluginFactory::plugins() const {
	
	return QList<Plugin>();
}

void WebPluginFactory::registerPlugin(const QString &mimeType, QObject *object) {
	
	if (m_d->plugins.contains(mimeType)) {
		m_d->log->warning(QString("A plugin for MIME type %1 is already registered.").arg(mimeType));
		return;
	}
	
	m_d->plugins[mimeType] = object;
}

void WebPluginFactory::unregisterPlugin(const QString &mimeType) {
	
	if (!m_d->plugins.contains(mimeType)) {
		m_d->log->warning(QString("No plugin registered for MIME type %1.").arg(mimeType));
		return;
	}
	
	m_d->plugins.remove(mimeType);
}

WebPluginFactory::WebPluginFactory(QObject *parent) :
	QWebPluginFactory(parent),
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("WebPluginFactory");
}

WebPluginFactory::~WebPluginFactory() {
	
	delete m_d;
}
