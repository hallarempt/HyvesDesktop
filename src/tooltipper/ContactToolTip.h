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

#ifndef CONTACTTOOLTIP_H
#define CONTACTTOOLTIP_H

#include <QVariantMap>
#include <QWidget>

class QWebView;

namespace ToolTipper {

class ContactToolTip : public QWidget {
	
	Q_OBJECT
	
	public:
		ContactToolTip(const QVariantMap &parameters, QWidget *parent = 0);
		virtual ~ContactToolTip();
		
		bool isHovered() const;
		
	public slots:
		QString parametersJSON() const;
		
		void close();
		
	protected:
		virtual void enterEvent(QEvent *event);
		virtual void leaveEvent(QEvent *event);
		
	private:
		QString m_jid;
		QVariantMap m_parameters;
		
		QWebView *m_webView;
		
		bool m_isHovered;
		
	private slots:
		void loadFinished(bool ok);
		void javaScriptWindowObjectCleared();
};

} // namespace ToolTipper

#endif // CONTACTTOOLTIP_H
