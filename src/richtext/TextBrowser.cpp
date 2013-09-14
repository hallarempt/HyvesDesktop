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

#include <QMessageBox>
#include <QNetworkReply>

#include "settingsmanager/SettingsManager.h"
#include "NetworkAccessManager.h"
#include "TextBrowser.h"
#include "TextDocument.h"

namespace RichText {

TextBrowser::TextBrowser(QWidget *parent) :
	QTextBrowser(parent) {
	
	setDocument(new TextDocument(this));
	setUndoRedoEnabled(false);
	setReadOnly(true);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setFrameStyle(QFrame::NoFrame);
	setContextMenuPolicy(Qt::PreventContextMenu);
	
	// Default policy for QAbstractScrollArea is Expanding, Expanding.
	QSizePolicy sz(QSizePolicy::Preferred, QSizePolicy::Preferred, QSizePolicy::Label);
	sz.setHeightForWidth(true); // ##: this need to depend on word wrap
	setSizePolicy(sz);
}

QSize TextBrowser::sizeForWidth(int width) const {
	
	QTextDocument *doc = document();
	const qreal oldTextWidth = doc->textWidth();
	if (width == -1) {
		doc->adjustSize();
	} else {
		doc->setTextWidth(width);
	}
	QSize size = doc->size().toSize();
	doc->setTextWidth(oldTextWidth);
	return size;
}

int TextBrowser::heightForWidth(int width) const {
	
	return sizeForWidth(width).height();
}

QSize TextBrowser::minimumSizeHint() const {
	
	QSize size;
	size.rwidth() = sizeForWidth(0).width(); // width of biggest word
	size.rheight() = sizeForWidth(QWIDGETSIZE_MAX).height(); // height of one line
	return size;
}

QSize TextBrowser::sizeHint() const {
	
	return sizeForWidth(-1);
}

TextBrowser::~TextBrowser() {
}

} // namespace RichText
