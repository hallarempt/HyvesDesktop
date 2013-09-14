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

#include "cinemafilter.h"
#include "qtimagefilter.h"
#include "desaturatefilter.h"
#include "noise.h"
#include "colorconversions.h"
#include "blendingmodes.h"

#include <stdlib.h>
#include <QtCore>
#include <QtGui>

/*
 * Cinema filter: Changes the image to look as if it's captured from a movie
 * Loosely based on: http://ebin.wordpress.com/2007/03/21/how-to-turn-your-photo-into-movie-like-effect-using-photoshop/
 */

QVariant CinemaFilter::option(int option) const {
	switch (CinemaFilterOption(option)) {
		case Darkness:
			return QVariant(m_darkness);
		case Contrast:
			return QVariant(m_contrast);
		case Noise:
			return QVariant(m_noise);
		case Cinemascope:
			return QVariant(m_cinemascope);
		case FocusRegionSize:
			return QVariant(m_focusRegionSize);
		case Blur:
			return QVariant(m_blur);
		case Desaturation:
			return QVariant(m_desaturation);
		case SpotlightSize:
			return QVariant(m_spotlightSize);
	}
	return QVariant();
}

bool CinemaFilter::setOption(int option, const QVariant &value) {
	switch (CinemaFilterOption(option)) {
		case Darkness:
			m_darkness = value.toInt();
			return true;
		case Contrast:
			m_contrast = value.toInt();
			return true;
		case Noise:
			m_noise = value.toInt();
			return true;
		case FocusRegionSize:
			m_focusRegionSize = value.toInt();
			return true;
		case Cinemascope:
			m_cinemascope = value.toBool();
			return true;
		case Blur:
			m_blur = value.toInt();
			return true;
		case Desaturation:
			m_desaturation = value.toInt();
			return true;
		case SpotlightSize:
			m_spotlightSize = value.toInt();
			return true;
	}
	return false;
}

bool CinemaFilter::supportsOption(int option) const {
	switch (CinemaFilterOption(option)) {
		case Darkness:
		case Contrast:
		case Noise:
		case Cinemascope:
		case FocusRegionSize:
		case Blur:
		case Desaturation:
		case SpotlightSize:
			return true;
	}
	return false;
}

QList<int> CinemaFilter::options() const {
	QList<int> ret;
	ret << CinemaFilter::Darkness << CinemaFilter::Contrast << CinemaFilter::Noise << CinemaFilter::Cinemascope
		<< CinemaFilter::FocusRegionSize << CinemaFilter::Blur << CinemaFilter::Desaturation << CinemaFilter::SpotlightSize;
	return ret;
}

QImage CinemaFilter::apply(const QImage &image, const QRect& clipRect) {

        QMutexLocker locker(&m_mutex);

	Q_UNUSED(clipRect);
	QImage resultImg = QImage(image.size(), QImage::Format_ARGB32);
	setProgress(0);
	m_isAborting = false;
	bool useCachedImage = ((m_cachedImage == image) && 
						   (m_cachedDesaturation == m_desaturation) &&
						   (m_cachedBlur == m_blur));
	if (!useCachedImage || 1) { // dont use cache - caching make it crash on cancelling at gaussblur
		// desaturate
		m_desatFilter = setCurrentSubFilter("Desaturate", 10);
		m_desatFilter->setOption(DesaturateFilter::Desaturation, m_desaturation);
		m_desatImage = m_desatFilter->apply(image);
		unsetCurrentSubFilter();
		
		if (m_desatImage.isNull())
			return QImage();
		// find mean
		m_desatImageMean = ColorConversions::meanPixelValue(m_desatImage, m_desatImage.rect());
		// blur
		m_blurFilter = setCurrentSubFilter("GaussianBlur", 20);
		m_blurFilter->setOption(QtImageFilter::Radius, m_blur);
		m_blurFilter->setOption(QtImageFilter::FilterChannels, "rgba");
		m_blurFilter->setOption(QtImageFilter::FilterBorderPolicy, "wrap");
		if (m_blur) {
			m_desatBlurredImage = m_blurFilter->apply(m_desatImage, m_desatImage.rect());
		} else {
			m_desatBlurredImage = m_desatImage;
		}
		unsetCurrentSubFilter();
		
		if (m_desatBlurredImage.isNull())
			return QImage();
		m_cachedImage = image;
		m_cachedDesaturation = m_desaturation;
		m_cachedBlur = m_blur;
	}
	qreal bright_f =   (-m_darkness + 100) / 100.0;
	qreal contrast_f = ( m_contrast * 5 + 100) / 100.0;
	qreal desatBrightImageMean = m_desatImageMean * bright_f;
	QPoint focusCenter = image.rect().center();
	int focusRadius = image.width() * m_focusRegionSize / 100;
	int darkenEdgesRadius = image.width() * m_spotlightSize * 2 / 100;
	qreal aspectRatio = image.width() / (image.height() * 0.5);

	// apply
	for (int y = 0; y < image.height(); ++y) {
		if (m_isAborting) {
			return QImage();
		}
		QRgb* resultLine = reinterpret_cast<QRgb*>(resultImg.scanLine(y));
		QRgb* line = reinterpret_cast<QRgb*>(const_cast<QImage*>(&m_desatImage)->scanLine(y));
		// Using const cast because we wont change line, but will write to resultLine instead
		QRgb* blurredLine = reinterpret_cast<QRgb*>(m_desatBlurredImage.scanLine(y));
		for (int x = 0; x < image.width(); ++x) {
			// Use of blending for all operations is based on fundae at:
			// http://www.graficaobscura.com/interp/
			if (m_cinemascope) {
				if (y < image.height() * 0.20 || y > image.height() * (1 - 0.20)) {
					resultLine[x] = qRgb(0, 0, 0);
					continue;
				}
			}
			QRgb p = line[x];
			// brightness
			p = Blending::interp(qRgb(0, 0, 0), p, bright_f * 255);
			// contrast
			p = Blending::interp(qRgb(desatBrightImageMean, desatBrightImageMean, desatBrightImageMean), p, contrast_f * 255);
			// blur edges
			qreal radialgrad_f = (focusRadius * focusRadius -
									((x - focusCenter.x()) * (x - focusCenter.x()) +
									 (y - focusCenter.y()) * (y - focusCenter.y()) * aspectRatio)  ) /
								 (1.0 * focusRadius * focusRadius);
			qreal radialgrad_d = (darkenEdgesRadius * darkenEdgesRadius -
									((x - focusCenter.x()) * (x - focusCenter.x()) +
									 (y - focusCenter.y()) * (y - focusCenter.y()) * aspectRatio)  ) /
								 (1.0 * darkenEdgesRadius * darkenEdgesRadius);
			QRgb pb = blurredLine[x];
			pb = Blending::interp(qRgb(0, 0, 0), pb, bright_f * 255);
			pb = Blending::interp(qRgb(desatBrightImageMean, desatBrightImageMean, desatBrightImageMean), pb, contrast_f * 255);
			p = Blending::interp(pb, p, qMin(255.0, qMax(0.0, radialgrad_f * 255)));
			// darken edges
			p = Blending::interp(0, p, qMin(255.0, qMax(0.0, (radialgrad_d + bright_f * 0.5) * 255)));
			// inject noise
			p = Noise::generateParametrizedGaussianNoise(p, m_noise);
			resultLine[x] = p;
		}
		setProgress(int(y * 70.0 / image.height()) + 30);
	}
	return resultImg;
}

