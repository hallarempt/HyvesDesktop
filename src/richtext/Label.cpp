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

#include <QtGui>

#include "Label.h"
#include "TextDocument.h"

namespace RichText {

Label::Label(QWidget *parent) :
	QWidget(parent) {
	
	m_doc = new TextDocument(this);
	connect(m_doc, SIGNAL(contentsChange(int, int, int)), this, SLOT(update()));
}

Label::Label(const QString &html, QWidget *parent) :
	QWidget(parent) {
	
	m_doc = new TextDocument(this);
	connect(m_doc, SIGNAL(contentsChange(int, int, int)), this, SLOT(update()));
	
	setHtml(html);
}

Label::~Label() {
}

void Label::updateLabel() {

	updateGeometry();
	update();
}

void Label::setHtml(const QString &html) {
	
	m_doc->setHtml(html);
	m_doc->adjustSize();
	updateLabel();
}

QString Label::html() const {
	
	return m_doc->toHtml();
}

TextDocument *Label::document() const {
	
	return m_doc;
}

QSize Label::sizeHint() const {
	
	return m_doc->size().toSize();
}

void Label::paintEvent(QPaintEvent *event) {
	
	QStyleOption opt;
	opt.init(this);
	QPainter painter(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this); // stylesheet background
	
	m_doc->drawContents(&painter, event->rect());
}

void Label::changeEvent(QEvent *event) {
	
	if (event->type() == QEvent::FontChange || event->type() == QEvent::ApplicationFontChange) {
		m_doc->setDefaultFont(font());
		updateLabel();
	}
	
	QWidget::changeEvent(event);
}

} // namepsace RichText
