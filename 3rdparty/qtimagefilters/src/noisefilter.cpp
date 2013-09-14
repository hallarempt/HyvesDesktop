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

#include "noisefilter.h"

#include <math.h>

#include <QtGui>
#include <QtCore>

#include "blendingmodes.h"
#include "colorconversions.h"
#include "noise.h"

NoiseFilter::NoiseFilter() 
		: QtImageFilter()
		, m_levelSlider(0)
		, m_opacitySlider(0)
{
	resetOptions();
}

void NoiseFilter::resetOptions()
{
	
	m_noiseType = Noise::GaussianNoise;
	m_level = 50;
	m_opacity = 100;
	
	if (m_levelSlider) {
		m_levelSlider->blockSignals(true);
		m_opacitySlider->blockSignals(true);

		m_levelSlider->setValue(m_level);
		m_opacitySlider->setValue(m_opacity);

		m_levelSlider->blockSignals(false);
		m_opacitySlider->blockSignals(false);
	}
	QtImageFilter::handleControlValueChanged();
}

NoiseFilter::~NoiseFilter()
{
}

QImage NoiseFilter::apply(const QImage& image, const QRect& clipRect)
{
        QMutexLocker locker(&m_mutex);

	QImage resultImg = image.convertToFormat(QImage::Format_ARGB32);	
	QRect rc = clipRect;
	if (rc.isEmpty()) {
		rc = image.rect();
	}
	m_isAborting = false;

	double threshold = (100.0 - m_level) * 0.01;

	QVector<QRgb> c;
	QVector<quint16> weights;
	weights << (255 * m_opacity) / 100 << 255 - ((255 * m_opacity) / 100);
	
	for (int y = rc.y(); y <= rc.bottom(); ++y) {
		if (m_isAborting) {
			return QImage();
		}
		QRgb* line = reinterpret_cast<QRgb*>(resultImg.scanLine(y));
		for (int x = rc.x(); x <= rc.right(); ++x) {

			QRgb pixel = line[x];
			if ( Noise::doubleRandReentrant() > threshold) {
				//QRgb noisyPixel = Noise::generateNoise(pixel, Noise::GaussianNoise);
				QRgb noisyPixel = qRgb((double)Noise::doubleRandReentrant()/RAND_MAX * 255,
						       (double)Noise::doubleRandReentrant()/RAND_MAX * 255,
						       (double)Noise::doubleRandReentrant()/RAND_MAX * 255);
				
				c.clear();
				c  << noisyPixel << pixel;
				line[x] = ColorConversions::mixColors(c, weights);
				//line[x] = Blending::over(pixel, noisyPixel, m_opacity);
				
			}
		}
		setProgress(int((y - rc.y()) * 100.0 / rc.height()));
	}

	return resultImg;
}


QVariant NoiseFilter::option(int option) const {
	switch (NoiseFilterOption(option)) {
	case NoiseType:
		return QVariant((int)m_noiseType);
	case Level:
		return QVariant(m_level);
	case Opacity:		
		return QVariant(m_opacity);
	}
	return QVariant();
}

bool NoiseFilter::setOption(int option, const QVariant &value) {
	switch (NoiseFilterOption(option)) {
	case NoiseType:
		m_noiseType = (Noise::NoiseType)value.toInt();
		return true;
	case Level:
		m_level = qBound(0, value.toInt(), 100);
		return true;
	case Opacity:
		m_opacity = qBound(0, value.toInt(), 100);
		return true;
	}
	return false;
}

bool NoiseFilter::supportsOption(int option) const {
	switch (NoiseFilterOption(option)) {
	case NoiseType:
	case Level:
	case Opacity:
		return true;
	}
	return false;
}

QList<int> NoiseFilter::options() const {
	QList<int> ret;
	ret << NoiseFilter::NoiseType << NoiseFilter::Level << NoiseFilter::Opacity;
	return ret;
}

QString NoiseFilter::name() const {
	
	return QObject::tr("Add Noise");
}

QString NoiseFilter::description() const {
	
	return QObject::tr("Add noise to the image.");
}

QWidget *NoiseFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
        l->setMargin(2);
        l->setSpacing(2);

	
	m_levelSlider = new QSlider(w);
	m_levelSlider->setValue(m_level);
	m_levelSlider->setMinimum(0);
	m_levelSlider->setMaximum(255);
	m_levelSlider->setOrientation(Qt::Horizontal);
	connect(m_levelSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));

	l->addWidget(new QLabel(QObject::tr("Level:"), w), 0, 0, 1, 1);
	l->addWidget(m_levelSlider, 0, 1, 1, 1);

	m_opacitySlider = new QSlider(w);
	m_opacitySlider->setValue(m_opacity);
	m_opacitySlider->setMinimum(0);
	m_opacitySlider->setMaximum(255);
	m_opacitySlider->setOrientation(Qt::Horizontal);
	connect(m_opacitySlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));

	l->addWidget(new QLabel(QObject::tr("Opacity:"), w), 1, 0, 1, 1);
	l->addWidget(m_opacitySlider, 1, 1, 1, 1);

	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}

void NoiseFilter::handleControlValueChanged() {
	
	setOption(NoiseFilter::Opacity, m_opacitySlider->value());
	setOption(NoiseFilter::Level, m_levelSlider->value());
	QtImageFilter::handleControlValueChanged();
}
