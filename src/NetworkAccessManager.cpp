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
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QNetworkReply>
#include <QRegExp>
#include <QSslConfiguration>

#include "logger/Logger.h"
#include "mediacache/NetworkReply.h"
#include "settingsmanager/SettingsManager.h"
#include "tester/Tester.h"
#include "translator/Translator.h"
#include "zipper/Zipper.h"
#include "CookieJar.h"
#include "NetworkAccessManager.h"
#include "NetworkReply.h"

NetworkAccessManager *NetworkAccessManager::s_instance = 0;

struct NetworkAccessManager::Private {
	Logger::Logger *log;
	
	struct Handler {
		Handler(const QString &prefix, QObject *receiver, const QByteArray &name) :
			prefix(prefix),
			receiver(receiver),
			name(name) {
		}
		
		QString prefix;
		QObject *receiver;
		QByteArray name;
	};
	
	typedef QList<Handler> HandlerList;
	HandlerList handlers;
	QString bundlePath;
	
	Private() :
		log(0) {

	}
	
	~Private() {
		
		delete log;
	}
	
	static QString normalizePrecompiledFileName(const QString &fileName) {
		
		static QRegExp precompiledPattern("^(\\w+_\\w{2}_\\w{2})\\.\\w{16}\\.js$");
		
		if (precompiledPattern.exactMatch(fileName)) {
			return precompiledPattern.capturedTexts()[1] + ".js";
		} else if (fileName.contains("_locale.")) {
			QString localizedFileName = fileName;
			return localizedFileName.replace("locale", Translator::Translator::instance()->locale());
		} else {
			return QString::null;
		}
	}
	
	static QString normalizeResourcePath(const QString &path) {
		
		static QRegExp resourcePattern("^(.*)\\.\\w{32}\\.(css|js)$");
		
		if (resourcePattern.exactMatch(path)) {
			return resourcePattern.capturedTexts()[1] + "." + resourcePattern.capturedTexts()[2];
		} else {
			return path;
		}
	}
};

NetworkAccessManager *NetworkAccessManager::instance() {
	
	if (s_instance == 0) {
		s_instance = new NetworkAccessManager();
	}
	
	return s_instance;
}

void NetworkAccessManager::destroy() {
	
	delete s_instance;
	s_instance = 0;
}

void NetworkAccessManager::loadSettings() {
	
	QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
	QList<QSslCertificate> caList = sslConfig.caCertificates();
	QVariant certificates = SettingsManager::SettingsManager::instance()->variantValue("SSLCertificates/CaCertificates");
	QList<QSslCertificate> savedCaList = QSslCertificate::fromData(certificates.toByteArray());
	caList += savedCaList;
	
	sslConfig.setCaCertificates(caList);
	QSslConfiguration::setDefaultConfiguration(sslConfig);
}

void NetworkAccessManager::setBundlePath(const QString &path) {

	m_d->bundlePath = path;
}

QString NetworkAccessManager::bundlePath() const {

	if (m_d->bundlePath.isEmpty()) {
		QDir dir(QApplication::applicationDirPath());
		m_d->bundlePath = dir.path() + "/bundle";
	}
	return m_d->bundlePath;
}

bool NetworkAccessManager::existsInBundle(const QString &path) {
	
	if (m_d->bundlePath == "bundle://") {
		return Zipper::ZippedFile::exists("bundle.zip", path);
	} else {
		return QFile::exists(m_d->bundlePath + path);
	}
}

QNetworkReply *NetworkAccessManager::createRequest(QNetworkAccessManager::Operation operation,
                                                   const QNetworkRequest &_request, QIODevice *outgoingData) {
	
	QString localPath = bundlePath();
	
	QNetworkRequest request(_request);
	QString url(request.url().toString());
	QFileInfo pathInfo(request.url().path());
	
	foreach(const NetworkAccessManager::Private::Handler &handler, m_d->handlers) {
		if (url.startsWith(handler.prefix)) {
			NetworkReply *reply = 0;
			bool result = QMetaObject::invokeMethod(handler.receiver, handler.name, Qt::DirectConnection, 
					  						Q_RETURN_ARG(NetworkReply *, reply),
					  						Q_ARG(QNetworkRequest, request));
			if (!result) {
				m_d->log->debug("Failed to invoke custom URL handler");
			} else {
				return reply;
			}
		}
	}
	
	// this is a hack to get authentication right from localhost pages
	if (request.url().scheme() == "https") {
		request.setRawHeader("Referer", ("http://www." + SettingsManager::SettingsManager::instance()->baseServer() + "/").toLatin1());
	}
	
	// match precompiled client-side templates to the locally installed version
	if (pathInfo.path() == "/precompiled") {
		QString normalizedFileName = Private::normalizePrecompiledFileName(pathInfo.fileName());
		if (normalizedFileName != QString::null) {
			pathInfo = "/precompiled/" + normalizedFileName;
		}
	}
	
	// match versioned resource files
	QString normalizedPath = Private::normalizeResourcePath(pathInfo.filePath());
	if (existsInBundle(normalizedPath)) {
		pathInfo = normalizedPath;
	}
	
	// cached media objects get some special treatment
	if (url.startsWith("http://localhost/media")) {
		return new MediaCache::NetworkReply(request, url, this);
	}
	
	// AJAX calls to localhost should be redirected to the website
	if (url.startsWith("http://localhost/") &&
	    (pathInfo.path() == "/" || pathInfo.filePath().endsWith(".php"))) {
		QString pathPlusQuery = pathInfo.filePath();
		if (request.url().hasQuery()) {
			pathPlusQuery += "?" + request.url().encodedQuery();
		}
		m_d->log->debug("AJAX call to: " + pathPlusQuery);
		return createRemoteRequest(operation, request, pathPlusQuery, outgoingData);
	}
	
	if (url.startsWith("http://localhost/") || url.contains("/images/") ||
	    url.contains("/precompiled/") || url.contains("/statics/")) {
		
		if (existsInBundle(pathInfo.filePath())) {
			// use the content bundle if the file exists there
			return new NetworkReply(request, localPath + pathInfo.filePath(), this);
		} else {
			m_d->log->debug(QString("Referring to file %1 that's not inside content bundle (%2).")
							.arg(url).arg(localPath + pathInfo.filePath()));
			
			if (url.startsWith("http://localhost/")) {
				return createRemoteRequest(operation, request, pathInfo.filePath());
			}
		}
	}
	
	return QNetworkAccessManager::createRequest(operation, request, outgoingData);
}

