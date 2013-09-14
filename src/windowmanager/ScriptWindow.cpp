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

#include <QAction>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QWebFrame>

#include "config.h"
#include "extender/Extender.h"
#include "logger/Logger.h"
#include "ScriptWindow.h"
#include "WebPage.h"
#include "WebView.h"

namespace WindowManager {

struct ScriptWindow::Private {
	Logger::Logger *log;
	
	QWebView *webView;
	
	Private() :
		log(0) {
	}
	
	~Private() {
		
		delete log;
	}
};

ScriptWindow::ScriptWindow(const QString &windowClass, QWidget *parent) :
#ifdef Q_WS_MAC
	QWidget(parent, Qt::Window),
#else
	QWidget(0, Qt::Window),
#endif
	WindowBase(windowClass),
	m_d(new Private()) {
	
	QList<QKeySequence> closeKeys;
	closeKeys << QKeySequence::Close << Qt::Key_Escape;
	QAction *closeAction = new QAction(this);
	closeAction->setShortcuts(closeKeys);
	connect(closeAction, SIGNAL(triggered()), SLOT(close()));
	addAction(closeAction);
	
	m_d->log = new Logger::Logger("ScriptWindow");
	
	setAttribute(Qt::WA_DeleteOnClose, true);
	
	m_d->webView = new WebView(this);
	
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(m_d->webView);
}

ScriptWindow::~ScriptWindow() {
	
	mainFrame()->evaluateJavaScript("if (typeof windowClosed != 'undefined') windowClosed()");
	
	delete m_d;
}

void ScriptWindow::setWebPage(QWebPage *webPage) {
	
	Extender::Extender::instance()->unregisterEnvironment(m_d->webView->page()->mainFrame());
	
	Q_ASSERT(webPage);
	webPage->setParent(m_d->webView);
	m_d->webView->setPage(webPage);
	webPage->view()->setAcceptDrops(false);	
	
	Extender::Extender::instance()->registerEnvironment(webPage->mainFrame());
	
	connect(webPage, SIGNAL(loadFinished(bool)), SLOT(webPageLoadFinished()));
}

QWebFrame *ScriptWindow::mainFrame() const {
	
	return m_d->webView->page()->mainFrame();
}

void ScriptWindow::changeEvent(QEvent *event) {
	
	// avoid ambiguous inheritance
	WindowBase::changeEvent(event);
	QWidget::changeEvent(event);
}

void ScriptWindow::closeEvent(QCloseEvent *event) {
	
	QWidget::closeEvent(event);
}

void ScriptWindow::moveEvent(QMoveEvent *event) {
	
	// avoid ambiguous inheritance
	WindowBase::moveEvent(event);
	QWidget::moveEvent(event);
}

void ScriptWindow::webPageLoadFinished() {
	
	emit loadFinished(windowId());
}

} // namespace WindowManager
