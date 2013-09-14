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

#ifndef HYVESAPI_H
#define HYVESAPI_H

#include <QDomDocument>
#include <QObject>
#include <QVariantMap>

#include "ResultDocument.h"
#include "OAuthUtil.h"

#include "hyveslib_export.h"

namespace HyvesAPI {

typedef GenusApis::ResultDocument ResultDocument;
typedef GenusApis::OAuthUtil::ParameterMap ParameterMap; 

class HYVESLIB_EXPORT HyvesAPI : public QObject {
	
	Q_OBJECT
	
	public:
		enum LoginStatus { NotLoggedIn, LoggingIn, LoggedIn};
		
		static HyvesAPI *instance();
		static void destroy();
		
	public slots:
		/**
		 * Sets the authorized request token that's given once logged in. After
		 * this call, normal methods can be called.
		 * 
		 * If there already is a valid access token, this call is ignored.
		 *
		 * @param token The token given by the website.
		 */
		void setRequestToken(const QString &token);
		
		/**
		 * Returns whether an access token is available. If so, there is no
		 * point in setting the request token.
		 * 
		 * @return @c true if there is an access token, @c false otherwise.
		 */
		bool hasAccessToken() const;
		
		/**
		 * Performs a method on the Hyves API. The result of the method is given
		 * back using the resultReady signal.
		 *
		 * @param method The method to call.
		 * @param parameters Parameters to pass to the method, using URL
		 *                   encoding.
		 * @return requestId The requestId for this call. You can use this to
		 *                   match a resultReady/error signal against this request.
		 */
		int doMethod(const QString &method, const QString &parameters);
		
		/**
		 * Performs a method on the Hyves API. The result of the method is given
		 * back using the resultReady signal.
		 *
		 * @param method The method to call.
		 * @param parameterMap A map containing all the parameters to pass.
		 * @return requestId The requestId for this call. You can use this to
		 *                   match a resultReady/error signal against this request.
		 */
		int doMethodWithMap(const QString &method, const ParameterMap &parameterMap);
		
		/**
		 * Performs a method on the Hyves API. The result of the method is given
		 * back using the resultReady signal.
		 *
		 * @param method The method to call.
		 * @param jsonParameters Parameters to pass to the method, encoded in JSON.
		 * @return requestId The requestId for this call. You can use this to
		 *                   match a resultReady/error signal against this request.
		 */
		int doMethodJson(const QString &method, const QString &jsonParameters);
		
		/**
		 * Returns the user ID of the currently authorized user.
		 */
		QString authorizedUserId() const;
		
		/**
		 * Returns the current user login status.
		 */
		LoginStatus loginStatus() const;
		
	signals:
		/**
		 * Emitted when the access token is requested.
		 */
		void accessTokenRequested();
		
		/**
		 * Emitted when the access token is retrieved, and we're ready for
		 * calling methods.
		 */
		void accessTokenReady();
		
		/**
		 * Emitted when a method resulted in an error
		 *
		 * @param requestId The request id that was earlier returned by the
		 *                      corresponding doMethod* call
		 * @param result DOM document containing the result
		 */
		void error(const QDomDocument &result);
		void error(int requestId, const QDomDocument &result);
		
		/**
		 * Emitted when a new result from a method is returned.
		 *
		 * @param requestId The request id that was earlier returned by the
		 *                      corresponding doMethod* call
		 * @param result DOM document containing the result
		 */
		void resultReady(const QDomDocument &result);
		void resultReady(int requestId, const QDomDocument &result);
		
	private:
		static HyvesAPI *s_instance;
		
		struct Private;
		Private *const m_d;
		
		HyvesAPI();
		virtual ~HyvesAPI();
		
	private slots:
		void handleResult(int requestId, ResultDocument result, bool errorOccurred);
		
		void resetToken();
		
		void onEventRaised(const QVariantMap &event);
};

} // namespace HyvesAPI

#endif // HYVESAPI_H
