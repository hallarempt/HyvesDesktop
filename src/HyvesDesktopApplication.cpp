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

#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QString>
#include <QWebSettings>
#include <QDesktopServices>
#include <QWidget>

#include <QtCrypto>

#include "config.h"
#include "contextmenumanager/ContextMenuManager.h"
#include "extender/Extender.h"
#include "historykeeper/HistoryKeeper.h"
#include "hyvesapi/HyvesAPI.h"
#include "informer/Informer.h"
#include "jabber/Jabber.h"
#include "logger/Logger.h"
#include "logger/JSLogger.h"
#include "logger/UsageStatsUploader.h"
#include "mediacache/MediaCache.h"
#include "player/Player.h"
#include "plugger/Plugger.h"
#include "tester/Tester.h"
#include "translator/Translator.h"
#include "roster/Roster.h"
#include "settingsmanager/SettingsManager.h"
#include "systemtrayicon/SystemTrayIcon.h"
#include "updater/Updater.h"
#include "windowmanager/WindowManager.h"
#include "HyvesDesktopApplication.h"
#include "NetworkAccessManager.h"
#include "WebPluginFactory.h"
#include "systemspy/SystemSpy.h"

#ifdef Q_WS_MAC
#include "LoginItemsAE.h"
#endif

struct HyvesDesktopApplication::Private {
	Logger::Logger *log;
	Logger::JSLogger *jslog;
	Logger::UsageStatsUploader *uploader;
	
	Private() :
		log(0),
		jslog(0),
		uploader(0) {
	}
	
	~Private() {
		
		delete log;
		delete jslog;
	}
	
	static QPair<QString, QStringList> plugin(QStringList args) {
		
		QString pluginName = "kwekker"; // the default plugin to run
		QStringList pluginArgs;
		
		// works like this: $ Hyvesdesktop.shell [other args] [--plugin name [plugin-option1] [plugin-option2] ...]
		int index = args.indexOf("--plugin");
		if (index > 0 && index < args.size() - 1) {
			pluginName = args.at(index + 1);
			if (index < args.size() - 2) {
				pluginArgs = args.mid(index + 2);
			}
		}
		
		return QPair<QString, QStringList>(pluginName, pluginArgs);
	}
};

HyvesDesktopApplication::HyvesDesktopApplication(int &argc, char *argv[], const QString &uniqueKey) :
	QtSingleApplication(uniqueKey, argc, argv),
	m_d(new Private()) {
	
	setApplicationName(HD_PRODUCT_NAME);
	setOrganizationName(HD_ORGANIZATION_NAME);
	setOrganizationDomain(HD_PRODUCT_URL);
	setApplicationVersion(HD_VERSION.toString());
	
	QStringList args = arguments();
	setLanguage(args);
	
	if (isRunning()) {
		sendArgs(args);
		std::exit(0); // quit this instance
	}
	
	m_d->log = new Logger::Logger("HyvesDesktopApplication");
	m_d->jslog = new Logger::JSLogger();
	m_d->uploader = new Logger::UsageStatsUploader(qApp);
	connect(m_d->uploader, SIGNAL(finished()), SLOT(setNextStatsUpload()));
	
	connect(SettingsManager::SettingsManager::instance(), SIGNAL(settingChanged(const QString &)), this, SLOT(writeSettings(const QString &)));
	connect(this, SIGNAL(messageReceived(const QString &)), SLOT(parseMessage(const QString &)));
	
	if (args.contains("--kill")) {
		if (doUploadTest(args)) {
			disconnect(m_d->uploader, SIGNAL(finished()), this, SLOT(setNextStatsUpload()));
			connect(m_d->uploader, SIGNAL(finished()), SLOT(quit()));
			connect(m_d->uploader, SIGNAL(failed()), SLOT(quit()));
			m_d->log->notice("Shutdown delayed: waiting for UsageStatsUploader...");
			return;
		}
		
		m_d->log->notice("No other instance running, goodbye...");
		SettingsManager::SettingsManager::instance()->destroy();
		std::exit(0);
	}
	
	init(args);
}

