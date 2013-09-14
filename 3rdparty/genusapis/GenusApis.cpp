/*
 * Copyright (c) 2008 Kilian Marjew <kilian@marjew.nl>
 *               2008 Arend van Beelen jr. <arendjr@gmail.com>
 *               2008 Girish Ramakrishnan <girish@forwardbias.in>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * @author Kilian Marjew (kilian@marjew.nl)
 * @url http://genusapis.marjew.nl/
 */

#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <QtCrypto>

#include <cstdlib>

#include "OAuthBase.h"
#include "OAuthConsumer.h"
#include "OAuthRequestToken.h"
#include "GenusApis.h"

namespace GenusApis {

const QString GenusApis::DEFAULT_HA_VERSION("1.0");
const QString GenusApis::DEFAULT_HA_FORMAT("xml");
const QString GenusApis::DEFAULT_HA_FANCYLAYOUT("false");
const QString GenusApis::DEFAULT_OAUTH_SIGNATURE_METHOD("HMAC-SHA1");
const QString GenusApis::HTTP_METHOD_GET("GET");
const QString GenusApis::HTTP_METHOD_POST("POST");	
const QString GenusApis::API_HOST("data.hyves-api.nl");
const QString GenusApis::AUTHORIZE_URL("http://www.hyves.nl/api/authorize/");

GenusApis::GenusApis(OAuthConsumer *consumer) :
	QObject(),
	m_consumer(consumer),
	m_networkAccessManager(0),
	m_requestId(0),
	m_timestampLastMethod(0),
	m_nonce(0),
	m_timestampOffset(0) {
	
	init();
}

GenusApis::GenusApis(QString consumerKey, QString consumerSecret) :
	QObject(),
	m_consumer(new OAuthConsumer(consumerKey, consumerSecret)),
	m_networkAccessManager(0),
	m_requestId(0),
	m_timestampLastMethod(0),
	m_nonce(0),
	m_timestampOffset(0) {
	
	init();
}

GenusApis::~GenusApis() {
#ifndef Q_WS_MAC
	delete m_qcaInitializer;
#endif
	delete m_consumer;
}

void GenusApis::setApiHost(const QString &host) {
	
	m_apiHost = host;
	m_apiUrl = "http://" + host + "/";
}

void GenusApis::setAuthorizeUrl(const QString &authorizeUrl) {
	
	m_authorizeUrl = authorizeUrl;
}

void GenusApis::setNetworkAccessManager(QNetworkAccessManager *manager) {
	
	if (manager != m_networkAccessManager) {
		if (m_networkAccessManager && m_networkAccessManager->parent() == this) {
			delete m_networkAccessManager;
		}
		
		m_networkAccessManager = manager;
	}
}

int GenusApis::doMethod(QString method, OAuthUtil::ParameterMap parameterMap,
                        OAuthBase *oAuthToken, QString httpMethod) {
	
	OAuthUtil::ParameterMap defaultParameters;
	defaultParameters["oauth_consumer_key"] = m_consumer->key();
	defaultParameters["oauth_timestamp"] = QString::number(oAuthTimestamp());
	defaultParameters["oauth_nonce"] = oAuthNonce();
	defaultParameters["oauth_signature_method"] = DEFAULT_OAUTH_SIGNATURE_METHOD;
	defaultParameters["ha_method"] = method;
	defaultParameters["ha_version"] = DEFAULT_HA_VERSION;
	defaultParameters["ha_format"] = DEFAULT_HA_FORMAT;
	defaultParameters["ha_fancylayout"] = DEFAULT_HA_FANCYLAYOUT;
	defaultParameters["ha_responsecode_always_200"] = "true";
	
	QString oAuthTokenSecret = "";
	
	if(oAuthToken != 0) {
		defaultParameters["oauth_token"] = oAuthToken->key();
		oAuthTokenSecret = oAuthToken->secret();
	}
	
	for(OAuthUtil::ParameterMap::ConstIterator it = defaultParameters.constBegin();
	    it != defaultParameters.constEnd(); ++it) {
		if(!parameterMap.contains(it.key())) {
			parameterMap[it.key()] = *it;
		}
	}
	
	sendRequest(method, parameterMap, oAuthTokenSecret, httpMethod, ++m_requestId);
	return m_requestId;
}

void GenusApis::sendRequest(QString method, OAuthUtil::ParameterMap parameterMap,
                        	QString oAuthTokenSecret, QString httpMethod, int requestId) {
	
	QString oAuthConsumerSecret = m_consumer->secret();
	
	RequestInfo requestInfo;
	requestInfo.oAuthTokenSecret = oAuthTokenSecret;
	requestInfo.parameters = parameterMap;
	requestInfo.httpMethod = httpMethod;
	requestInfo.timestampOffset = m_timestampOffset;
	m_sentRequests[requestId] = requestInfo;
	
	parameterMap["oauth_signature"] = OAuthUtil::calculateOAuthSignature(httpMethod, m_apiUrl, parameterMap,
	                                                                     oAuthConsumerSecret, oAuthTokenSecret);
	
	QString parameterString = OAuthUtil::normalizeParameters(parameterMap);
	
	QNetworkReply *reply;
	
	if(httpMethod == HTTP_METHOD_POST) {
		QNetworkRequest request(m_apiUrl);
		request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
		reply = m_networkAccessManager->post(request, parameterString.toUtf8());
		Q_ASSERT(reply);
	} else {
		QNetworkRequest request(m_apiUrl + "?" + parameterString.toUtf8());
		reply = m_networkAccessManager->get(request);
		Q_ASSERT(reply);
	}
	reply->setProperty("genusapi_request_id", requestId);
	connect(reply, SIGNAL(finished()), SLOT(processResult()));
}

int GenusApis::retrieveRequestToken(QStringList requestedMethods, QString expirationType) {
	
	OAuthUtil::ParameterMap parameterMap;
	parameterMap["methods"] = requestedMethods.join(",");
	parameterMap["expirationtype"] = expirationType;
	
	return doMethod("auth.requesttoken", parameterMap);
}

int GenusApis::retrieveAccessToken(OAuthRequestToken *requestToken) {
	
	return doMethod("auth.accesstoken", OAuthUtil::ParameterMap(), requestToken);
}

QUrl GenusApis::getAuthorizeUrl(OAuthRequestToken *requestToken) const {
	
	QString url = m_authorizeUrl + "?oauth_token=" + requestToken->key();
	return QUrl(url);
}

time_t GenusApis::oAuthTimestamp() {
	
	time_t timestamp = time(0) + m_timestampOffset;
	if(m_timestampLastMethod == timestamp) {
		m_nonce++;		
	} else {
		m_timestampLastMethod = timestamp;
		m_nonce = 0;
	}
	
	return m_timestampLastMethod;
}

void GenusApis::init() {
	
	m_qcaInitializer = new QCA::Initializer();
	Q_ASSERT(QCA::isSupported("hmac(sha1)"));
	
	setApiHost(API_HOST);
	setAuthorizeUrl(AUTHORIZE_URL);
	setNetworkAccessManager(new QNetworkAccessManager(this));
}

QString GenusApis::oAuthNonce() const {
	
	QString ipAddress = "0.0.0.0";
	int randomNumber = rand();
	return QString::number(m_nonce) + "_" + ipAddress + "_" + QString::number(randomNumber);
}

void GenusApis::retry(int requestId) {
	
	Q_ASSERT(m_sentRequests.contains(requestId));
	const RequestInfo &requestInfo = m_sentRequests[requestId];
	OAuthUtil::ParameterMap parameters = requestInfo.parameters;
	parameters["oauth_timestamp"] = QString::number(oAuthTimestamp());
	sendRequest(parameters.value("ha_method"), parameters, requestInfo.oAuthTokenSecret, requestInfo.httpMethod, requestId);
}

void GenusApis::processResult() {
	
	QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
	reply->deleteLater();
	
	ResultDocument result;
	const int requestId = reply->property("genusapi_request_id").toInt();
	
	bool parseSuccessful = false;
	if (reply->error() == QNetworkReply::NoError) {
		parseSuccessful = result.setContent(reply->readAll());
	}
	
	if (reply->error() == QNetworkReply::NoError && parseSuccessful) {
		const long timestampOffset = m_sentRequests[requestId].timestampOffset;
		if (result.errorCode() == 16) {
			long diff = result.property("/error_result/info/timestamp_difference").toLong();
			m_timestampOffset = timestampOffset + diff;
			retry(requestId);
			return;
		}
		
		QDomElement element = result.elementsByTagName("info").item(0).firstChildElement("timestamp_difference");
		if (!element.isNull()) {
			long diff = element.text().toLong();
			m_timestampOffset = timestampOffset + diff;
		}
	} else if (reply->error()) {
		result.setError(-reply->error(), reply->errorString());
	} else {
		result.setError(-800, QString("Could not parse result"));
	}
	
	emit resultReady(result, !parseSuccessful);
	emit resultReady(requestId, result, !parseSuccessful);
	m_sentRequests.remove(requestId);
}

} // namespace GenusApis
