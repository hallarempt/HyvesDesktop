/*
 * Copyright (c) 2008 Kilian Marjew <kilian@marjew.nl>
 *               2008 Arend van Beelen jr. <arendjr@gmail.com>
 *               2009 Girish Ramakrishnan <girish@forwardbias.in>
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

#ifndef __GENUSAPIS_H
#define __GENUSAPIS_H

#include <ctime>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QHash>

#include "OAuthUtil.h"
#include "ResultDocument.h"

class QNetworkAccessManager;

namespace QCA {

class Initializer;

}

namespace GenusApis {

class OAuthBase;
class OAuthConsumer;
class OAuthRequestToken;

/**
 * Main class for interacting with the Hyves API.
 * 
 * You should create one instance of this class. Before you can do anything,
 * you should retrieve an access token which will allow you to perform method
 * calls using doMethod().
 * 
 * The steps for retrieving an access token are as follows:
 * <ol>
 *   <li>Retrieve a request token using retrieveRequestToken().</li>
 *   <li>Use the result from retrieveRequestToken() and convert it to an
 *      OAuthRequestToken using ResultDocument::toRequestToken().</li>
 *   <li>Generate the authorize URL using getAuthorizeUrl(), and show it
 *      to the user (using QDesktopServices::openUrl(), for example).</li>
 *   <li>When the user has authorized your token, you can retrieve the
 *      actual access token using retrieveAccessToken().</li>
 *   <li>Use the final access token (which can be generated from the result
 *      using ResultDocument::toAccessToken() for further calls to
 *      doMethod().</li>
 * </ol> 
 */
class GenusApis : public QObject {
	
	Q_OBJECT
	
	public:
		static const QString DEFAULT_HA_VERSION;
		static const QString DEFAULT_HA_FORMAT;
		static const QString DEFAULT_HA_FANCYLAYOUT;
		static const QString DEFAULT_OAUTH_SIGNATURE_METHOD;
		static const QString HTTP_METHOD_GET;
		static const QString HTTP_METHOD_POST;
		static const QString API_HOST;
		static const QString AUTHORIZE_URL;
		
		/**
		 * Constructor.
		 * 
		 * @param consumerKey Your Hyves API consumer key.
		 * @param consumerKey Your Hyves API consumer secret.
		 */
		GenusApis(QString consumerKey, QString consumerSecret);
		
		/**
		 * Constructor.
		 * 
		 * @param consumer Your OAuthConsumer instance. The instance will be
		 *                 owned by the GenusApis instance, and deleted upon
		 *                 destruction.
		 */
		GenusApis(OAuthConsumer *consumer);
		
		/**
		 * Destructor.
		 */
		virtual ~GenusApis();
		
		/**
		 * Sets the host to use to connect to the API. The default is
		 * data.hyves-api.nl.
		 * 
		 * @param host The new host for connecting to the API.
		 */
		void setApiHost(const QString &host);
		
		/**
		 * Sets the URL that should be redirected to for authorization.
		 * The default is http://www.hyves.nl/api/authorize/.
		 * 
		 * @param authorizeUrl The new authorize URL.
		 */
		void setAuthorizeUrl(const QString &authorizeUrl);
		
		/**
		 * Sets the network access manager to use for connecting to the API.
		 * 
		 * @param manager The network access manager instance.
		 */ 
		void setNetworkAccessManager(QNetworkAccessManager *manager);
		
		/**
		 * Performs a method using the Hyves API.
		 * 
		 * @param method The method to call. You should be authorized to perform
		 *               this method.
		 * @param parameterMap Parameters you wish to pass to the method. Can
		 *                     also be given as a string.
		 * @param oAuthToken Your access token.
		 * @param httpMethod The HTTP method (GET or POST) to use.
		 * @result requestId The requestId for this call. You can use this to
		 *                   match a resultReady signal against this request.
		 *
		 * This method will return immediately, and the actual method call will
		 * be performed asynchronously. When the call has completed, you will
		 * receive a resultReady() signal.
		 */
		int doMethod(QString method, OAuthUtil::ParameterMap parameterMap,
		             OAuthBase *oAuthToken = 0,
		             QString httpMethod = HTTP_METHOD_POST);
		
		/**
		 * Retrieves a request token for authorization.
		 * 
		 * @param requestedMethods The methods you wish to receive authorization
		 *                         for.
		 * @param expirationType Defines when the token should expire.
		 * 
		 * The result of this call will be available from resultReady() signal.
		 */
		int retrieveRequestToken(QStringList requestedMethods,
		                         QString expirationType = "default");
		
		/**
		 * Retrieves the access token.
		 * 
		 * @param requestToken The authorized request token.
		 * 
		 * The result of this call will be available from resultReady() signal. 
		 * See doMethod() for details.
		 * 
		 * You should only call this method after the user has authorized the
		 * request token.
		 */
		int retrieveAccessToken(OAuthRequestToken *requestToken);
		
		/**
		 * Generates the authorization URL from a request token.
		 * 
		 * @param requestToken The request token.
		 * @return The authorization URL to show to the user.
		 */
		QUrl getAuthorizeUrl(OAuthRequestToken *requestToken) const;
		
	signals:
		/**
		 * This signal is emitted every time a method performed through
		 * doMethod() has completed.
		 * 
		 * @param requestId The request id that was earlier returned by the
		 *                      corresponding doMethod() call
		 * @param result DOM document containing the result. The document is
		 *               empty if @p errorOccurred is true.
		 * @param errorOccurred Indicates whether an error occurred during the
		 *                      method call.
		 */
		void resultReady(ResultDocument result, bool errorOccurred);
		void resultReady(int requestId, ResultDocument result, bool errorOccurred);
		
	private:
		OAuthConsumer *m_consumer;
		
		QCA::Initializer *m_qcaInitializer;
		
		QNetworkAccessManager *m_networkAccessManager;
		
		QString m_apiHost;
		QString m_authorizeUrl;
		QString m_apiUrl;

		int m_requestId;
		
		time_t m_timestampLastMethod;
		int m_nonce;
		time_t m_timestampOffset;
		
		struct RequestInfo {
			QString oAuthTokenSecret;
			OAuthUtil::ParameterMap parameters;
			QString httpMethod;
			time_t timestampOffset;
		};
		QHash<int, RequestInfo> m_sentRequests;

		void init();
		
		time_t oAuthTimestamp();
		QString oAuthNonce() const;

		void sendRequest(QString method, OAuthUtil::ParameterMap parameterMap,
						 QString oAuthTokenSecret,
						 QString httpMethod,
						 int requestId);

		void retry(int requestId);
		
	private slots:
		void processResult();
};

}

#endif //__GENUSAPIS_H