bool HyvesDesktopApplication::doUploadTest(const QStringList &args) {
	
	int uploadTest = args.indexOf("--uploadtest");
	if (uploadTest == -1) {
		return false;
	}
	
	QString dataLocation = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
	QString logFileLocation = dataLocation + "/log/session.txt";
	
	// do not change/remove the next log message ('teststatsupload.py' is testing against it)
	m_d->log->test(QString("If_you_can_read_this_on_the_server_the_logfile_has_not_been_truncated_before_uploading!!!"));
	for (int i = 0; i < 2000; i++) {
		m_d->log->test(QString("Flooding the logfile on purpose to make sure it has to be truncated for uploading (%1)").arg(i));
	}
	// use the next argument as key to allow identification by the tester-script
	if (uploadTest < args.size() - 1) {
		m_d->log->notice("Uploadtest key: " + args[uploadTest + 1]);
	}
	
	Tester::Tester::instance();
	m_d->uploader->uploadUsageStats(logFileLocation, false);
	return true;
}

HyvesDesktopApplication::~HyvesDesktopApplication() {
	
	SystemSpy::SystemSpy::destroy();
	Plugger::Plugger::destroy();
	ContextMenuManager::ContextMenuManager::destroy();
	SystemTrayIcon::SystemTrayIcon::destroy();
	HistoryKeeper::HistoryKeeper::destroy();
	HyvesAPI::HyvesAPI::destroy();
	Informer::Informer::destroy();
	Roster::Roster::destroy();
	Jabber::Jabber::destroy();
	Player::Player::destroy();
	MediaCache::MediaCache::destroy();
	WindowManager::WindowManager::destroy();
	Translator::Translator::destroy();
	SettingsManager::SettingsManager::destroy();
	Extender::Extender::destroy();
	NetworkAccessManager::destroy();
	WebPluginFactory::destroy();
	Updater::Updater::destroy();
	
	delete m_d;
	
	Logger::Logger::destroy();
}

void HyvesDesktopApplication::quit() {
	
	Plugger::Plugger *plugger = Plugger::Plugger::instance();
	
	foreach (QString pluginName, plugger->installedPlugins()) {
		if (!plugger->stopPlugin(pluginName)) {
			return;
		}
	}
	
	QtSingleApplication::quit();
}

void HyvesDesktopApplication::doStatsUpload() {
#ifdef DEBUG_BUILD
	m_d->log->debug("UsageStatsUploading disabled (debug build)");
#else
	QString logLocation = QDesktopServices::storageLocation(QDesktopServices::DataLocation).append("/log");
	if (Tester::Tester::isTesting()) {
		m_d->log->debug("Not uploading usage stats while Tester is running");
		return;
	}
	
	// send last session log if errors were encountered
	if (QFile::exists(logLocation + "/session.txt.last")) {
		m_d->uploader->uploadUsageStats(logLocation + "/session.txt.last", true);
		return;
	}
	
	// do monthly stats upload
	uint nextUpload = SettingsManager::SettingsManager::instance()->intValue("Statistics/nextUsageStatsUpload", 0);
	if (!nextUpload) {	// no upload scheduled yet
		setNextStatsUpload();
		return;
	}
	if (QDateTime::currentDateTime().toTime_t() > nextUpload && !Tester::Tester::isTesting()) {
		// if 'usage.txt.last' doesn't exist (which means we haven't collected 200kB usage stats yet) rotate log and send what we got so far
		if (!QFile::exists(logLocation + "/usage.txt.last")) {
			m_d->log->rotateUsageLog();
		}
		m_d->uploader->uploadUsageStats(logLocation + "/usage.txt.last", true);
	}
	
	m_d->log->notice(QString("Next usage statistics upload scheduled for %1").arg( QDateTime::fromTime_t(nextUpload).toString()));
#endif
}

void HyvesDesktopApplication::setNextStatsUpload() {
	
	// next upload will be in ~1 month (+- 7 days)
	qsrand(1);
	unsigned int nextUpload = QDateTime::currentDateTime().addMonths(1).addDays(rand() % 14 - 7).toTime_t();
	SettingsManager::SettingsManager::instance()->setValue("Statistics/nextUsageStatsUpload", nextUpload , 0);
	m_d->log->notice(QString("Scheduling next usage stats upload for %1").arg(QDateTime::fromTime_t(nextUpload).toString()));
}
	