void NetworkAccessManager::addRequestHandler(const QString &prefix, QObject *receiver, const char *method) {

	m_d->handlers.append(NetworkAccessManager::Private::Handler(prefix, receiver, method));
}

void NetworkAccessManager::removeRequestHandler(const QString &prefix) {
	
	for(NetworkAccessManager::Private::HandlerList::iterator it = m_d->handlers.begin();
	    it != m_d->handlers.end(); ++it) {
		if ((*it).prefix == prefix) {
			m_d->handlers.erase(it);
			break;
		}
	}
}

NetworkAccessManager::NetworkAccessManager() :
	QNetworkAccessManager(),
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("NetworkAccessManager");
	
	connect(this, SIGNAL(sslErrors(QNetworkReply *, const QList<QSslError> &)),
	        SLOT(sslErrors(QNetworkReply *, const QList<QSslError> &)));
	
	loadSettings();
	
	setCookieJar(new CookieJar(this));
	
	if (QFile::exists("bundle.zip")) {
		m_d->bundlePath = "bundle://";
	}
}

NetworkAccessManager::~NetworkAccessManager() {
	
	delete m_d;
}

void NetworkAccessManager::sslErrors(QNetworkReply *reply, const QList<QSslError> &error) {
	
	m_d->log->debug("sslErrors()");
	
	if (Tester::Tester::isTesting()) {
		reply->ignoreSslErrors();
		return;
	}
	
	
	QVariant certificates = SettingsManager::SettingsManager::instance()->variantValue("SSLCertificates/CaCertificates");
	QList<QSslCertificate> savedCaList = QSslCertificate::fromData(certificates.toByteArray());
	
	QList<QSslCertificate> ca_new;
	QStringList errorStrings;
	for (int i = 0; i < error.count(); ++i) {
		if (savedCaList.contains(error.at(i).certificate()))
			continue;
		errorStrings += error.at(i).errorString();
		if (!error.at(i).certificate().isNull()) {
			ca_new.append(error.at(i).certificate());
		}
	}
	if (errorStrings.isEmpty()) {
		reply->ignoreSslErrors();
		return;
	}
	
	QString errors = errorStrings.join(QLatin1String("\n"));
	int ret = QMessageBox::warning(QApplication::activeWindow(), QCoreApplication::applicationName(),
	                               tr("SSL Errors:\n\n%1\n\n%2\n\n"
	                                  "Do you want to ignore these errors?").arg(reply->url().toString()).arg(errors),
	                               QMessageBox::Yes | QMessageBox::No,
	                               QMessageBox::No);
	
	if (ret == QMessageBox::Yes) {
		if (ca_new.count() > 0) {
			ret = QMessageBox::question(QApplication::activeWindow(), QCoreApplication::applicationName(),
			                            tr("Do you want to accept all these certificates?"),
			                            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
			if (ret == QMessageBox::Yes) {
				savedCaList += ca_new;
				
				QSslConfiguration sslCfg = QSslConfiguration::defaultConfiguration();
				QList<QSslCertificate> ca_list = sslCfg.caCertificates();
				ca_list += ca_new;
				sslCfg.setCaCertificates(ca_list);
				QSslConfiguration::setDefaultConfiguration(sslCfg);
				reply->setSslConfiguration(sslCfg);
				
				QByteArray pems;
				for (int i = 0; i < savedCaList.count(); ++i)
					pems += savedCaList.at(i).toPem() + '\n';
				SettingsManager::SettingsManager::instance()->setValue("SSLCertificates/CaCertificates", pems);
			}
		}
		reply->ignoreSslErrors();
	}
	
}

QNetworkReply *NetworkAccessManager::createRemoteRequest(QNetworkAccessManager::Operation operation,
                                                         const QNetworkRequest &originalRequest,
                                                         const QString &path, QIODevice *outgoingData) {
	
	SettingsManager::SettingsManager *settingsManager = SettingsManager::SettingsManager::instance();
	
	QNetworkRequest remoteRequest(originalRequest);
	remoteRequest.setUrl("http://" + settingsManager->baseServer() + path);
	return QNetworkAccessManager::createRequest(operation, remoteRequest, outgoingData);
}
