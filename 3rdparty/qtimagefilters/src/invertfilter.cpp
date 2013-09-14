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
#include "invertfilter.h"

InvertFilter::InvertFilter()
	: QtImageFilter()
{
}

InvertFilter::~InvertFilter()
{
}

QImage InvertFilter::apply(const QImage &image, const QRect& clipRect)
{
        QMutexLocker locker(&m_mutex);

	QImage resultImg = image.convertToFormat(QImage::Format_ARGB32);	
	
	QRect rc = clipRect;
	if (rc.isEmpty()) {
		rc = image.rect();
	}
	m_isAborting = false;
 
	for (int y = rc.y(); y <= rc.bottom(); ++y) {
		if (m_isAborting) {
			return QImage();
		}
		for (int x = rc.x(); x <= rc.right(); ++ x) {
			QRgb pixel = resultImg.pixel(x, y);
			resultImg.setPixel(x, y, qRgb(~qRed(pixel), ~qGreen(pixel), ~qBlue(pixel)));
		}
		setProgress(int((y - rc.y()) * 100.0 / rc.height()));
	}
	
	return resultImg;
}

QString InvertFilter::name() const
{
	return QObject::tr("Invert");
}

QString InvertFilter::description() const
{
	return QObject::tr("Invert the pixels in the image");
}
	