void HyvesDesktopApplication::sendArgs(const QStringList &args) {
	
	if (sendMessage(args.join("\n"))) {
		qDebug() << QString("Message '%1' sent to previous instance").arg(args.join(" "));
	}
}
	
void HyvesDesktopApplication::init(const QStringList &args)  {
	
	QDir::setCurrent(applicationDirPath());
	
#ifdef Q_WS_MAC 
	// The mac shows tooltips for the systray menu, which is bad, 
	// but this at least makes them go away pretty quickly.
	setEffectEnabled(Qt::UI_FadeTooltip, false);
	setEffectEnabled(Qt::UI_AnimateTooltip, false);
#endif

	initApplicationIcons();
	
	QWebSettings::setMaximumPagesInCache(0);
	
	if (args.contains("--test")) {
		Tester::Tester::instance();
	} else if (args.contains("--clear-settings")) {
		SettingsManager::SettingsManager::instance()->clearSettings();
	} else {
		Jabber::Jabber::instance();
	}
	
	ContextMenuManager::ContextMenuManager::instance();
	HistoryKeeper::HistoryKeeper::instance();
	HyvesAPI::HyvesAPI::instance();
	Informer::Informer::instance();
	Player::Player::instance();
	Roster::Roster::instance();
	SettingsManager::SettingsManager::instance();
	
	if (SettingsManager::SettingsManager::instance()->boolValue("firstrun", true)) {
		enableAutostart();
	}
	
	Translator::Translator::instance();
	Plugger::Plugger::instance();
	
	doSystemChecks();
	
	setQuitOnLastWindowClosed(false);
	
	MediaCache::MediaCache::instance();
	SystemTrayIcon::SystemTrayIcon::instance();
	Plugger::Plugger::instance()->initialize();
	Updater::Updater::instance();
	
	if (args.contains("--silent-update")) {
		Updater::Updater::instance()->setUseSilentUpdate(true);
	}
	
	if (args.contains("--server-update-path")) {
		int index = args.indexOf("--server-update-path");
		if (index + 1 < args.count()) {
			Updater::Updater::instance()->setServerUpdatePath(args.at(index + 1));
		}
	}
	Updater::Updater::instance()->checkForUpdates();
	
	connect(Updater::Updater::instance(), SIGNAL(updateAvailable()), SLOT(updateAvailable()));
	
	Plugger::Plugger *plugger = Plugger::Plugger::instance();
	QPair<QString, QStringList> pair = Private::plugin(args);
	if (!plugger->runPlugin(pair.first, pair.second)) {
		m_d->log->warning("Could not start plugin: " + plugger->errorMessage(plugger->lastError()));
	}
	
	doStatsUpload();
}

void HyvesDesktopApplication::initApplicationIcons() {
	
	QIcon appIcon;
	appIcon.addFile(":/data/appicon_16_r.png", QSize(16, 16), QIcon::Normal);
	appIcon.addFile(":/data/appicon_22_r.png", QSize(22, 22), QIcon::Normal);
	appIcon.addFile(":/data/appicon_24_r.png", QSize(24, 24), QIcon::Normal);
	appIcon.addFile(":/data/appicon_32_r.png", QSize(32, 32), QIcon::Normal);
	appIcon.addFile(":/data/appicon_64_r.png", QSize(64, 64), QIcon::Normal);
	appIcon.addFile(":/data/appicon_128_r.png", QSize(128, 128), QIcon::Normal);
	setWindowIcon(appIcon);
}

void HyvesDesktopApplication::doSystemChecks() {
	
	if (!QSystemTrayIcon::isSystemTrayAvailable()) {
		QMessageBox::critical(0, HD_PRODUCT_NAME,
		                         QObject::tr("Could not detect any system tray on this system. "
		                                     "%1 will not run!").arg(HD_PRODUCT_NAME));
		std::exit(1);
	}
	
	if (!QCA::isSupported("hmac(sha1)")) {
		QMessageBox::critical(0, HD_PRODUCT_NAME,
		                      tr("It appears there is an installation problem because of "
		                    	 "missing cryptography support. %1 will not run!").arg(HD_PRODUCT_NAME));
		std::exit(1);
	}
	
#ifdef Q_WS_MAC
	if (QSysInfo::MacintoshVersion < QSysInfo::MV_10_5) {
		QMessageBox::critical(0, HD_PRODUCT_NAME,
		                      tr("%1 requires OS X version 10.5 (Leopard) or above. %1 will not run!").arg(HD_PRODUCT_NAME));
		std::exit(1);
	}
	
#ifndef QT_NO_DBUS	
#error "You are using a Qt compiled with DBus on OSX."
#endif
#endif
}

