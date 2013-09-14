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

#include <QtXml>
#include <QApplication>
#include <QDesktopServices>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QPluginLoader>

#include "extender/Extender.h"
#include "filer/Filer.h"
#include "logger/Logger.h"
#include "zipper/Zipper.h"
#include "CXXPluginLoader.h"
#include "Plugger.h"
#include "config.h"

namespace Plugger {

Plugger *Plugger::s_instance = 0;

struct Plugger::Private {
	Logger::Logger *log;
	
	bool initialized;
	
	QStringList installedPlugins;
	PluginMap pluginMap;
	QStringList pluginDirs;
	
	ErrorCode lastError;
	
	Private() :
		log(0),
		initialized(false),
		lastError(NoError) {
	}
	
	~Private() {
		
		delete log;
	}
};

Plugger *Plugger::instance() {
	
	if (s_instance == 0) {
		s_instance = new Plugger();
	}
	
	return s_instance;
}

void Plugger::destroy() {
	
	delete s_instance;
	s_instance = 0;
}

void Plugger::initialize() {
	
	if (m_d->pluginDirs.isEmpty()) {
		
		QString dataLocation = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#if QT_VERSION == 0x040500
#ifdef Q_WS_MAC
		dataLocation = QDesktopServices::storageLocation(QDesktopServices::HomeLocation) + "/Library/Application Support/Hyves Desktop";
#endif
#endif
		QDir dir(dataLocation);
		if (!dir.exists()) {
			dir.mkpath(".");
		}
		if (!dir.exists("plugins")) {
			if (!dir.mkdir("plugins")) {
				m_d->log->error("Cannot create plugin directory; installing of plugins disabled.");
			}
		}
		m_d->pluginDirs.append(dir.canonicalPath() + "/plugins");
	} else {
		QDir dir(m_d->pluginDirs[0]);
		if (!dir.exists()) {
			m_d->log->error(QString("Specified plugins directory %1 does not exist; "
			                        "installing of plugins disabled.").arg(m_d->pluginDirs[0]));
			m_d->pluginDirs.removeAt(0);
		}
	}
	
	QDir appDir(QApplication::applicationDirPath());
	m_d->pluginDirs.append(appDir.canonicalPath() + "/plugins");
	
	m_d->initialized = true;
	
	loadPlugins();
}

void Plugger::setPluginDir(QString pluginDir) {
	
	if (!m_d->initialized) {
		m_d->pluginDirs.prepend(pluginDir);
	}
}

bool Plugger::installPlugin(QString filePath) {
	
	if (m_d->pluginDirs.size() < 2) {
		m_d->log->warning("Installing of plugins is disabled.");
		return false;
	}
	
	QByteArray manifestContent = Zipper::readFileFromArchive(filePath, "/manifest.xml");
	if (manifestContent.isEmpty()) {
		m_d->log->warning(QString("Cannot read manifest file from %1.").arg(filePath));
		m_d->lastError = FileNotFound;
		return false;
	}
	
	PluginInfo info;
	info.loadManifestData(manifestContent);
	
	if (m_d->pluginMap.contains(info.name)) {
		m_d->log->warning(QString("Plugin %1 already installed.").arg(info.name));
		m_d->lastError = PluginAlreadyInstalled;
		return false;
	}
	
	QDir pluginDir(m_d->pluginDirs[0]);
	bool result = pluginDir.mkdir(info.name);
	
	if (!result) {
		m_d->log->warning(QString("Cannot create plugin directory for %1.").arg(info.name));
		m_d->lastError = CannotOpenForWriting;
		return false;
	}
	
	result = Zipper::unzipArchive(filePath, m_d->pluginDirs[0] + "/" + info.name);	
	
	if (!result) {
		m_d->log->warning(QString("Error while unpacking package for %1.").arg(info.name));
		m_d->lastError = ErrorUnpackingArchive;
		return false;
	}
	
	info.path = m_d->pluginDirs[0] + "/" + info.name;
	m_d->pluginMap[info.name] = info;
	m_d->installedPlugins.append(info.name);
	
	return loadPlugin(info.name);
}

bool Plugger::uninstallPlugin(QString pluginName) {
	
	if (m_d->pluginDirs.size() < 2) {
		m_d->log->warning("Uninstalling of plugins is disabled.");
		return false;
	}
	
	bool result = unloadPlugin(pluginName);
	if (!result) {
		return false;
	}
	
	PluginInfo &info = m_d->pluginMap[pluginName];
	
	Filer::removeDirectory(info.path);
	
	m_d->installedPlugins.removeAll(info.name);
	m_d->pluginMap.remove(info.name);
	
	return true;
}

QStringList Plugger::installedPlugins() const {
	
	return m_d->installedPlugins;
}

PluginInfo Plugger::pluginInfo(QString pluginName) {
	
	if (m_d->pluginMap.contains(pluginName)) {
		return m_d->pluginMap[pluginName];
	}
	
	return PluginInfo();
}

bool Plugger::setAutoStart(QString pluginName, bool autoStart) {
	
	if (!m_d->pluginMap.contains(pluginName)) {
		m_d->lastError = UnknownPlugin;
		return false;
	}
	
	PluginInfo &info = m_d->pluginMap[pluginName];
	info.autoStart = autoStart;
	bool result = info.saveManifest();
	
	if (!result) {
		m_d->log->warning(QString("Cannot write manifest file for plugin %1.").arg(pluginName));
		m_d->lastError = CannotOpenForWriting;
		return false;
	}
	
	return true;
}

ErrorCode Plugger::lastError() const {
	
	return m_d->lastError;
}

QString Plugger::errorMessage(ErrorCode errorCode) {
	
	switch (errorCode) {
		case NoError:
			return tr("No error");
			
		case FileNotFound:
			return tr("File not found");
			
		case UnknownPlugin:
			return tr("Unknown plugin");
			
		case CouldNotLinkPlugin:
			return tr("Link error while loading plugin");
			
		case CouldNotInstantiatePlugin:
			return tr("Could not instantiate plugin");
			
		case CannotOpenForWriting:
			return tr("Cannot open file for writing");
			
		case ErrorUnpackingArchive:
			return tr("Error while unpacking archive");
			
		case PluginAlreadyInstalled:
			return tr("Plugin already installed");
			
		default:
			return tr("Unknown error");
	}
}

bool Plugger::runPlugin(QString pluginName, QStringList args) {
	
	return loadPlugin(pluginName, args);
}

bool Plugger::stopPlugin(QString pluginName) {
	
	return unloadPlugin(pluginName);
}

bool Plugger::isRunning(QString pluginName) const {
	
	if (!m_d->pluginMap.contains(pluginName)) {
		m_d->lastError = UnknownPlugin;
		return false;
	}
	
	return (m_d->pluginMap[pluginName].loader != 0);
}

Plugger::Plugger() :
	QObject(),
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("Plugger");
	
