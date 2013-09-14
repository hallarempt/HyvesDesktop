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
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QDir>
#include <QDateTime>
#include <QDomDocument>
#include <QFile>
#include <QTemporaryFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QTimer>
#include <QUrl>
 
#include "config.h"
#include "extender/Extender.h"
#include "filer/Filer.h"
#include "logger/Logger.h"
#include "settingsmanager/SettingsManager.h"
#include "Updater.h"
#include "UrlFetcher.h"
#include "Version.h"


const int MINUTE = 1000 * 60;
const int HOUR = MINUTE * 60;
const int DAY = HOUR * 24;

#ifdef Q_WS_WIN
#include <cstring>
#include <windows.h>

/**
* Native Win32 method for starting a process. This is required in order to
* launch the installer with User Account Control enabled.
* 
* @param path Path to the process to start.
* @param parameters Parameters for the process.
* @return @c true if the process is started successfully, @c false otherwise.
*/
bool startProcess(char *path, char *parameters = 0) {

	Q_ASSERT(path != 0);
	
	SHELLEXECUTEINFO info;
	memset(&info, '\0', sizeof(info));
	
	info.cbSize = sizeof(info);
	info.fMask = 0;
	info.hwnd = 0;
	info.lpVerb = TEXT("open");
	info.lpFile = path;
	info.lpParameters = parameters;
	info.lpDirectory = 0;
	info.nShow = SW_SHOWNORMAL;
	return ShellExecuteEx(&info);
}
#endif

namespace Updater {

static const QString UPDATE_HOST = "download.hyves.org";
static const QString UPDATE_PATH = "kwekker2";

Updater *Updater::s_instance = 0;

struct Updater::Update {
	Version	version;
	QString path;
	QString md5;
};

struct Updater::Private {
	Logger::Logger *log;
	
	QTimer timer;
	UrlFetcher *versionFetcher;
	UrlFetcher *updateFetcher;
	
	QString updateFilename;
	Update  update;
	
	QProcess *updateProcess;
	bool updateProcessStarted;
	
	int errorCount;
	
	bool useSilentUpdate;
	QString updatePath;
	
	bool downloadHqUpdate;
	bool downloadPreUpdate;
	
	Private() :
		log(0),
		versionFetcher(0),
		updateFetcher(0),
		updateProcess(0),
		errorCount(0),
		useSilentUpdate(false),
		updatePath(UPDATE_PATH),
		downloadHqUpdate(false),
		downloadPreUpdate(false) {
	}
	
