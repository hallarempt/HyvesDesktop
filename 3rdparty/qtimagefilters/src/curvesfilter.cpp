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
#include "curvesfilter.h"
#include "colorconversions.h"

#include <QtGui>
#include <QtCore>

inline uint UINT8_TO_UINT16(uint c)
{
    return c | (c<<8);
}


inline uint UINT16_TO_UINT8(uint c)
{
    c = c - (c >> 8) + 128;
    return c >>8;
}


CurvesFilter::CurvesFilter()
		: QtImageFilter()
{
	resetOptions();
}

void CurvesFilter::resetOptions()
{
	m_type = RGB;
	// start with a completely straight curve
	for (int i = 0; i < 256; ++i) {
		m_curve << UINT8_TO_UINT16(i);
	}	
	QtImageFilter::handleControlValueChanged();
}

CurvesFilter::~CurvesFilter()
{
}

QImage CurvesFilter::apply(const QImage &image, const QRect& clipRect)
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
		QRgb* line = reinterpret_cast<QRgb*>(resultImg.scanLine(y));
		
		for (int x = rc.x(); x <= rc.right(); ++x) {

			switch (m_type) {
			case RGB:
				{
					line[x] = qRgba(UINT16_TO_UINT8(m_curve[qRed(line[x])]),
							UINT16_TO_UINT8(m_curve[qGreen(line[x])]),
						        UINT16_TO_UINT8(m_curve[qBlue(line[x])]),
						        qAlpha(line[x]));
				}
			case Y: 
				{
					int Y, u, v;
					ColorConversions::rgbToYuv(line[x], &Y, &u, &v);
					Q_ASSERT( Y < m_curve.size());
					Y = UINT16_TO_UINT8(m_curve[Y]);
					line[x] = ColorConversions::yuvToRgb(Y, u, v);
				}
		}
		}
		setProgress(int((y - rc.y()) * 100.0 / rc.height()));
	}

	return resultImg;
	
	return resultImg;}

QString CurvesFilter::name() const
{
	return QObject::tr("Brightness/Contrast");
}

QString CurvesFilter::description() const
{
 	return QObject::tr("Adjust the brightness/contrast of an image");
}
	

QVariant CurvesFilter::option(int option) const {
	switch (CurvesFilterOption(option)) {
		case Curve:
			return QVariant::fromValue(m_curve);
	}
	return QVariant();
}

bool CurvesFilter::setOption(int option, const QVariant &value) {
	switch (CurvesFilterOption(option)) {
		case Curve:
			QVector<quint16> curve = value.value< QVector<quint16> >();
			Q_ASSERT(curve.size() >= 255);
			if (curve.size() >= 255) {
				m_curve = curve;
			}
			return true;
	}
	return false;
}

bool CurvesFilter::supportsOption(int option) const {
	switch (CurvesFilterOption(option)) {
		case Curve:
		return true;
	}
	return false;
}

QWidget *CurvesFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
        l->setMargin(2);
        l->setSpacing(2);

	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}

void CurvesFilter::handleControlValueChanged() {
	
\
	QtImageFilter::handleControlValueChanged();
}
