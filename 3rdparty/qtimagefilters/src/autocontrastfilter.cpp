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

#include "autocontrastfilter.h"

#include <QtGui>
#include <QtCore>

AutoContrastFilter::AutoContrastFilter()
	: QtImageFilter()
	, m_rgb(0)
	, m_hsv(0)
{
	resetOptions();
}

void AutoContrastFilter::resetOptions()
{
	m_mode = RGB;
	if (m_rgb) {
		m_rgb->blockSignals(true);
		m_hsv->blockSignals(true);

		m_rgb->setChecked(true);
		m_hsv->setChecked(false);

		m_rgb->blockSignals(false);
		m_hsv->blockSignals(false);
	}
	QtImageFilter::handleControlValueChanged();
	
}


AutoContrastFilter::~AutoContrastFilter()
{
}


void AutoContrastFilter::init(const QImage& image, const QRect& clipRect)
{
	m_sMin = m_vMin = 1.0;
	m_rMin = m_gMin = m_bMin = 255;
	m_sMax =  m_vMax = 1.0;
	m_rMax = m_gMax = m_bMax = 0;
	
	QRect rc = clipRect;
	
	if (rc.isEmpty()) {
		rc = image.rect();
	}
	
	for (int y = rc.y(); y <= rc.bottom(); ++y) {
		const QRgb* line = reinterpret_cast<const QRgb*>(image.scanLine(y));
		
		for (int x = rc.x(); x <= rc.right(); ++x) {
			if (m_mode == RGB) {
				m_rMin = qMin(m_rMin, qRed(line[x]));
				m_gMin = qMin(m_gMin, qGreen(line[x]));
				m_bMin = qMin(m_bMin, qBlue(line[x]));
				m_rMax = qMax(m_rMax, qRed(line[x]));
				m_gMax = qMax(m_gMax, qGreen(line[x]));
				m_bMax = qMax(m_bMax, qBlue(line[x]));
			}
			else if (m_mode == HSV) {
				qreal h, s, v;
				QColor c(line[x]);
				c.getHsvF(&h, &s, &v);
				m_sMin = qMin(m_sMin, s);
				m_sMax = qMax(m_sMax, s);
				m_vMin = qMin(m_vMin, v);
				m_vMax = qMax(m_vMax, v);
			}
		}	
	}	
	
	// create the rgb luts
	if (m_mode == RGB) {
		
		m_lutRed.clear();
		m_lutGreen.clear();
		m_lutBlue.clear();
		
		{
			int range = m_rMax - m_rMin;
			if (range == 0) {
				m_lutRed << m_rMin;
			}
			else {
				for (int i = m_rMin; i <= m_rMax; i++) {
					m_lutRed << 255 * (i - m_rMin) / range;
				}
			}
		}
		{
			int range = m_gMax - m_gMin;
			if (range == 0) {
				m_lutGreen << m_rMin;
			}
			else {			
				for (int i = m_gMin; i <= m_gMax; i++) {
					m_lutGreen << 255 * (i - m_gMin) / range;
				}
			}
		}
		{
			int range = m_bMax - m_bMin;
			if (range == 0) {
				m_lutBlue << m_rMin;
			}
			else {
				for (int i = m_bMin; i <= m_bMax; i++) {
					m_lutBlue << 255 * (i - m_bMin) / range;
				}
			}
		}
	}
}

QRgb AutoContrastFilter::applyPixel(QRgb pixel)
{
	QMutexLocker locker(&m_mutex);
	if (m_mode == RGB) {
		quint8 r, g, b;
		
		r = qRed(pixel);
		g = qGreen(pixel);
		b = qBlue(pixel);
		
		return qRgba(m_lutRed  [qMin(qRed(pixel), m_lutRed.count() - 1)],
			     m_lutGreen[qMin(qGreen(pixel), m_lutGreen.count() - 1)],
			     m_lutBlue [qMin(qBlue(pixel), m_lutBlue.count() - 1)],
			     qAlpha(pixel));
	}
	else if (m_mode == HSV) {
		QColor c(pixel);
		qreal h, s, v;
		c.getHsvF(&h, &s, &v);
		
		if (m_sMin != m_sMax) {
			s = (s - m_sMin) / (m_sMax - m_sMin);
		}
		if (m_vMin != m_vMax) {
			v = (v - m_vMin) / (m_vMax - m_vMin);
		}
		c.setHsvF(h, s, v);
		return c.rgba();
	}	
	return pixel;
}

QImage AutoContrastFilter::apply(const QImage &image, const QRect& clipRect)
{
	QImage resultImg = image.convertToFormat(QImage::Format_ARGB32);	
		
	QRect rc = clipRect;
	
	if (rc.isEmpty()) {
		rc = image.rect();
	}
	
	init(resultImg, rc);
	

	m_isAborting = false;
	
	for (int y = rc.y(); y <= rc.bottom(); ++y) {
		if (m_isAborting) {
			return QImage();
		}
		QRgb* line = reinterpret_cast<QRgb*>(resultImg.scanLine(y));
		
		for (int x = rc.x(); x <= rc.right(); ++x) {
			line[x] = applyPixel(line[x]);
		}
		setProgress(int((y - rc.y()) * 100.0 / rc.height()));
	}
	
	m_lutRed.clear();
	m_lutGreen.clear();
	m_lutBlue.clear();
	
	return resultImg;

}

QString AutoContrastFilter::name() const
{
	return QObject::tr("Autocontrast");
}

QString AutoContrastFilter::description() const
{
	return QObject::tr("Stretch contrast.");
}

QVariant AutoContrastFilter::option(int filteroption) const
{
	switch ((AutoContrastFilterOption)filteroption) {
	case Mode:
		return QVariant(m_mode);
		break;
	default:
		break;
	}
	return QVariant();
}

bool AutoContrastFilter::setOption(int filteroption, const QVariant &value)
{
	switch ((AutoContrastFilterOption)filteroption) {
	case Mode:
		m_mode = (AutoContrastMode)value.toInt();
		break;
	default:
		break;
	}
	return true;
}

bool AutoContrastFilter::supportsOption(int option) const
{
	bool supports = false;
	switch ((AutoContrastFilterOption)option) {
	case Mode:
		supports = true;
		break;
	default:
		break;
	}
	return supports;
}

QList<int> AutoContrastFilter::options() const {
	QList<int> ret;
	ret << AutoContrastFilter::Mode;
	return ret;
}

QWidget *AutoContrastFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
	l->setMargin(2);
	l->setSpacing(2);

	m_rgb = new QRadioButton(tr("RGB mode:"), w);
	m_rgb->setChecked(true);
	connect(m_rgb, SIGNAL(toggled(bool)), SLOT(handleControlValueChanged()));
	
	m_hsv = new QRadioButton(tr("HSV mode:"), w);
	m_hsv->setChecked(false);
	connect(m_hsv, SIGNAL(toggled(bool)), SLOT(handleControlValueChanged()));
	
	l->addWidget(m_rgb, 0, 0, 1, 1);
	l->addWidget(m_hsv, 1, 0, 1, 1);
	
	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}


void AutoContrastFilter::handleControlValueChanged() {
	
	if (m_rgb->isChecked()) {
		setOption(AutoContrastFilter::Mode, RGB);
	}
	else {
		setOption(AutoContrastFilter::Mode, HSV);
	}
	QtImageFilter::handleControlValueChanged();
}