	~Private() {
		delete updateProcess;
		delete updateFetcher;
		delete versionFetcher;
		delete log;
	}
};

Updater *Updater::instance() {
	
	if (s_instance == 0) {
		s_instance = new Updater();
	}
	
	return s_instance;
}

void Updater::destroy() {
	
	delete s_instance;
	s_instance = 0;
}

bool Updater::forceUpdates() const {
	
	return m_d->downloadHqUpdate || m_d->useSilentUpdate;
}

void Updater::setUseSilentUpdate(bool useSilentUpdate) {
	
	m_d->useSilentUpdate = useSilentUpdate;
}

void Updater::setServerUpdatePath(const QString& path) {
	
	m_d->updatePath = path;
}

void Updater::setDownloadHqUpdate(bool downloadHqUpdate) {
	
	if (downloadHqUpdate) {
		m_d->log->notice("Checking for HQ-only update.");
	}
	m_d->downloadHqUpdate = downloadHqUpdate;
}

void Updater::setDownloadPreUpdate(bool downloadPreUpdate) {
	
	if (downloadPreUpdate) {
		m_d->log->notice("Checking for pre-update.");
	}
	m_d->downloadPreUpdate = downloadPreUpdate;
}

void Updater::checkForUpdates() {
	
	emit checkingForUpdates();
	
	// first detect an already downloaded update, ready for installation
	QString dataLocation = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#ifdef Q_OS_MAC
	dataLocation.replace("\\", "\\\\");
#endif
	QString availableUpdateLocation = dataLocation;
	QString updateFilePath = availableUpdateLocation + "/" + m_d->updateFilename;
	if (QFile::exists(updateFilePath)) {
		QString installableFilePath = updateFilePath + ".installable";
		if (QFile::exists(installableFilePath)) {
			QFile installableFile(installableFilePath);
			QByteArray contents = installableFile.readAll();
			Version installableVersion(contents);
			if (installableVersion > HD_VERSION) {
				m_d->log->notice(QString("Found available update: %1 -> %2").arg(HD_VERSION.toString()).arg(installableVersion.toString()));
				emit updateAvailable();
				return;
			}
		}
		
		// if update installer exists, but installable file doesn't, or
		// its version is too low, the update has already been
		// installed
		QFile::remove(updateFilePath);
		
		// now we still continue to check for an even newer one
	}
	
	QUrl url;
	url.setScheme("http");
	url.setHost(UPDATE_HOST);
	
	url.setPath("/" + m_d->updatePath + "/updates.xml");
	
	m_d->log->notice(QString("Now checking for updates on %1.")
			.arg(url.toString()));

	m_d->versionFetcher = new UrlFetcher(url, UrlFetcher::FetchString, this);
	connect(m_d->versionFetcher, SIGNAL(replyReady(const QString &)), SLOT(updateCheckResult(const QString &)));
}

void Updater::installAvailableUpdate() {
	
	QString dataLocation = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
	QString availableUpdateLocation = dataLocation;
	QString updateFile = availableUpdateLocation + "/" + m_d->updateFilename;
	
	m_d->log->debug("Installing update file " + m_d->log->anonymize(updateFile, Logger::File));
	
	if (!QFile::exists(updateFile)) {
		return;
	}
	
	// if update installer exists, but installable file doesn't, the update has
	// already been installed
	QString installableFile = updateFile + ".installable";
	if (!QFile::exists(installableFile)) {
		QFile::remove(updateFile);
		return;
	}
	
	QFile::remove(installableFile);
	m_d->log->notice("Now installing latest update.");
	
#ifdef Q_WS_WIN
	QDir applicationDir(qApp->applicationDirPath());
	//go up from bin directory
	applicationDir.cdUp();
	QString paramString;
	if (m_d->useSilentUpdate) {
		paramString = "--mode unattended --install-type update --prefix \"" + applicationDir.canonicalPath() +"\"";
	} else {
		paramString =  "--install-type update --prefix \"" + applicationDir.canonicalPath() +"\"";
	}
	QByteArray byteArray = paramString.toLatin1();
	char *parameters = byteArray.data();
	
	if (!startProcess(updateFile.replace("/", "\\").toLatin1().data(), parameters)) {
		QMessageBox::warning(0, HD_PRODUCT_NAME,
			tr("Failed to start the update. Please restart %1.").arg(HD_PRODUCT_NAME)
		);
	}
#else
	m_d->updateProcess = new QProcess(this);
	m_d->updateProcess->setEnvironment(QProcess::systemEnvironment());
	connect(m_d->updateProcess, SIGNAL(error(QProcess::ProcessError)),
	        SLOT(processError(QProcess::ProcessError)));
	
	QProcess restartProcess;
	QString restartCommand;
	
	QString tmpPath = QDir::tempPath() + "/HyvesDesktopUpdate" + QDateTime::currentDateTime().toString("yyMMdd-hhmmsszzz");
	
	// Check whether the HyvesDesktopUpdate dir already exists
	if (QDir(tmpPath).exists()) {
		m_d->log->debug("Deleting update path " + m_d->log->anonymize(tmpPath, Logger::File));
		Filer::removeDirectory(tmpPath);
	}
	QDir().mkpath(tmpPath + "/new");
	
#ifdef Q_WS_MAC
	QDir bundleDir(qApp->applicationDirPath());
	bundleDir.cdUp();
	bundleDir.cdUp();
	QString s;
	QStringList args;
	
	// Start with a clean slate
	if (QDir("/Volumes/Hyves Desktop").exists()) {
		args << "detach" << "/Volumes/Hyves Desktop";
		m_d->log->debug(QString("unmounting old dmg-file: hdiutil '%1'").arg(args.join("' '")));
		QProcess::execute("hdiutil", args);
	}
	
	QDir().mkpath(tmpPath + "/orig/");
	
	// Step 1: mount the dmg: hdiutil attach "updateFile -noautoopen
	args.clear();
	args << "attach" << updateFile << "-noautoopen";
	m_d->log->debug(QString("Mounting dmg file: hdiutil '%1'").arg(args.join("' '")));
	int exitCode = QProcess::execute("hdiutil", args);
	if (exitCode) {
		m_d->log->error(QString("Failed to mount dmg: hdiutil returned %1").arg(exitCode));
		m_d->log->error("Update failed.");
		QMessageBox::critical(0, HD_PRODUCT_NAME, tr("Oops, the updater encountered a problem and the application has not been updated.\n\n"
													 "Please try and download the latest version from http://www.hyves.nl/hyvesdesktop/download "
													 "or contact us at http://www.hyves.nl/help"));
		return;
	}
	
	// Step 2: move the active bundle into a backup place
	args.clear();
	args << bundleDir.absolutePath() << tmpPath + "/orig/HyvesDesktop.app";
	m_d->log->debug(QString("Backing up previous installation: mv '%1'").arg(args.join("' '")));
	exitCode = QProcess::execute("mv", args);
	if (exitCode) {
		m_d->log->error(QString("Failed to backup old HyvesDesktop installation: mv returned %1").arg(exitCode));
		QMessageBox::critical(0, HD_PRODUCT_NAME, tr("Oops, the updater encountered a problem and the application has not been updated.\n\n"
													 "Please try and download the latest version from http://www.hyves.nl/hyvesdesktop/download "
													 "or contact us at http://www.hyves.nl/help"));
		return;
	}
	
	// Step 3: copy from the dmg: ditto /Volumes/Hyves Desktop/Hyves Desktop.app oldBundleLocation
	bool doRollback = false;
	args.clear();
	if (QDir().exists("/Volumes/Hyves Desktop/Hyves Desktop.app")) {
		args << "/Volumes/Hyves Desktop/Hyves Desktop.app";
	} else if (QDir().exists("/Volumes/Hyves Desktop/HyvesDesktop.app")) {
		args << "/Volumes/Hyves Desktop/HyvesDesktop.app";
	} else {
		m_d->log->error("Could not find HyvesDesktop.app in dmg file.");
		doRollback = true;
	}
	
	if (!args.isEmpty()) {
		args << bundleDir.absolutePath();
		m_d->log->debug(QString("Copying application bundle from dmg: ditto '%1'").arg(args.join("' '")));
		exitCode = QProcess::execute("ditto", args);
		if (exitCode) {
			m_d->log->error(QString("Failed to copy application bundle: ditto returned %1").arg(exitCode));
			doRollback = true;
		}
	}
	
	// Step 4: close the dmg: hdiutil detach /Volumes/Hyves\ Desktop
	args.clear();
	args << "detach" << "/Volumes/Hyves Desktop";
	m_d->log->debug(QString("Unmounting dmg: hdiutil '%1'").arg(args.join("' '")));
	exitCode = QProcess::execute("hdiutil", args);
	if (exitCode) {
		m_d->log->warning(QString("Failed to unmount dmg file: hdiutil returned %1").arg(exitCode));
	}
	
	// if something went wrong in step 3 restore the old version from tmp dir
	if (doRollback) {
		m_d->log->notice("Update failed. Rolling back...");
		args.clear();
		args << tmpPath + "/orig/HyvesDesktop.app" << bundleDir.absolutePath();
		QProcess::execute("mv", args);
		QMessageBox::critical(0, HD_PRODUCT_NAME, tr("Oops, the updater encountered a problem and the application has not been updated.\n\n"
													 "Please try and download the latest version from http://www.hyves.nl/hyvesdesktop/download "
													 "or contact us at http://www.hyves.nl/help"));
		return;
	}
	
	// Step 5: remove the old stuff
	args.clear();
	args << "-rf" << tmpPath;
	m_d->log->debug(QString("Removing backup of previous installation: rm '%1'").arg(args.join("' '")));
	exitCode = QProcess::execute("rm", args);
	if (exitCode) {
		m_d->log->warning(QString("Failed to remove backup: rm returned %1").arg(exitCode));
	}
	
	// Help with renaming from HyvesDesktop to Hyves Desktop
	// Step 6: restart the kwekker: open oldBundleLocation/HyvesDesktop.app
	restartCommand = QString("open \""  + bundleDir.absolutePath() + "\"");
#endif
	
#ifdef Q_WS_X11
	QStringList args;
	args << "--install-type" << "update" << "--prefix" << tmpPath + "/new";
	if (m_d->useSilentUpdate) {
		args << "--mode" << "unattended";
	}
	
	m_d->log->debug(QString("Starting %1 %2").arg(m_d->log->anonymize(updateFile, Logger::File)).arg(args.join(" ")));
	
	m_d->updateProcessStarted = true;
	m_d->updateProcess->start(updateFile, args);
	m_d->updateProcess->waitForFinished(-1);
	if (m_d->updateProcessStarted && m_d->updateProcess->exitCode() == 0) {
		QDir appDir(qApp->applicationDirPath());
		appDir.cdUp(); // move up from bin/ subdir
		
		// first, move the original stuff to a temp place
		QDir().mkpath(tmpPath + "/orig/");
		system(QString("mv " + appDir.absolutePath().replace(' ', "\\ ") + "/* " + tmpPath + "/orig/").toLatin1());
		
		// move the new stuff into place
		system(QString("mv " + tmpPath + "/new/* "
		               + appDir.absolutePath().replace(' ', "\\ ")).toLatin1());
		
		m_d->log->notice("Update installed.");
	} else {
		m_d->log->error(QString("Update installer terminated with exit code %1 (exit status: %2).")
				.arg(m_d->updateProcess->exitCode()).arg(m_d->updateProcess->exitStatus()));
	}
	// Remove the temp stuff
	Filer::removeDirectory(tmpPath);
	
	// restart app
	restartCommand = "sh -c \"" + qApp->applicationDirPath().replace(' ', "\\ ") + "/HyvesDesktop.sh\"";
#endif
	
	if (!restartCommand.isEmpty()) {
		if (!restartProcess.startDetached(restartCommand)) {
			m_d->log->notice(QString("Failed to restart process after update: %1.").arg(restartCommand));
			QMessageBox::warning(0, HD_PRODUCT_NAME,
				tr("Failed to restart %1 after update. Please restart %1 manually.").arg(HD_PRODUCT_NAME)
			);
		}
	}
#endif
}

bool Updater::verifyReply(const QString &reply, Updater::Update *update) {
	
	if (reply.isEmpty()) {
		m_d->log->warning("Could not fetch update information.");
		return false;
	}
	
	QDomDocument document;
	document.setContent(reply);
	QDomElement element = document.documentElement();
	for (QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
		QDomElement element = node.toElement();
		if (element.isNull()) {
			continue;
		}
		
		bool isUpdateElement = false;
		if (m_d->downloadHqUpdate) {
			isUpdateElement = (element.tagName() == "hq-update");
		} else if (m_d->downloadPreUpdate) {
			isUpdateElement = (element.tagName() == "pre-update");
		} else {
			isUpdateElement = (element.tagName() == "update");
		}
		if (isUpdateElement) {
			for (QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
				QDomElement element = node.toElement();
				if (element.isNull()) {
					continue;
				}
				
				if (element.tagName() == "version") {
					update->version = Version(element.text());
				}
				if (element.tagName() == "file" &&
				    (element.attribute("platform", "all") == "all" ||
				     element.attribute("platform") == HD_PLATFORM)) {
					update->path = element.text();
					update->md5 = element.attribute("md5");
				}
			}
		}
	}
	
	return update->version != Version() && !update->path.isEmpty();
}

void Updater::fetchUpdate(const Updater::Update &update) {
	
	emit downloadingUpdate();
	
	m_d->update = update;
	
	QUrl url;
	url.setScheme("http");
	url.setHost(UPDATE_HOST);
	url.setPath("/" + m_d->updatePath + "/" + update.path);
	
	m_d->log->notice(QString("Now fetching update %1 from %2. Hash: %3")
			.arg(update.version.toString()).arg(url.toString())
			.arg(update.md5));
	
	delete m_d->updateFetcher;
	m_d->updateFetcher = new UrlFetcher(url, UrlFetcher::FetchTemporaryFile, this);
	
	connect(m_d->updateFetcher, SIGNAL(replyReady(QTemporaryFile *)),
				SLOT(updateDownloaded(QTemporaryFile *)));
}

void Updater::updateCheckResult(const QString &reply) {
	
	// if update succeeds, set timer to day, if fails, set timer to hour
	Update update;
	if (!verifyReply(reply, &update)) {
		m_d->timer.setInterval(HOUR);
	} else {
		m_d->timer.setInterval(DAY);
		if (update.version > HD_VERSION) {
			fetchUpdate(update);
		} else {
			m_d->log->notice("At latest version.");
			emit noUpdateAvailable();
		}
	}
}

void Updater::errorDownloading() {
	
	if (m_d->errorCount < 3) {
		m_d->errorCount++;
		m_d->timer.setInterval(HOUR);
	} else {
		m_d->errorCount = 0;
		m_d->timer.setInterval(DAY);
	}
	
	emit errorDownloadingUpdate();
}

void Updater::updateDownloaded(QTemporaryFile *temporaryFile) {
	
	if (temporaryFile == 0) {
		m_d->log->warning("Downloading update failed.");
		errorDownloading();
		return;
	}
	
	// check whether the hash is ok -- this detects failed or compromised downloads
	// in a local block, so we get rid of the bytearray asap
	if (!m_d->update.md5.isEmpty()) {
		if (!temporaryFile->isOpen()) {
			temporaryFile->open();
		} else {
			temporaryFile->reset();
		}
		// if our updates get really big, this'll suck down memory pretty hard
		QByteArray md5 = QCryptographicHash::hash(temporaryFile->readAll(), QCryptographicHash::Md5);
		
		if (QString(md5.toHex()) != m_d->update.md5) {
			m_d->log->warning(QString("Hash check on update failed. Was: %1, expected: %2")
			                  .arg(QString(md5.toHex()))
			                  .arg(m_d->update.md5));
			
			delete temporaryFile;
			errorDownloading();
			return;
		}
	}
	
	temporaryFile->reset();
	
	// we store the update file for later reference, in case the user doesn't want
	// to install it right away, we will install it automatically on next run
	QString dataLocation = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
	QString availableUpdateLocation = dataLocation;
	
	// make sure the data directory exists and clean up old updates
	QDir dataDir(availableUpdateLocation);
	dataDir.mkpath(".");
	dataDir.remove(m_d->updateFilename);
	
	QString updateFile = availableUpdateLocation + "/" + m_d->updateFilename;
	QFile file(updateFile);
	file.open(QIODevice::ReadWrite);
	int bytesWritten = file.write(temporaryFile->readAll());
	file.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
	file.close();
	
	delete temporaryFile;
	
	if (bytesWritten == -1) {
		m_d->log->error("Cannot copy update to data directory.");
		emit errorDownloadingUpdate();
		return;
	}
	
	QString installableFile = updateFile + ".installable";
	file.setFileName(installableFile);
	bool result = file.open(QIODevice::WriteOnly);
	if (!result) {
		m_d->log->error("Cannot create installable file.");
		emit errorDownloadingUpdate();
		return;
	}
	
	file.write(m_d->update.version.toString().toLatin1());
	file.close();
	
	m_d->errorCount = 0;
	
	m_d->log->notice(QString("Found available update: %1 -> %2").arg(HD_VERSION.toString()).arg(m_d->update.version.toString()));
	emit updateAvailable();
}

void Updater::processError(QProcess::ProcessError error) {
	
	m_d->log->warning(QString("Update installer failed to start. Error code: %1 (%2)")
	                  .arg(error).arg(QString(m_d->updateProcess->errorString())));
	
	m_d->updateProcessStarted = false;
}

Updater::Updater() :
	QObject(),
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("Updater");
	
#ifdef Q_WS_MAC
	m_d->updateFilename = "available-update.dmg";
#endif
#ifdef Q_WS_WIN
	m_d->updateFilename = "available-update.exe";
#endif
#ifdef Q_WS_X11
	m_d->updateFilename = "available-update.bin";
#endif
	
	m_d->timer.setInterval(DAY);
	connect(&m_d->timer, SIGNAL(timeout()), SLOT(checkForUpdates()));
	
	Extender::Extender::instance()->registerObject("updater", this);
}

Updater::~Updater() {
	
	Extender::Extender::instance()->unregisterObject("updater");
	
	delete m_d;
}

} // namespace Updater
