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
#include <QContextMenuEvent>
#include <QDropEvent>
#include <QMenu>
#include <QDebug>
#include <QWebFrame>

#include "config.h"
#include "extender/Extender.h"
#include "logger/Logger.h"
#include "WebPage.h"
#include "WebView.h"

struct WebView::Private {
	Logger::Logger *log;
	
	QString jsForReload;
	bool ignoreMouseMoveEvents;
	
	Private() :
			log(0),
			ignoreMouseMoveEvents(false) {
	}
	
	~Private() {
		
		delete log;
	}
};

WebView::WebView(QWidget *parent) :
		QWebView(parent),
		m_d(new Private()) {
	
	m_d->log = new Logger::Logger("WebView");
	
	WebPage *webPage = new WebPage(this);
	setPage(webPage);
	
	QString defaultFontFamily = qApp->font().family();
	settings()->setFontFamily(QWebSettings::StandardFont, defaultFontFamily);
	settings()->setFontFamily(QWebSettings::SerifFont, defaultFontFamily);
	settings()->setAttribute(QWebSettings::PluginsEnabled, true);
	
	connect(this, SIGNAL(loadStarted()), SLOT(loadStarted()));
	connect(this, SIGNAL(loadFinished(bool)), SLOT(loadFinished(bool)));
	connect(this, SIGNAL(loadFinished(bool)), SLOT(evaluateWindowResized()));
	
	Extender::Extender::instance()->registerEnvironment(webPage->mainFrame());
	
	initShortcuts();
}

WebView::~WebView() {
	
	Extender::Extender::instance()->unregisterEnvironment(page()->mainFrame());
	
	delete m_d;
}

Qt::Modifier WebView::accessKeyModifier() {
	
#ifdef Q_WS_MAC
	return Qt::CTRL; // the command key in the mac
#else
	return Qt::ALT;
#endif
}

void WebView::setJsForReload(const QString &js) {
	
	m_d->jsForReload = js;
}

QString WebView::jsForReload() const {
	
	return m_d->jsForReload;
}

void WebView::displayErrorPage(const QString &title, const QString &contentHtml) {
	
	setHtml(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">"
			"<html>"
			"<head>"
			"<link rel=\"stylesheet\" href=\"http://localhost/statics/style20.css\" type=\"text/css\">"
			"<link rel=\"stylesheet\" href=\"http://localhost/statics/kwekker2/client.css\" type=\"text/css\">"
			"<style type=\"text/css\">"
			".ChatBody {"
			"margin: 0px;"
			"background-repeat: repeat-x;"
			"background-position: left top;"
			"overflow: hidden;"
			"}"
			"</style>"
			"<script>"
			"function reload() {"
			"	%3"
			"}"
			"</script>"
			"</head>"
			"<body class=\"ChatBody BodyText\">"
			"<div class=\"ChatHeader\" style=\"background-image: url(http://localhost/images/hyppo/login_header.png); width: 100%; border-bottom: 1px solid #fff\">"
			"<img src=\"http://localhost/images/hyppo/chat_logo_trans2.png\">"
			"</div>"
			"<div class=\"MenuBlue\" style=\"height: 7px; margin-bottom: 10px\">&nbsp;</div>"
			"<p class=\"SubjectNoLink\" style=\"padding: 3px 10px 3px 10px;\">%1</p>"
			"%2"
			"</body>"
			"</html>"
			)
		.arg(title)
		.arg(contentHtml)
		.arg(jsForReload()).toUtf8(), QUrl("http://localhost/"));
}

void WebView::contextMenuEvent(QContextMenuEvent *event) {
	
	QMenu *menu = new QMenu();
	
	QList<QWebPage::WebAction> webActions;
	webActions << QWebPage::Cut << QWebPage::Copy << QWebPage::Paste;
#ifdef DEBUG_BUILD
	webActions << QWebPage::InspectElement;
#endif
	bool show = false;	
	foreach (QWebPage::WebAction webAction, webActions) {
		QAction *action = pageAction(webAction);
		if (action->isEnabled()) {
			menu->addAction(action);
			show = true;
		}
	}
	if (show) {	
		menu->exec(event->globalPos());
	}
	delete menu;
}

void WebView::dropEvent(QDropEvent *event) {
	
	event->ignore();
}

void WebView::resizeEvent(QResizeEvent *event) {
	
	evaluateWindowResized();
	QWebView::resizeEvent(event);
}

void WebView::keyPressEvent(QKeyEvent *event) {
	
	emulateKeyPressInJavaScript(event);
	QWebView::keyPressEvent(event);
}