	Extender::Extender::instance()->registerObject("plugger", this);
}

Plugger::~Plugger() {
	
	unloadPlugins();
	
	delete m_d;
}

void Plugger::loadPlugins() {
	
	foreach (QString path, m_d->pluginDirs) {
		loadPluginDirectory(path);
	}
}

void Plugger::loadPluginDirectory(QString path) {
	
	QDir dir(path);
	QStringList entries = dir.entryList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
	for (QStringList::ConstIterator it = entries.begin(); it != entries.end(); ++it) {
		
		PluginInfo info;
		QString pluginPath = path + "/" + *it;
		bool result = info.loadManifest(pluginPath);
		
		if (!result) {
			m_d->log->warning(QString("Error loading manifest file; plugin %1 skipped.").arg(pluginPath));
			continue;
		}
		
		if (!info.path.endsWith("/" + info.name)) {
			m_d->log->warning(QString("Name specified does not match directory; plugin %1 skipped.").arg(pluginPath));
			continue;
		}
		
		Q_ASSERT(pluginPath == info.path);
		
		if (m_d->pluginMap.contains(info.name)) {
			m_d->log->warning(QString("Plugin %1 already loaded; skipped.").arg(info.name));
			continue;
		}
		
		m_d->pluginMap[info.name] = info;
		m_d->installedPlugins.append(info.name);
		
		if (info.registerEntryPoint) {
			emit registerEntryPoint(info.name, info.displayName);
		}
		
		if (info.autoStart) {
			loadPlugin(info.name);
		}
	}
}

bool Plugger::loadPlugin(QString name, QStringList args) {
	
	if (!m_d->pluginMap.contains(name)) {
		m_d->lastError = UnknownPlugin;
		return false;
	}
	
	PluginInfo &info = m_d->pluginMap[name];
	
	if (info.loader) {
		info.loader->sendMessage(args);
		return info.loader->activatePlugin();
	}
	
	PluginLoader *loader;
	if (info.type == "application/x-c++-code") {
		loader = new CXXPluginLoader();
	} else {
		m_d->log->warning(QString("Plugin %1 specified unknown type; skipped.").arg(info.name));
		return false;
	}
	
	bool result = loader->loadPlugin(info, args);
	
	if (result) {
		info.loader = loader;
		
		connect(loader, SIGNAL(terminated(QString)), SLOT(pluginTerminated(QString)));
	} else {
		m_d->lastError = CouldNotInstantiatePlugin;
	}
	
	return result;
}

void Plugger::unloadPlugins() {
	
	for (PluginMap::ConstIterator it = m_d->pluginMap.begin();
	     it != m_d->pluginMap.end(); ++it) {
		unloadPlugin(it.key());
	}
}

bool Plugger::unloadPlugin(QString name) {
	
	if (!m_d->pluginMap.contains(name)) {
		m_d->lastError = UnknownPlugin;
		return false;
	}
	
	PluginInfo &info = m_d->pluginMap[name];
	
	if (info.loader == 0) {
		m_d->log->debug(QString("Plugin %1 already unloaded.").arg(name));
		return true; // already unloaded is considered success
	}
	
	if (!info.loader->stopPlugin()) {
		return false; // the plugin refuses to stop
	}
	
	bool result = info.loader->unloadPlugin();
	
	info.loader->disconnect(this);
	delete info.loader;
	info.loader = 0;
	
	return result;
}

void Plugger::pluginTerminated(const QString &pluginName) {
	
	if (!m_d->pluginMap.contains(pluginName)) {
		m_d->log->warning(QString("Catched termination of non-registered plugin %1.").arg(pluginName));
		return;
	}
	
	m_d->log->debug(QString("Plugin %1 terminated").arg(pluginName));
	
	PluginInfo &info = m_d->pluginMap[pluginName];
	info.loader = 0;
}

} // namespace Plugger
