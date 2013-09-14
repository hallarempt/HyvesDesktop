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

#include <QStringList>

#include "OAuthAccessToken.h"
#include "OAuthRequestToken.h"
#include "ResultDocument.h"

namespace GenusApis {

ResultDocument::ResultDocument() :
	QDomDocument() {
}

ResultDocument::~ResultDocument() {
}

QString ResultDocument::property(QString path) const {
	
	QStringList components = path.split("/");
	
	if(!components[0].isEmpty()) {
		qDebug("ResultDocument::property(): Only absolute paths are supported.");
		return "";
	}
	
	QDomNode node = *this;
	for(int i = 1; i < components.size(); i++) {
		QString component = components[i];
		int index = 0;
		
		int squareOffset = component.indexOf('[');
		if(squareOffset != -1) {
			index = component.mid(squareOffset + 1, component.size() - squareOffset - 2).toInt() - 1;
			if(index < 1) {
				qDebug("ResultDocument::property(): Illegal offset in %s.", component.toLatin1().data());
				return "";
			}
			component = component.left(squareOffset);
		}
		
		if(component.isEmpty()) {
			qDebug("ResultDocument::property(): Empty path element.");
			return "";
		}
		
		QDomNode childNode = node.firstChild();
		while(!childNode.isNull()) {
			if(component == "*" || childNode.nodeName() == component) {
				if(index == 0) {
					break;
				} else {
					index--;
				}
			}
			childNode = childNode.nextSibling();
		}
		
		if(childNode.isNull()) {
			return ""; // property not found
		}
		
		node = childNode;
	}
	
	return node.toElement().text();
}

int ResultDocument::errorCode() const {
	
	return property("/error_result/error_code").toInt();
}

QString ResultDocument::errorMessage() const {

	return property("/error_result/error_message");
}

OAuthRequestToken *ResultDocument::toRequestToken() const {
	
	QString key = property("/auth_requesttoken_result/oauth_token");
	QString secret = property("/auth_requesttoken_result/oauth_token_secret");
	
	if(key == "" || secret == "") {
		return 0;
	}
	
	return new OAuthRequestToken(key, secret);
}

OAuthAccessToken *ResultDocument::toAccessToken(const QString &rootElement) const {
	
	QString key = property("/" + rootElement + "/oauth_token");
	QString secret = property("/" + rootElement + "/oauth_token_secret");
	QString userId = property("/" + rootElement + "/userid");
	QString methods = property("/" + rootElement + "/methods");
	int expireDate = property("/" + rootElement + "/expiredate").toInt();
	
	if(key == "" || secret == "" || userId == "" || methods == "" || expireDate == 0) {
		return 0;
	}
	
	return new OAuthAccessToken(key, secret, userId, methods, expireDate);
}

void ResultDocument::setError(int c, const QString &m) {

	QDomElement errorResult = createElement("error_result");
	appendChild(errorResult);
	QDomElement errorCode = createElement("error_code");
	errorResult.appendChild(errorCode);
	QDomText code = createTextNode(QString::number(c));
	errorCode.appendChild(code);
	
	QDomElement errorMessage = createElement("error_message");
	errorResult.appendChild(errorMessage);
	QDomText message = createTextNode(m);
	errorMessage.appendChild(message);
}

}
