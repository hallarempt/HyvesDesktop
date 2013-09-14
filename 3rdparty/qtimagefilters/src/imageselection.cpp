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
 * Foundation, Inc., 59		Temple Place, Suite 330, Boston, MA  02111-1307  US
 */
#include "imageselection.h"
#include <QPainterPath>
#include <QDir>
#include <QPainter>

ImageSelection::ImageSelection()
		: QImage()
		, m_threshold(128)
{
}

ImageSelection::ImageSelection(const QSize& size)
		: QImage(size, QImage::Format_ARGB32)
		, m_threshold(128)
{
	clear();
}

ImageSelection ImageSelection::scaled(const QSize& size, Qt::AspectRatioMode mode) const {
	QImage s = QImage::scaled(size, mode, Qt::SmoothTransformation);
	ImageSelection ret = ImageSelection(s.size());
	QPainter gc(&ret);
	gc.drawImage(0, 0, s, 0, 0, s.width(), s.height());
	gc.end();
	return ret;
}

void ImageSelection::setSelectionThreshold(quint8 threshold)
{
	m_threshold = threshold;
}

bool ImageSelection::isSelected(QPoint pt)
{
	return qRed(pixel(pt)) >= m_threshold;
}

quint8 ImageSelection::selectedAt(QPoint pt)
{
	return qRed(pixel(pt));
}

void ImageSelection::clear()
{
	if (!isNull()) {
		QPainter gc(this);
		gc.fillRect(rect(), Qt::black);
		gc.end();
	}
}

void ImageSelection::addPainterPath(const QPainterPath& path)
{
	QPainter gc(this);
	gc.setRenderHint(QPainter::Antialiasing, true);
	gc.setBrush(Qt::white);
	gc.drawPath(path);
	gc.end();
	
}

void ImageSelection::subtractPainterPath(const QPainterPath& path)
{
	QPainter gc(this);
	gc.setRenderHint(QPainter::Antialiasing, true);
	gc.setBrush(Qt::black);
	gc.drawPath(path);
	gc.end();
}
