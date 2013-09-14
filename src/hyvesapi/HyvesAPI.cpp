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

#include "GenusApis"

#include "extender/Extender.h"
#include "logger/Logger.h"
#include "settingsmanager/SettingsManager.h"
#include "windowmanager/WindowManager.h"
#include "HyvesAPI.h"
#include "NetworkAccessManager.h"
#include "Util.h"

namespace HyvesAPI {

HyvesAPI *HyvesAPI::s_instance = 0;

struct HyvesAPI::Private {
	Logger::Logger *log;
	GenusApis::GenusApis *genusApis;
	GenusApis::OAuthAccessToken *accessToken;
	HyvesAPI::LoginStatus loginStatus;

	Private() :
		log(0),
		genusApis(0),
		accessToken(0),
		loginStatus(HyvesAPI::NotLoggedIn) {
	}
	
	~Private() {
		
		delete genusApis;
		delete log;
		delete accessToken;
	}
};

HyvesAPI *HyvesAPI::instance() {
	
	if (s_instance == 0) {
		s_instance = new HyvesAPI();
	}
	
	return s_instance;
}

void HyvesAPI::destroy() {
	
	delete s_instance;
	s_instance = 0;
}

void HyvesAPI::setRequestToken(const QString &token) {
	
	if (!hasAccessToken()) {
		GenusApis::OAuthUtil::ParameterMap map;
		map["desktoptoken"] = token;
		int id = m_d->genusApis->doMethod("auth.accesstokenByDesktoptoken", map);
		m_d->log->debug(QString("Called method 'auth.accesstokenByDesktoptoken' (request #%1)").arg(id));
		m_d->loginStatus = HyvesAPI::LoggingIn;
		m_d->log->notice("Requested a new access token");
		emit accessTokenRequested();
	}
}

bool HyvesAPI::hasAccessToken() const {
	
	return (m_d->accessToken != 0);
}

int HyvesAPI::doMethod(const QString &method, const QString &parameters) {
	
	int id = m_d->genusApis->doMethod(method, parameters, m_d->accessToken);
	m_d->log->debug(QString("Called method '%1' (request #%2)").arg(method).arg(id));
	return id;
}

int HyvesAPI::doMethodWithMap(const QString &method, const ParameterMap &parameterMap) {
	
	int id = m_d->genusApis->doMethod(method, parameterMap, m_d->accessToken);
	m_d->log->debug(QString("Called method '%1' (request #%2)").arg(method).arg(id));
	return id;
}

int HyvesAPI::doMethodJson(const QString &method, const QString &jsonParameters) {
	
	QVariantMap variantMap = Util::jsonToVariantMap(jsonParameters);
	GenusApis::OAuthUtil::ParameterMap parameterMap;
	for (QVariantMap::ConstIterator it = variantMap.begin();
	     it != variantMap.end(); ++it) {
		parameterMap[it.key()] = it.value().toString();
	}
	
	int id = m_d->genusApis->doMethod(method, parameterMap, m_d->accessToken);
	m_d->log->debug(QString("Calling method '%1' (request #%2)").arg(method).arg(id));
	return id;
}

QString HyvesAPI::authorizedUserId() const {
	
	return m_d->accessToken ? m_d->accessToken->userId() : QString();
}

HyvesAPI::LoginStatus HyvesAPI::loginStatus() const {
	
	return m_d->loginStatus;
}

HyvesAPI::HyvesAPI() :
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("HyvesAPI");
	
	Extender::Extender::instance()->registerObject("hyvesApi", this);
	
	QString baseServer = SettingsManager::SettingsManager::instance()->baseServer();
	
	QString consumerKey;
	QString consumerSecret;
	if (baseServer.endsWith(".vmware")) {
		// development keys
		consumerKey = "N1_oC7UKCi9Kx1dWMiHRqIr9";
		consumerSecret = "N19ZLYKDpyooMnRkXBYJoJgI";
	} else if (baseServer == "hyves.nl") {
		// live keys
		consumerKey = "N19fDUdURoNxyRwGgUEr1QZo";
		consumerSecret = "N18N6EFtM5tktXNAW11z4iuf";
	} else if (baseServer.contains("staging")) {
		// staging keys
		consumerKey = "N1-R29hjojOpgts7Iopp3lKB";
		consumerSecret = "N18UTG3nrp8ifKD5hvDKkXdF";
	}
	
	m_d->genusApis = new GenusApis::GenusApis(consumerKey, consumerSecret);
	m_d->genusApis->setNetworkAccessManager(NetworkAccessManager::instance());
	
	if (baseServer.startsWith("hyves.")) {
		m_d->genusApis->setApiHost("data.hyves-api." + baseServer.mid(6));
	}
	
	connect(m_d->genusApis, SIGNAL(resultReady(int, ResultDocument, bool)),
	                        SLOT(handleResult(int, ResultDocument, bool)));
	
	connect(WindowManager::WindowManager::instance(), SIGNAL(eventRaised(QVariantMap)),
	                                                  SLOT(onEventRaised(QVariantMap)));
}

HyvesAPI::~HyvesAPI() {
	
	Extender::Extender::instance()->unregisterObject("hyvesApi");
	
	delete m_d;
}

void HyvesAPI::handleResult(int requestId, ResultDocument result, bool errorOccurred) {
	
	m_d->log->debug(QString("Received response for request #%1").arg(requestId));
	
	if (errorOccurred || result.errorCode() != ResultDocument::RESULT_OK) {
		m_d->log->error(QString("An error occurred during API call (request #%3): %1 %2").arg(result.errorCode()).arg(result.errorMessage()).arg(requestId));
		emit error(result);
		emit error(requestId, result);
		return;
	}
	
	if (m_d->accessToken == 0) {
		GenusApis::OAuthAccessToken *accessToken = result.toAccessToken("auth_accesstokenByDesktoptoken_result");
		
		if (accessToken != 0) {
			m_d->log->notice("Got access token from API");
			
			m_d->accessToken = accessToken;
			
			m_d->loginStatus = HyvesAPI::LoggedIn;
			emit accessTokenReady();
			return;
		}

		m_d->log->debug("Response received but no access token yet!");
	}
	
	emit resultReady(result);
	emit resultReady(requestId, result);
}

void HyvesAPI::resetToken() {
	
	delete m_d->accessToken;
	m_d->accessToken = 0;
	
	m_d->log->debug("API token reset.");
}

void HyvesAPI::onEventRaised(const QVariantMap &event) {
	
	if (event["name"] == "userLoggedOut") {
		m_d->loginStatus = HyvesAPI::NotLoggedIn;
		resetToken();
	}
}

} // namespace HyvesAPI