bool HyvesDesktopApplication::event(QEvent *event) {

#ifdef Q_WS_MAC
	
	if (event->type() == QEvent::ApplicationActivate) {

		foreach (QWidget *widget, QApplication::topLevelWidgets()) {
			if (widget->isVisible()) {
				m_d->log->debug("Visible window found");
				// there is already a window visible, so we don't have to raise the chat window
				return QApplication::event(event);
			}
		}
		QList<QWidget *> windowList = WindowManager::WindowManager::instance()->windows();
		foreach(QWidget *window, windowList) {
			if (window->isHidden()) {
				m_d->log->debug("hidden window found");				
				window->show();
				QApplication::setActiveWindow(window);
			}
		}
	}
#endif
	return QApplication::event(event);
}

void HyvesDesktopApplication::enableAutostart() {
	
#ifdef Q_WS_WIN	
	QSettings regKey("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
	regKey.setValue("HyvesDesktop.exe", applicationFilePath().replace("/", "\\"));
#endif
#ifdef Q_WS_MAC
	CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
	LIAEAddURLAtEnd(appUrlRef, false);
#endif
}

void HyvesDesktopApplication::disableAutostart() {
	
#ifdef Q_WS_WIN
	QSettings regKey("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
	regKey.remove("HyvesDesktop.exe");
#endif
#ifdef Q_WS_MAC
	CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
	LIAERemoveURL(appUrlRef);
#endif
}

void HyvesDesktopApplication::writeSettings(const QString &key) {
	
	if (key != "General/Autostart") {
		return;
	}
	
	if (SettingsManager::SettingsManager::instance()->boolValue(key, true)) {
		enableAutostart();
	} else {
		disableAutostart();
	}
}

void HyvesDesktopApplication::startPlugin(const QPair<QString, QStringList> &plugin) {
	
	Plugger::Plugger *plugger = Plugger::Plugger::instance();
	if (plugin.first.isEmpty()) {
		m_d->log->debug("No plugin to be started (empty argument)");
		return;
	}
	
	if (!plugger->runPlugin(plugin.first, plugin.second)) {
		m_d->log->warning("Could not start plugin '" + plugin.first + "': " + plugger->errorMessage(plugger->lastError()));
	}
}

void HyvesDesktopApplication::setLanguage(const QStringList &args) {
	
	int index = args.indexOf("--language");
	if (index > 0 && index < args.size() - 1) {
		qDebug() << "Setting language to " << args.at(index + 1);
		SettingsManager::SettingsManager::instance()->setValue("language", args.at(index + 1));
	}
}

void HyvesDesktopApplication::parseMessage(const QString &message) {
	
	QStringList args = message.split("\n");
	
	m_d->log->notice("Received message from another instance: " + args.join(" "));
	// check for kill switch...
	if (args.contains("--kill")) {
		m_d->log->notice("Received kill switch, goodbye...");
		quit();
		return;
	}
	
	// check for plugin...
	startPlugin(Private::plugin(args));
}

void HyvesDesktopApplication::updateAvailable() {
	
	bool applyUpdate = false;
	if (Updater::Updater::instance()->forceUpdates()) {
		applyUpdate = true;
	} else {
		QMessageBox::StandardButton buttonClicked = QMessageBox::question(0, HD_PRODUCT_NAME,
			tr("A new update is available for %1. Do you want to install the update now?\n"
			   "If you select No, the update will be installed the next time you start %1.").arg(HD_PRODUCT_NAME),
			QMessageBox::Yes | QMessageBox::No
		);
		
		if (buttonClicked == QMessageBox::Yes) {
			applyUpdate = true;
		}
	}
	
	if (applyUpdate) {
		QApplication::exit(Updater::InstallUpdateExitCode);
	}
}
