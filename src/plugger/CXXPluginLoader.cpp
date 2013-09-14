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

#include <QApplication>
#include <QFile>
#include <QPluginLoader>
#include <QDebug>

#include "logger/Logger.h"
#include "CXXPluginInterface.h"
#include "CXXPluginLoader.h"
#include "PluginInfo.h"

namespace Plugger {

struct CXXPluginLoader::Private {
	Logger::Logger *log;
	
	PluginInfo info;
	QPluginLoader loader;
	CXXPluginInterface *plugin;
	
	Private() :
		log(0),
		plugin(0) {
	}
	
	~Private() {
		
		delete log;
	}
};

CXXPluginLoader::CXXPluginLoader(QObject *parent) :
	PluginLoader(parent),
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("CXXPluginLoader");
}

CXXPluginLoader::~CXXPluginLoader() {
	
	delete m_d;
}

bool CXXPluginLoader::loadPlugin(const PluginInfo &info, const QStringList &args) {
	
	if(m_d->plugin) {
		m_d->log->warning(QString("Plugin %1 already loaded.").arg(info.name));
		return true;
	}
	m_d->plugin = 0;
	
	m_d->loader.setFileName(info.path + "/" + info.entry);
	QObject *object = m_d->loader.instance();
	if(object == 0) {
		m_d->log->warning(QString("Could not instantiate plugin %1: %2")
		                  .arg(info.name).arg(m_d->loader.errorString()));
		m_d->loader.unload();
		return false;
	}
	
	CXXPluginInterface *plugin = qobject_cast<CXXPluginInterface *>(object);
	if(plugin == 0) {
		m_d->log->warning(QString("Plugin %1 does not implement interface.").arg(info.name));
		delete object;
		m_d->loader.unload();
		return false;
	}
	
	bool result = plugin->run();
	if(result == 0) {
		m_d->log->warning(QString("Plugin %1 failed to initialize.").arg(info.name));
		delete object;
		m_d->loader.unload();
		return false;
	}
	
	connect(object, SIGNAL(destroyed()), SLOT(instanceDestroyed()));
	
	m_d->plugin = plugin;
	m_d->plugin->handleMessage(args);
	m_d->plugin->activate();
	m_d->info = info;
	return true;
}

bool CXXPluginLoader::activatePlugin() {
	
	if (!m_d->plugin) {
		return false;
	}
	m_d->plugin->activate();
	return true;
}

bool CXXPluginLoader::sendMessage(const QStringList &args) {

	if (!m_d->plugin) {
		return false;
	}
	m_d->plugin->handleMessage(args);
	return true;
}

bool CXXPluginLoader::stopPlugin() {
	
	if (!m_d->plugin) {
		return true;
	}
	return m_d->plugin->acceptStopEvent();
}

bool CXXPluginLoader::unloadPlugin() {
	
	if (m_d->plugin) {
		QObject *object = dynamic_cast<QObject *>(m_d->plugin);
		Q_ASSERT(object);
		object->disconnect(this);
		
		delete m_d->plugin;
		m_d->plugin = 0;
		
		m_d->log->debug(QString("Plugin %1 unloaded.").arg(m_d->info.name));
		return true;//m_d->loader.unload();
	} else {
		m_d->log->debug(QString("Plugin %1 already unloaded.").arg(m_d->info.name));
		return false;
	}
}

CXXPluginInterface *CXXPluginLoader::pluginInterface() const {

	return m_d->plugin;
}

QObject *CXXPluginLoader::pluginObject() const {

	return dynamic_cast<QObject *>(m_d->plugin);
}

void CXXPluginLoader::instanceDestroyed() {
	
	m_d->log->debug(QString("Instance of plugin %1 destroyed.").arg(m_d->info.name));
	
	m_d->plugin = 0;
	//m_d->loader.unload();
	
	deleteLater();
	
	emit terminated(m_d->info.name);
}

} // namespace Plugger
