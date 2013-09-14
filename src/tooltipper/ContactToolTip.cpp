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
#include <QWebFrame>
#include <QWebView>

#include "mediacache/MediaCache.h"
#include "windowmanager/WindowManager.h"
#include "ContactToolTip.h"
#include "Util.h"
#include "WebView.h"

namespace ToolTipper {

ContactToolTip::ContactToolTip(const QVariantMap &parameters, QWidget *parent) :
	QWidget(parent),
	m_parameters(parameters),
	m_isHovered(false) {
	
	setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
	setVisible(false);
	
	resize(parameters["width"].toInt(), 103);
	
	m_webView = new WebView(this);
	m_webView->setUrl(QUrl("http://localhost/statics/kwekker2/tooltip.html"));
	m_webView->setContextMenuPolicy(Qt::NoContextMenu);
	
	connect(m_webView, SIGNAL(loadFinished(bool)), SLOT(loadFinished(bool)));
	connect(m_webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), SLOT(javaScriptWindowObjectCleared()));
}

ContactToolTip::~ContactToolTip() {
}

bool ContactToolTip::isHovered() const {
	
	return m_isHovered;
}

QString ContactToolTip::parametersJSON() const {
	
	return Util::variantMapToJSON(m_parameters);
}

void ContactToolTip::close() {
	
	deleteLater();
}

void ContactToolTip::enterEvent(QEvent *) {
	
	m_isHovered = true;
}

void ContactToolTip::leaveEvent(QEvent *) {
	
	deleteLater();
}

void ContactToolTip::loadFinished(bool ok) {
	
	resize(m_parameters["width"].toInt(), m_webView->page()->mainFrame()->evaluateJavaScript("document.body.offsetHeight").toInt());
	
	show();
}

void ContactToolTip::javaScriptWindowObjectCleared() {
	
	m_webView->page()->mainFrame()->addToJavaScriptWindowObject("toolTip", this);
}

} // namespace ToolTipper
