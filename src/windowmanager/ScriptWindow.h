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

#ifndef SCRIPTWINDOW_H
#define SCRIPTWINDOW_H

#include <QString>
#include <QWidget>

#include "WindowBase.h"

class QCloseEvent;

namespace WindowManager {

class ScriptWindow : public QWidget, public WindowBase {
	
	Q_OBJECT
	
	public:
		ScriptWindow(const QString &windowClass, QWidget* parent = 0);
		virtual ~ScriptWindow();
		
		void setWebPage(QWebPage *webPage);
		
		QWebFrame *mainFrame() const;
		
	signals:
		/**
		 * This signal is emitted whenever the webpage inside the window has
		 * finished loading a page.
		 */
		void loadFinished(int windowId);
		
	protected:
		virtual void changeEvent(QEvent *event);
		virtual void closeEvent(QCloseEvent *event);
		virtual void moveEvent(QMoveEvent *event);
		
	private:
		struct Private;
		Private *const m_d;
		
	private slots:
		void webPageLoadFinished();
};

} // namespace WindowManager

#endif // SCRIPTWINDOW_H
