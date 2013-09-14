/*
 * Hyves Desktop, Copyright (C) 2009 Hyves (Startphone Ltd.)
 * http://www.hyves.nl/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "thresholdfilter.h"

#include <math.h>

#include <QtGui>
#include <QtCore>

#include "colorconversions.h"

ThresholdFilter::ThresholdFilter() 
		: QtImageFilter()
{
	resetOptions();
}

void ThresholdFilter::resetOptions()
{
	m_min = 0;
	m_max = 95;
	
	if (m_sliders.count() > 0) {
		m_sliders["Min"]->blockSignals(true);
		m_sliders["Max"]->blockSignals(true);

		m_sliders["Min"]->setValue(m_min);
		m_sliders["Max"]->setValue(m_max);

		m_sliders["Min"]->blockSignals(false);
		m_sliders["Max"]->blockSignals(false);
	}
	
	QtImageFilter::handleControlValueChanged();
}

ThresholdFilter::~ThresholdFilter()
{
}

QRgb ThresholdFilter::threshold(QRgb pixel, quint8 min, quint8 max, QRgb background, QRgb foreground)
{
	int y, u, v;
	ColorConversions::rgbToYuv(pixel, &y, &u, &v);
	
	if (y < min || y > max) {
		return background;
	}
	return foreground;
}

QImage ThresholdFilter::apply(const QImage& image, const QRect& clipRect)
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
			line[x] = threshold(line[x], m_min, m_max, qRgba(255, 255, 255, qAlpha(line[x])), qRgba(0, 0, 0, qAlpha(line[x])));
		}
		setProgress(int((y - rc.y()) * 100.0 / rc.height()));
	}
	return resultImg;
}


QVariant ThresholdFilter::option(int option) const {
	switch (ThresholdFilterOption(option)) {
	case Min:
		return QVariant(m_min);
	case Max:
		return QVariant(m_max);
	}
	return QVariant();
}

bool ThresholdFilter::setOption(int option, const QVariant &value) {
	switch (ThresholdFilterOption(option)) {
	case Min:
		{
			int i = value.toInt();
			i = qBound(0, i, 100);
			m_min = i;
			
			return true;
		}
	case Max:
		{
			int i = value.toInt();
			i = qBound(0, i, 100);
			m_max = i;
			return true;
		}
	}
	return false;
}

bool ThresholdFilter::supportsOption(int option) const {
	switch (ThresholdFilterOption(option)) {
	case Min:
	case Max:
		return true;
	}
	return false;
}

QList<int> ThresholdFilter::options() const {
	QList<int> ret;
	ret << ThresholdFilter::Min << ThresholdFilter::Max;
	return ret;
}


QString ThresholdFilter::name() const {
	
	return QObject::tr("Threshold");
}

QString ThresholdFilter::description() const {
	
	return QObject::tr("Remove all colors above or below the threshold and make the rest black.");
}


QWidget *ThresholdFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
        l->setMargin(2);
        l->setSpacing(2);

	typedef QPair<QString,QString> ID;
	
	QMap<ID, int> defaultValues;
	defaultValues[ID("Min", tr("Minimum value:"))] = m_min;
	defaultValues[ID("Max", tr("Maximum value:"))] = m_max;
	int row = 0;
	foreach (ID pair, defaultValues.keys()) {
		++row;
		QSlider *slider = new QSlider(w);
		slider->setMinimum(0);
		slider->setMaximum(99);
		slider->setOrientation(Qt::Horizontal);
		slider->setValue(defaultValues[pair]);
		l->addWidget(new QLabel(pair.second, w), row, 0, 1, 1);
		l->addWidget(slider, row, 1, 1, 2);
		connect(slider, SIGNAL(valueChanged(int)), this, SLOT(handleControlValueChanged()));
		m_sliders[pair.first] = slider;
	}
	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}

void ThresholdFilter::handleControlValueChanged() {
	

	int min = m_sliders["Min"]->value();
	int max = m_sliders["Max"]->value();
	if (min > max) {
		if (sender() == m_sliders["Min"]) {
			max = min;
		} else if (sender() == m_sliders["Max"]) {
			min = max;
		}
		m_sliders["Max"]->blockSignals(true);
		m_sliders["Min"]->blockSignals(true);
		m_sliders["Max"]->setValue(max);
		m_sliders["Min"]->setValue(min);
		m_sliders["Max"]->blockSignals(false);
		m_sliders["Min"]->blockSignals(false);
	}
	setOption(ThresholdFilter::Min, min);
	setOption(ThresholdFilter::Max, max);
	QtImageFilter::handleControlValueChanged();
}
