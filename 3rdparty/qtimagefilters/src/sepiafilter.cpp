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

#include "sepiafilter.h"
#include "blendingmodes.h"
#include <QColor>
	

SepiaFilter::SepiaFilter()
		: QtImageFilter()
{
}

SepiaFilter::~SepiaFilter()
{
}

static QRgb qSepia(QRgb rgb) {
	// somewhat like this: http://www.gimp.org/tutorials/Sepia_Toning/
	QRgb sepia = qRgb(162, 138, 101);
	QRgb gray = qRgb(qGray(rgb), qGray(rgb), qGray(rgb));
	return Blending::color(sepia, gray);
}

QImage SepiaFilter::apply(const QImage& image, const QRect& clipRect)
{
        QMutexLocker locker(&m_mutex);

	QImage resultImg = image.convertToFormat(QImage::Format_ARGB32);	

	QRect rc = clipRect;
	if (rc.isEmpty()) {
		rc = image.rect();
	}

	m_isAborting = false;

	for (int y = rc.y(); y <= rc.bottom(); ++y) {
		QRgb* line = reinterpret_cast<QRgb*>(resultImg.scanLine(y));
		for (int x = rc.x(); x <= rc.right(); ++x) {
			line[x] = qSepia(line[x]);
		}
		if (m_isAborting) {
			return QImage();
		}
		setProgress(int((y - rc.y()) * 100.0 / rc.height()));
	}

	return resultImg;
}

QString SepiaFilter::name() const {
	
	return QObject::tr("Sepia");
}

QString SepiaFilter::description() const {
	
	return QObject::tr("Color the image sepia");
}

