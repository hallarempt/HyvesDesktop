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

#ifndef LABEL_H
#define LABEL_H

#include <QTextCursor>
#include <QTextDocument>
#include <QWidget>

#include "hyveslib_export.h"

namespace RichText {

class TextDocument;

/**
 * Label is a widget that displays html with animated gif. The user
 * cannot interact with the text in the Label - the cursor will not
 * change when hovered over links, text cannot be selected, link
 * clicks are not detected. In other words, use this class only when
 * the html is known to have no links.
 *
 * The text in Label always wraps and its sizeHint is always the
 * golden ratio. It also does not support heightForWidth.
 *
 * If you need user interaction or heightForWidth, use TextBrowser 
 * instead.
 *
 * @author Girish Ramakrishnan <girish@forwardbias.in>
 */
class HYVESLIB_EXPORT Label : public QWidget {
	
	Q_OBJECT
	
	public:
		Label(QWidget *parent = 0);
		Label(const QString &html, QWidget *parent = 0);
		~Label();
		
		void setHtml(const QString &html);
		QString html() const;
		
		TextDocument *document() const;
		
		QSize sizeHint() const;
		
	protected:
		void paintEvent(QPaintEvent *event);
		void changeEvent(QEvent *event);
		
	private:
		void updateLabel();

		TextDocument *m_doc;
};

} // namespace RichText

#endif // LABEL_H
