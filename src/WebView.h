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

#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QSize>
#include <QString>
#include <QWebView>

#include "hyveslib_export.h"

class HYVESLIB_EXPORT WebView : public QWebView {
	
	Q_OBJECT
	
	public:
		WebView(QWidget *parent = 0);
		virtual ~WebView();
		
		static Qt::Modifier accessKeyModifier();
		
		void setJsForReload(const QString &js);
		QString jsForReload() const;
		
		void displayErrorPage(const QString &title, const QString &contentHtml);

	public slots:
		void setFocus(Qt::FocusReason reason = Qt::OtherFocusReason);
		
		/**
		 * If set to true, ignore all mouse move events with one or more buttons clicked.
		 * this disabled dragging of images and selection of text.
		 */
		void setIgnoreMouseMove(bool ignore);
		
	protected:
		void contextMenuEvent(QContextMenuEvent *event);
		void dropEvent(QDropEvent *event);
		void resizeEvent(QResizeEvent *event);
		void keyPressEvent(QKeyEvent *event);
		void mouseMoveEvent(QMouseEvent*);
				
	private:
		struct Private;
		Private *const m_d;
		
		void initShortcuts();
		
	private slots:
		void evaluateWindowResized();
		void emulateKeyPressInJavaScript(QKeyEvent *event);
		void loadStarted();
		void loadFinished(bool ok);
};

#endif // WEBVIEW_H
