#ifndef __OAUTHUTIL_H
#define __OAUTHUTIL_H

#include <QStringList>
#include <QList>
#include <QMap>

namespace GenusApis {

class OAuthUtil {
	
	public:
		struct Parameter {
			
			QString key;
			QString value;
			
			Parameter(QString key, QString value) : key(key), value(value) { };
		};
		
		class ParameterMap : public QMap<QString, QString> {
			
			public:
				ParameterMap() :
					QMap<QString, QString>() {	
				}
				
				ParameterMap(const char *parameterString) :
					QMap<QString, QString>() {
					
					QStringList parameters = QString(parameterString).split('&');
					for(QStringList::ConstIterator it = parameters.constBegin();
						it != parameters.constEnd(); ++it) {
						QStringList parts = (*it).split('=');
						QString key = parts.size() > 0 ? parts[0] : "";
						QString value = parts.size() > 1 ? parts[1] : "";
						insert(key, value);
					}
				}
				
				ParameterMap(QString parameterString) :
					QMap<QString, QString>() {
					
					QStringList parameters = parameterString.split('&');
					for(QStringList::ConstIterator it = parameters.constBegin();
						it != parameters.constEnd(); ++it) {
						QStringList parts = (*it).split('=');
						QString key = parts.size() > 0 ? parts[0] : "";
						QString value = parts.size() > 1 ? parts[1] : "";
						insert(key, value);
					}
				}
		};
		
		class ParameterList : public QList<Parameter> {
			
			public:
				ParameterList() :
					QList<Parameter>() {	
				}
				
				ParameterList(ParameterMap parameterMap) :
					QList<Parameter>() {
					
					for(ParameterMap::ConstIterator it = parameterMap.constBegin();
						it != parameterMap.constEnd(); ++it) {
						append(Parameter(it.key(), *it));
					}
				}
				
				ParameterList(const char *parameterString) :
					QList<Parameter>() {
					
					QStringList parameters = QString(parameterString).split('&');
					for(QStringList::ConstIterator it = parameters.constBegin();
						it != parameters.constEnd(); ++it) {
						QStringList parts = (*it).split('=');
						QString key = parts.size() > 0 ? parts[0] : "";
						QString value = parts.size() > 1 ? parts[1] : "";
						append(Parameter(key, value));
					}
				}
				
				ParameterList(QString parameterString) :
					QList<Parameter>() {
					
					QStringList parameters = parameterString.split('&');
					for(QStringList::ConstIterator it = parameters.constBegin();
						it != parameters.constEnd(); ++it) {
						QStringList parts = (*it).split('=');
						QString key = parts.size() > 0 ? parts[0] : "";
						QString value = parts.size() > 1 ? parts[1] : "";
						append(Parameter(key, value));
					}
				}
		};
		
		/**
		 * Normalizes a ParameterList into an encoded string.
		 */ 
		static QString normalizeParameters(ParameterList parameterList);
		
		/**
		 * Encodes strings in an RFC3986 compatible encoding.
		 *
		 * @param string String to encode.
		 * @return The encoded string. 
		 */
		static QString urlencodeRFC3986(QString string);
		
		/**
		 * Decodes an RFC3986 encoded string.
		 * 
		 * @param string String to decode.
		 * @return The decoded string.
		 */
		static QString urldecodeRFC3986(QString string);

		/**
		 * Creates the base string needed for signing per oAuth Section 9.1.2.
		 * All strings are latin1.
		 *
		 * @param httpMethod One of the HTTP methods (GET, POST, etc.).
		 * @param uri The URI; the URL without query string.
		 * @param paramaterMap Parameters.
		 */
		static QString generateBaseString(QString httpMethod, QString uri,
		                                  ParameterMap parameterMap);

		/**
		 * Calculates the HMAC-SHA1 secret.
		 *
		 * @param baseString Returned by generateBaseString().
		 * @param consumerSecret
		 * @param tokenSecret Leave empty if no token present.
		 */
		static QString calculateHMACSHA1Signature(QString baseString,
		                                          QString consumerSecret,
		                                          QString tokenSecret);
		
		/**
		 * Calculates the OAuth signature. This method combines the calls to
		 * generateBaseString() and calculateHMACSHA1Signature().
		 */
		static QString calculateOAuthSignature(QString httpMethod, QString uri,
		                                       ParameterMap parameterMap,
		                                       QString consumerSecret, QString oauthTokenSecret);
};

}

#endif //__OAUTHUTIL_H
