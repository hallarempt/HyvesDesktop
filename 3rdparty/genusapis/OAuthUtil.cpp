/*
 * Copyright (c) 2008 Kilian Marjew <kilian@marjew.nl>
 *               2008 Arend van Beelen jr. <arendjr@gmail.com>
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

#include <QUrl>

#include <QtCrypto>

#include "OAuthUtil.h"

namespace GenusApis {

bool parameterLessThan(const OAuthUtil::Parameter &p1, const OAuthUtil::Parameter &p2) {
	
	int result = QString::compare(OAuthUtil::urlencodeRFC3986(p1.key),
	                              OAuthUtil::urlencodeRFC3986(p2.key));
	if(result == 0) {
		result = QString::compare(OAuthUtil::urlencodeRFC3986(p1.value),
		                          OAuthUtil::urlencodeRFC3986(p2.value));
	}
	return result < 0;
}

QString OAuthUtil::normalizeParameters(OAuthUtil::ParameterList parameterList) {
	
	qSort(parameterList.begin(), parameterList.end(), parameterLessThan);
	
	QStringList normalizedParameters;
	foreach(Parameter parameter, parameterList) {
		normalizedParameters.append(urlencodeRFC3986(parameter.key) + "=" + urlencodeRFC3986(parameter.value));
	}
	return normalizedParameters.join("&");
}

QString OAuthUtil::urlencodeRFC3986(QString string) {
	
    return QUrl::toPercentEncoding(string, "~");
}

QString OAuthUtil::urldecodeRFC3986(QString string) {
	
    return QUrl::fromPercentEncoding(string.toUtf8()); // no exta stuff needed for ~, goes correctly automatically
}

QString OAuthUtil::generateBaseString(QString httpMethod, QString uri, OAuthUtil::ParameterMap parameterMap) {
	
	if(parameterMap.contains("oauth_signature")) {
		parameterMap.remove("oauth_signature"); // this key is explicitly ignored
	}
	
	QString parameterString = normalizeParameters(parameterMap);
	
	return urlencodeRFC3986(httpMethod) + "&" +
	       urlencodeRFC3986(uri) + "&" +
	       urlencodeRFC3986(parameterString);
}

QString OAuthUtil::calculateHMACSHA1Signature(QString baseString, QString consumerSecret, QString tokenSecret) {
	
	QCA::MessageAuthenticationCode hmac("hmac(sha1)", QCA::SecureArray());
	
	QCA::SymmetricKey key((urlencodeRFC3986(consumerSecret) + "&" + urlencodeRFC3986(tokenSecret)).toUtf8());
	hmac.setup(key);
	
	QCA::SecureArray data = baseString.toUtf8();
	hmac.update(data);
	
	QCA::SecureArray result = hmac.final();
	
	QCA::Base64 base64;
	return base64.encode(result).toByteArray();
}

QString OAuthUtil::calculateOAuthSignature(QString httpMethod, QString uri,
                                           OAuthUtil::ParameterMap parameterMap,
                                           QString consumerSecret, QString oauthTokenSecret) {
	
	QString baseString = generateBaseString(httpMethod, uri, parameterMap);
	return calculateHMACSHA1Signature(baseString, consumerSecret, oauthTokenSecret);
}

}