QWidget *CinemaFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
        l->setMargin(2);
        l->setSpacing(2);

	m_cinemascopeCheckBox = new QCheckBox(w);
	m_cinemascopeCheckBox->setChecked(m_cinemascope);
	l->addWidget(new QLabel(QObject::tr("Cinemascope:"), w), 0, 0, 1, 1);
	l->addWidget(m_cinemascopeCheckBox, 0, 1, 1, 1);
	connect(m_cinemascopeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(handleControlValueChanged()));

	typedef QPair<QString,QString> ID;
	
	QMap<ID, int> defaultValues;
	defaultValues[ID(tr("Desaturate:"),"Desaturate")] = m_desaturation;
	defaultValues[ID(tr("Blur:"),"Blur")] = m_blur;
	defaultValues[ID(tr("Darken:"), "Darken")] = m_darkness;
	defaultValues[ID(tr("Contrast:"),"Contrast")] = m_contrast;
	defaultValues[ID(tr("Focus:"), "Focus")] = m_focusRegionSize;
	defaultValues[ID(tr("Spotlight:"), "Spotlight")] = m_spotlightSize;
	defaultValues[ID(tr("Noise:"), "Noise")] = m_noise;
	int row = 0;
	foreach (ID pair, defaultValues.keys()) {
		++row;
		QSlider *slider = new QSlider(w);
		slider->setMinimum(0);
		slider->setMaximum(99);
		slider->setOrientation(Qt::Horizontal);
		slider->setValue(defaultValues[pair]);
		l->addWidget(new QLabel(pair.first, w), row, 0, 1, 1);
		l->addWidget(slider, row, 1, 1, 2);
		connect(slider, SIGNAL(valueChanged(int)), this, SLOT(handleControlValueChanged()));
		slider->setTracking(false);
		m_sliders[pair.second] = slider;
	}
	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}

void CinemaFilter::handleControlValueChanged() {
	setOption(CinemaFilter::Cinemascope, m_cinemascopeCheckBox->isChecked());
	setOption(CinemaFilter::Desaturation, m_sliders["Desaturate"]->value());
	setOption(CinemaFilter::Blur, m_sliders["Blur"]->value());
	setOption(CinemaFilter::Darkness, m_sliders["Darken"]->value());
	setOption(CinemaFilter::Contrast, m_sliders["Contrast"]->value());
	setOption(CinemaFilter::FocusRegionSize, m_sliders["Focus"]->value());
	setOption(CinemaFilter::SpotlightSize, m_sliders["Spotlight"]->value());
	setOption(CinemaFilter::Noise, m_sliders["Noise"]->value());
	QtImageFilter::handleControlValueChanged();
}
