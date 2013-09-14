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

#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <QWebPage>

#include "hyveslib_export.h"

class HYVESLIB_EXPORT WebPage : public QWebPage {
	
	Q_OBJECT
	
	public:
		WebPage(QObject *parent = 0);
		virtual ~WebPage();
		
		bool openLinksInUserBrowser() const;
		void setOpenLinksInUserBrowser(bool openLinksInUserBrowser);
		
		virtual void triggerAction(QWebPage::WebAction action, bool checked = false);
		
		/**
		 * Returns the URL was last loaded.
		 * 
		 * As opposed to url(), this URL may not have succeeded in loading.
		 */
		QUrl loadingUrl() const;
		
	protected:
		struct Private;
		Private *const m_d;
		
		virtual void javaScriptAlert(QWebFrame *frame, const QString &message);
		virtual void javaScriptConsoleMessage(const QString &message, int lineNumber,
		                                      const QString &sourceId);
		
		bool acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type);
		
	private slots:
		void openExternalUrl(const QUrl &url);
		void receivedUnsupportedContent(QNetworkReply *reply);
};

#endif // WEBPAGE_H
