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

#include "WideSplitter.h"

WideSplitterHandle::WideSplitterHandle(Qt::Orientation orientation, QSplitter *parent) :
	QSplitterHandle(orientation, parent) {
}

// Paint the horizontal handle as a gradient, paint
// the vertical handle as a line.
void WideSplitterHandle::paintEvent(QPaintEvent *) {
	
	QPainter painter(this);
	
	QColor topColor(207, 223, 239);
	QColor textColor(62, 83, 118);
	QColor gradientTop(252, 253, 254);
	QColor gradientBottom(237, 243, 249);
	
	if (orientation() == Qt::Vertical) {
		painter.setPen(topColor);
		painter.drawLine(0, 0, width(), 0);
		
		QLinearGradient linearGradient(QPointF(0, 0), QPointF(0, height() - 3));
		linearGradient.setColorAt(0, gradientTop);
		linearGradient.setColorAt(1, gradientBottom);
		painter.fillRect(QRect(QPoint(0,1), size() - QSize(0, 1)), QBrush(linearGradient));
		
		QFont font;
		font.setBold(true);
		painter.setFont(font);
		painter.setPen(textColor);
		painter.drawText(11, 20, tr("Friend buzz"));
		
		QImage splitterImage(":/data/splitterhandle.png", "PNG");
		painter.drawImage(width() - 22, 9, splitterImage);
	} else {
		painter.setPen(topColor);
		painter.drawLine(0, 0, 0, height());
	}
}

QSize WideSplitterHandle::sizeHint() const {
	
	QSize parent = QSplitterHandle::sizeHint();
	if (orientation() == Qt::Vertical) {
		return QSize(parent.width(), 29);
	} else {
		return QSize(1, parent.height());
	}
}

void WideSplitterHandle::mouseDoubleClickEvent(QMouseEvent *) {
	
	emit doubleClicked();
}

QSplitterHandle *WideSplitter::createHandle() {
	
	return new WideSplitterHandle(orientation(), this);
}
