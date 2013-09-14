/*
 * Hyves Desktop, Copyright (C) 2009 Hyves (Startphone Ltd.)
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
#include "colorwell.h"

#include <QtGui>
#include <QtCore>

ColorWell::ColorWell(QWidget* parent) 
	: QToolButton(parent)
	, m_color(Qt::red) {

	setFixedSize(QSize(22,22));
	setIconSize(QSize(16,16));
	updateIcon();
	connect(this, SIGNAL(clicked()), this, SLOT(showMoreColorsDialog()));

}

ColorWell::~ColorWell() {
}

QColor ColorWell::selectedColor() {
	
	return m_color;
}

void ColorWell::setColor(QColor color) {

	m_color = color;	
	updateIcon();
}

void ColorWell::showMoreColorsDialog() {

	QColor color = QColorDialog::getColor(selectedColor(), this);
	if (color.isValid()) {
		setColor(color);
		emit colorChanged(color);
	}
}

void ColorWell::updateIcon() {
	
	// XXX: pimp this by adding a well-like border
	QSize iconSize(24, 24);
	QPixmap p(iconSize);
	QPainter gc(&p);
	gc.fillRect(0, 0, 24, 24, m_color);
	gc.end();
	setIcon(QIcon(p));
}
