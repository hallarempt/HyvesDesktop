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

#include "colorizefilter.h"

#include <QtGui>
#include <QtCore>

#include "colorwell.h"

ColorizeFilter::ColorizeFilter()
	: QtImageFilter()
	, m_colorWell(0)
{
	resetOptions();
}

void ColorizeFilter::resetOptions()
{
	m_color = Qt::red;
	if (m_colorWell) {
		m_colorWell->blockSignals(true);
		m_colorWell->setColor(m_color);
		m_colorWell->blockSignals(false);
	}
	QtImageFilter::handleControlValueChanged();
}

ColorizeFilter::~ColorizeFilter()
{
}

QImage ColorizeFilter::apply(const QImage& image, const QRect& clipRect)
{
        QMutexLocker locker(&m_mutex);

	int h, h2, s, s2, v, v2;
	option(ColorizeFilter::Color).value<QColor>().getHsv(&h, &s, &v);

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
			QColor pixel(line[x]);
			pixel.getHsv(&h2, &s2, &v2);
			pixel.setHsv(h, s2, v2);
			// qDebug() << "h " << h << ", h2 " << h2;
			resultImg.setPixel(x, y, pixel.rgb());
			
		}
		setProgress(int((y - rc.y()) * 100.0 / rc.height()));

	}
	
	return resultImg;
}

QString ColorizeFilter::name() const {
	
	return QObject::tr("Colorize");
}

QString ColorizeFilter::description() const {
	
	return QObject::tr("Make the image monochrome in the given color");
}

QVariant ColorizeFilter::option(int filteroption) const
{
	switch ((ColorizeFilterOption)filteroption) {
	case Color:
		if (m_color.isValid()) {
			return QVariant(m_color);
		}
		return QColor(Qt::red);
		break;
	default:
		break;
	}
	return QVariant();
}

bool ColorizeFilter::setOption(int filteroption, const QVariant &value)
{
	switch ((ColorizeFilterOption)filteroption) {
	case Color:
		m_color = value.value<QColor>();
		break;
	default:
		break;
	}
	return true;
}

bool ColorizeFilter::supportsOption(int option) const
{
	bool supports = false;
	switch ((ColorizeFilterOption)option) {
	case Color:
		supports = true;
		break;
	default:
		break;
	}
	return supports;
}

QList<int> ColorizeFilter::options() const {
	QList<int> ret;
	ret << ColorizeFilter::Color;
	return ret;
}

QWidget *ColorizeFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
        l->setMargin(2);
        l->setSpacing(2);

	m_colorWell = new ColorWell();	
	
	l->addWidget(new QLabel(QObject::tr("Color:"), w), 0, 0, 1, 1);
	l->addWidget(m_colorWell, 0, 1, 1, 1);
	
	connect(m_colorWell, SIGNAL(colorChanged(QColor)), this, SLOT(handleControlValueChanged()));
	
	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}

void ColorizeFilter::handleControlValueChanged() {
	
	setOption(ColorizeFilter::Color, m_colorWell->selectedColor());
	QtImageFilter::handleControlValueChanged();
}
