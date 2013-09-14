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

#include <QDesktopServices>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "extender/Extender.h"
#include "logger/Logger.h"
#include "settingsmanager/SettingsManager.h"
#include "tester/Tester.h"
#include "windowmanager/WindowManager.h"
#include "NetworkAccessManager.h"
#include "WebPage.h"

struct WebPage::Private {
	Logger::Logger *log;
	Logger::Logger *jslog;
	
	bool openLinksInUserBrowser;
	QUrl loadingUrl;
	
	Private() :
		log(0),
		jslog(0),
		openLinksInUserBrowser(true) {
	}
	
	~Private() {
		delete log;
		delete jslog;
	}
};

WebPage::WebPage(QObject *parent) :
	QWebPage(parent),
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("WebPage");
	m_d->jslog = new Logger::Logger("JS-Alert");
	
	setOpenLinksInUserBrowser(true);
	
	setNetworkAccessManager(NetworkAccessManager::instance());
	setForwardUnsupportedContent(true);
	connect(this, SIGNAL(unsupportedContent(QNetworkReply*)), SLOT(receivedUnsupportedContent(QNetworkReply*)));
}

WebPage::~WebPage() {
}

bool WebPage::openLinksInUserBrowser() const {
	
	return m_d->openLinksInUserBrowser;
}

void WebPage::setOpenLinksInUserBrowser(bool openLinksInUserBrowser) {
	
	m_d->openLinksInUserBrowser = openLinksInUserBrowser;
	
	if (m_d->openLinksInUserBrowser) {
		setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);
		connect(this, SIGNAL(linkClicked(QUrl)), this, SLOT(openExternalUrl(QUrl)));
	} else {
		setLinkDelegationPolicy(QWebPage::DontDelegateLinks);
		disconnect(this, SIGNAL(linkClicked(QUrl)), this, SLOT(openExternalUrl(QUrl)));
	}
}

void WebPage::triggerAction(QWebPage::WebAction action, bool checked) {
	
	if (action == QWebPage::Back || action == QWebPage::Forward ||
	    action == QWebPage::Reload || action == QWebPage::Stop) {
		return; // ignore these actions
	}
	
	QWebPage::triggerAction(action, checked);
}

QUrl WebPage::loadingUrl() const {
	
	return m_d->loadingUrl;
}

void WebPage::javaScriptAlert(QWebFrame *frame, const QString &message) {
	
	if (Tester::Tester::isTesting() &&
	    (message.contains("ERROR") || message.contains("FATAL"))) {
		Tester::Tester::instance()->failTest(message);
	} else {
		m_d->jslog->debug(message);
	}
}

void WebPage::javaScriptConsoleMessage(const QString &message, int lineNumber,
                                       const QString &sourceId) {
	
	QString source(sourceId);
	QString baseServer = SettingsManager::SettingsManager::instance()->baseServer();	
	if (source.contains(baseServer)) {
		source = sourceId.mid(source.indexOf(baseServer) + baseServer.length());
	}
	if (source.contains("&pageid=")) {
		source = source.left(source.indexOf("&pageid="));
	}
	bool isErrorMsg = message.startsWith("ReferenceError") || message.startsWith("SyntaxError") || message.startsWith("TypeError");
	
	if (Tester::Tester::isTesting()) {
		if (!message.startsWith("Assertion failed") && !message.startsWith("Test failed")) {
			m_d->log->warning(QString("%1 (line %2): " + message).arg(source).arg(lineNumber));
		}
		if (isErrorMsg) {
			Tester::Tester::instance()->failTest(message);
		}
	} else {
		if (isErrorMsg) {
			m_d->log->error(QString("%1 (line %2): " + message).arg(source).arg(lineNumber));
		} else {
			m_d->log->warning(QString("%1 (line %2): " + message).arg(source).arg(lineNumber));
		}
		
		if (isErrorMsg && Extender::Extender::instance()->currentExecutingStatement() != QString::null) {
			m_d->log->error("In statement: " + Extender::Extender::instance()->currentExecutingStatement());
		}
	}
}

void WebPage::openExternalUrl(const QUrl &url) {
	
	QDesktopServices::openUrl(url);
}

bool WebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type) {
	
	if (frame) {
		bool result = QWebPage::acceptNavigationRequest(frame, request, type);
		
		if (result) {
			m_d->loadingUrl = request.url();
		}
		
		return result;
	} else {
		if (m_d->openLinksInUserBrowser) {
			// if there is no frame, we want to navigate to a new window
			QDesktopServices::openUrl(request.url());
			return true; 
		} else {
			return QWebPage::acceptNavigationRequest(frame, request, type);
		}
	}
}

void WebPage::receivedUnsupportedContent(QNetworkReply *reply) {
	
	m_d->log->error(QString("Received unsupported content (url: %1 NetworkError: %2)").arg(reply->url().toString()).arg(reply->error()));
}