void WebView::initShortcuts() {
	
	QList<QWebPage::WebAction> webActions;
	webActions << QWebPage::Cut << QWebPage::Copy << QWebPage::Paste;
	
	QList<QKeySequence::StandardKey> shortcuts;
	shortcuts << QKeySequence::Cut << QKeySequence::Copy << QKeySequence::Paste;
	
	for (int i = 0; i < webActions.size(); i++) {
		QWebPage::WebAction webAction = webActions[i];
		QKeySequence::StandardKey shortcut = shortcuts[i];
		
		QAction *action = pageAction(webAction);
		action->setShortcut(shortcut);
		addAction(action);
	}
}

void WebView::emulateKeyPressInJavaScript(QKeyEvent *event) {
	
	// event->text() doesnt work with ctl-<something> key events, so:
	QString keyText = QString(QChar::fromAscii(event->key()));
	
	if (keyText.isEmpty()) {
		return;
	}
	QString shiftPressed = "false";
	QString controlPressed = "false";
	QString altPressed = "false";
	QString metaPressed = "false";
	
	// The access key modifier for QtWebkit is Ctrl key in all platforms.
	// So let's set that to true when our desired modifier is used.
	if ((event->modifiers() & accessKeyModifier()) == (int) accessKeyModifier()) {
		controlPressed = "true";
	}
	
	// the initKeyboardEvent call below is based on the declaration of
	// initKeyboardEvent at <qt-src>/src/3rdparty/webkit/WebCore/dom/KeyboardEvent.cpp
	page()->mainFrame()->evaluateJavaScript( QString("") +
						 "var event = document.createEvent(\"KeyboardEvent\");\n" +
						 "event.initKeyboardEvent(\"keydown\", true, true, document.defaultView, " +
						 "\"" + keyText + "\", 0, " +
						 controlPressed + ", " +
						 altPressed + ", " +
						 shiftPressed + ", " +
						 metaPressed + ", " +
						 "false );\n" +
						 "document.dispatchEvent(event);" );
}

void WebView::setFocus(Qt::FocusReason reason) {
	
	QWebView::setFocus(reason);
}

void WebView::setIgnoreMouseMove(bool ignore) {
	
	m_d->ignoreMouseMoveEvents = ignore;
}

void WebView::evaluateWindowResized() {
	
	page()->mainFrame()->evaluateJavaScript(QString("if (typeof windowResized != 'undefined') windowResized(%1, %2)").arg(size().width()).arg(size().height()));
	update();
}

void WebView::loadStarted() {
	
	setCursor(QCursor(Qt::WaitCursor));
}

void WebView::loadFinished(bool ok) {
	
	if (!ok) {
		QUrl loadingUrl = url();
		WebPage *webPage = qobject_cast<WebPage *>(page());
		if (webPage) {
			loadingUrl = webPage->loadingUrl();
		}
		
		displayErrorPage(
				tr("Cannot connect to Hyves"),
				QString("<ul>"
					"<li>%2</li>"
					"<li>%3</li>"
					"<li>%4</li>"
					"<li>%5</li>"
					"</ul>"
					"<p style=\"padding: 0px 10px\"><button class=\"large\" onclick=\"reload()\"><span>%6</span></button></p>"
					"<br><br><hr>"
					"<p style=\"color: #555\">URL: %7</p>")
				.arg(tr("Are you connected to the internet? Try opening some pages in a webbrowser."))
				.arg(tr("Is Hyves Desktop allowed to connect to the internet? Hyves Desktop could be blocked by a firewall. Make sure Hyves Desktop is allowed to connect to ports 80, 443, and 5222."))
				.arg(tr("Is the Hyves website working? Try navigating to <a href=\"http://www.hyves.nl/\">www.hyves.nl</a> in your webbrowser. If the website is not working, please be patient until the site is working again."))
				.arg(tr("Still having trouble? Please <a href=\"http://www.hyves.nl/help/feedback?faqid=816&amp;topicid=301\">leave your feedback</a>."))
				.arg(tr("Try Again"))
				.arg(loadingUrl.toString())
				);
		m_d->log->error(QString("Failed to load url (%1)").arg(loadingUrl.toString()));
	}
	
	unsetCursor();
}

void WebView::mouseMoveEvent(QMouseEvent *event) {
	if (event->buttons() && m_d->ignoreMouseMoveEvents) {
		
		return;
	}
	QWebView::mouseMoveEvent(event);
}
