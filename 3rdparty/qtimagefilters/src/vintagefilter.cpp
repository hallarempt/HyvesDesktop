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

#include "vintagefilter.h"

#include <math.h>

#include <QtGui>
#include <QtCore>

#include "blendingmodes.h"
#include "colorconversions.h"
#include "noise.h"
#include "colorwell.h"

VintageFilter::VintageFilter() :
		QtImageFilter()
		, m_noiseSlider(0)
		, m_startColorWell(0)
		, m_endColorWell(0)
{
	resetOptions();
}

void VintageFilter::resetOptions() {
	
	m_noise = 10;
	m_startColor = Qt::red;
	m_endColor = Qt::green;
	updateLuts();
	
	if (m_noiseSlider) {
		m_noiseSlider->blockSignals(true);
		m_startColorWell->blockSignals(true);
		m_endColorWell->blockSignals(true);

		m_noiseSlider->setValue(m_noise);
		m_startColorWell->setColor(m_startColor);
		m_endColorWell->setColor(m_endColor);

		m_noiseSlider->blockSignals(false);
		m_startColorWell->blockSignals(false);
		m_endColorWell->blockSignals(false);

	}
	
	QtImageFilter::handleControlValueChanged();

}


VintageFilter::~VintageFilter()
{
}

QImage VintageFilter::apply(const QImage& image, const QRect& clipRect)
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

			QRgb pixel = line[x];
			
			// step one: apply gradient to pixel
			pixel = qRgb(m_lutRed[qRed(pixel)],
				     m_lutGreen[qGreen(pixel)],
				     m_lutBlue[qBlue(pixel)]);
			
			// step two: blend the gradient pixel with 50% opacity with the original pixel
			pixel = Blending::color(pixel, line[x], 128);
			
			
			// Add noise
			pixel = Noise::generateParametrizedGaussianNoise(pixel, m_noise);
			
			line[x] = pixel;
			

		}
		setProgress(int((y - rc.y()) * 100.0 / rc.height()));
	}

	return resultImg;
}


QVariant VintageFilter::option(int option) const {
	switch (VintageFilterOption(option)) {
	case Noise:
		return QVariant(m_noise);
	case StartColor:
		return QVariant(m_startColor);
	case EndColor:		
		return QVariant(m_endColor);
	}
	return QVariant();
}

bool VintageFilter::setOption(int option, const QVariant &value) {
	switch (VintageFilterOption(option)) {
	case Noise:
		m_noise = value.toInt();
		return true;
	case StartColor:
		m_startColor = value.value<QColor>();
		updateLuts();
		return true;
	case EndColor:
		m_endColor = value.value<QColor>();
		updateLuts();
		return true;
	}
	return false;
}

bool VintageFilter::supportsOption(int option) const {
	switch (VintageFilterOption(option)) {
	case Noise:
	case StartColor:
	case EndColor:
		return true;
	}
	return false;
}

QList<int> VintageFilter::options() const {
	QList<int> ret;
	ret << VintageFilter::Noise << VintageFilter::StartColor << VintageFilter::EndColor;
	return ret;
}

QString VintageFilter::name() const {
	
	return QObject::tr("Vintage");
}

QString VintageFilter::description() const {
	
	return QObject::tr("Make your picture look like it was taken in the seventies.");
}

void VintageFilter::updateLuts() {
	
	QLinearGradient gradient(QPointF(0, 0), QPointF(255, 255));
	gradient.setColorAt(0, m_startColor);
	gradient.setColorAt(1, m_endColor);
	gradient.setStart(0, 0);

	QImage gradientImage(256, 256, QImage::Format_ARGB32);
	QPainter gc(&gradientImage);
	gc.fillRect(gradientImage.rect(), gradient);
	gc.end();
	
	m_lutRed.clear();
	m_lutGreen.clear();
	m_lutBlue.clear();
	
	for (int i = 0; i <= 255; ++i) {
		QRgb pixel = gradientImage.pixel(i, i);
		m_lutRed << qRed(pixel);
		m_lutGreen << qGreen(pixel);
		m_lutBlue << qBlue(pixel);
	}
}

QWidget *VintageFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
        l->setMargin(2);
        l->setSpacing(2);
	
	
	m_noiseSlider = new QSlider(w);
	m_noiseSlider->setMinimum(1);
	m_noiseSlider->setOrientation(Qt::Horizontal);
	m_noiseSlider->setMaximum(100);
	m_noiseSlider->setValue(m_noise);
	connect(m_noiseSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));
	l->addWidget(new QLabel(QObject::tr("Noise:"), w), 0, 0, 1, 1);
	l->addWidget(m_noiseSlider, 0, 1, 1, 1);

	
	m_startColorWell = new ColorWell(w);
	m_startColorWell->setColor(m_startColor);
	connect(m_startColorWell, SIGNAL(colorChanged(QColor)), SLOT(handleControlValueChanged()));
	l->addWidget(new QLabel(QObject::tr("Start Color:"), w), 1, 0, 1, 1);
	l->addWidget(m_startColorWell, 1, 1, 1, 1);
	
	m_endColorWell = new ColorWell(w);
	m_endColorWell->setColor(m_endColor);
	connect(m_endColorWell, SIGNAL(colorChanged(QColor)), SLOT(handleControlValueChanged()));
	l->addWidget(new QLabel(QObject::tr("End Color:"), w), 2, 0, 1, 1);
	l->addWidget(m_endColorWell, 2, 1, 1, 1);

	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}

void VintageFilter::handleControlValueChanged() {
	
	setOption(VintageFilter::Noise, m_noiseSlider->value());
	setOption(VintageFilter::StartColor, m_startColorWell->selectedColor());
	setOption(VintageFilter::EndColor, m_endColorWell->selectedColor());
	QtImageFilter::handleControlValueChanged();
}
