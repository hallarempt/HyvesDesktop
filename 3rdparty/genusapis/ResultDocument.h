/*
 * Copyright (c) 2008 Arend van Beelen jr. <arendjr@gmail.com>
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
 * @author Arend van Beelen jr. (arendjr@gmail.com)
 * @url http://genusapis.marjew.nl/
 */

#ifndef __RESULTDOCUMENT_H
#define __RESULTDOCUMENT_H

#include <QDomDocument>

namespace GenusApis {

class OAuthRequestToken;
class OAuthAccessToken;

class ResultDocument : public QDomDocument {
	
	public:
		static const int RESULT_OK = 0;
		
		ResultDocument();
		virtual ~ResultDocument();
		
		/**
		 * Returns the value of a property in the document, identified by its
		 * path.
		 * 
		 * @param path Path of the property in XPath format.
		 * @return The value of the property, or an empty string if not found.
		 * 
		 * @note Please note that only a minimal subset of XPath is supported.
		 */
		QString property(QString path) const;
		
		/**
		 * Returns the error code of the result. If the result does not contain
		 * any errors, the constant RESULT_OK is returned.
		 * 
		 * @return Error code of the result.
		 */ 
		int errorCode() const;
	
		/**
		 * Returns the error message of the result. If the result does not contain
		 * any errors, an empty string is returned.
		 * 
		 * @return Error message of the result.
		 */ 
		QString errorMessage() const;
		
		/**
		 * Extracts the request token parameters from the result and creates a
		 * new OAuthRequestToken object.
		 * 
		 * @return A new OAuthRequestToken object. The object should be deleted
		 *         by the caller. Returns 0 if the result does not contain an
		 *         OAuth request token.
		 */
		OAuthRequestToken *toRequestToken() const;
		
		/**
		 * Extracts the access token parameters from the result and creates a
		 * new OAuthAccessToken object.
		 * 
		 * @return A new OAuthAccessToken object. The object should be deleted
		 *         by the caller. Returns 0 if the result does not contain an
		 *         OAuth access token.
		 */
		OAuthAccessToken *toAccessToken(const QString &rootElement = "auth_accesstoken_result") const;

		/**
		 * Sets the error specified by code and message.
		 */
		void setError(int code, const QString &message);
};

}

#endif //__RESULTDOCUMENT_H
