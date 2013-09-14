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

#ifndef WINDOWBASE_H
#define WINDOWBASE_H

#include <QString>
#include <QVariantMap>

#include "hyveslib_export.h"

class QCloseEvent;
class QEvent;
class QMoveEvent;
class QWebFrame;
class QWebPage;

namespace WindowManager {

class HYVESLIB_EXPORT WindowBase {
	
	public:
		WindowBase(const QString &windowClass = QString::null);
		virtual ~WindowBase();
		
		virtual void setWebPage(QWebPage *webPage);
		
		virtual QWebFrame *mainFrame() const = 0;
		
		virtual int windowId() const;
		
		virtual QString windowClass() const;
		
		virtual void initialize(const QString &argumentsJSON);
		
		virtual void executeCommand(const QString &command, const QString &arguments);
		
	protected:
		virtual void bringToFront();
		virtual void minimize();
		virtual void close();
		virtual void documentWrite(const QString &arguments);
		virtual void flash(const QMap<QString, QString> &arguments);
		virtual void navigate(const QMap<QString, QString> &arguments);
		virtual void update(const QVariantMap &arguments);
		
		virtual void loadJsFile(const QString &path);
		virtual void executeJs(const QString &statement);
		
		virtual void changeEvent(QEvent *event);
		virtual void moveEvent(QMoveEvent *event);
		
		static QMap<QString, QString> argumentStringToMap(const QString &arguments);
		
	private:
		struct Private;
		Private *const m_d;
};

} // namespace WindowManager

#endif // WINDOWBASE_H
