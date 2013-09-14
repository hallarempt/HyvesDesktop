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

#ifndef TEXTBROWSER_H
#define TEXTBROWSER_H

#include <QTextBrowser>

#include "hyveslib_export.h"

namespace RichText {

/**
 * Browser is a widget that displays html with animated gif. Since
 * Browser is a QTextBrowser, you can use all the functionality of
 * QTextBrowser - selection behavior, signals for link clicks.
 *
 * Browser add heightForWidth support. Currently, it assumes that
 * some form of word wrapping is enabled (i.e NoWrap does not work).
 *
 * @author Girish Ramakrishnan <girish@forwardbias.in>
 */
class HYVESLIB_EXPORT TextBrowser : public QTextBrowser {
	
	Q_OBJECT
	
	public:
		TextBrowser(QWidget *parent = 0);
		~TextBrowser();
		
		QSize minimumSizeHint() const;
		QSize sizeHint() const;
		
		int heightForWidth(int width) const;
		
	private:
		QSize sizeForWidth(int width) const;
};

} // namespace RichText

#endif // TEXTBROWSER_H
