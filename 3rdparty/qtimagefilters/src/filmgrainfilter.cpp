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

#include "filmgrainfilter.h"

#include <math.h>

#include <QtGui>
#include <QtCore>

#include "blendingmodes.h"
#include "colorconversions.h"
#include "noise.h"

FilmgrainFilter::FilmgrainFilter()
		: QtImageFilter()
		, m_radiusSlider(0)
{
	setThreadedPreviewEnabled(true);
	resetOptions();
}

void FilmgrainFilter::resetOptions()
{
	
	m_radius = 2;
	if (m_radiusSlider) {
		m_radiusSlider->blockSignals(true);
		m_radiusSlider->setValue(m_radius);
		m_radiusSlider->blockSignals(false);
	}
	QtImageFilter::handleControlValueChanged();
}

FilmgrainFilter::~FilmgrainFilter()
{
}

QImage FilmgrainFilter::apply(const QImage& image, const QRect& clipRect)
{
	QMutexLocker locker(&m_mutex);
	QImage resultImg = image.convertToFormat(QImage::Format_ARGB32);	

	QRect rc = clipRect;
	if (rc.isEmpty()) {
		rc = image.rect();
	}
	rc &= resultImg.rect();
	setProgress(0);
	m_isAborting = false;
	
	QImage grain = init(resultImg.size());
	

	for (int y = rc.y(); y <= rc.bottom(); ++y) {
		if (m_isAborting) {
			return QImage();
		}
		QRgb* line = reinterpret_cast<QRgb*>(resultImg.scanLine(y));
		QRgb* grainLine = reinterpret_cast<QRgb*>(grain.scanLine(y));
		
		for (int x = rc.x(); x <= rc.right(); ++x) {
			// blend with the blurred noise layer
			int gray = qGray(line[x]);
			line[x] = Blending::overlay(grainLine[x], qRgb(gray, gray, gray), Blending::MAX);
		}
		setProgress(40 + int((y - rc.y()) * 60.0 / rc.height()));
	}

	return resultImg;
}

QVariant FilmgrainFilter::option(int option) const {

	if (option == QtImageFilter::Radius) {
		return m_radius;
	}
	return QVariant();
}

bool FilmgrainFilter::setOption(int option, const QVariant &value) {

	switch (option) {
		
	case QtImageFilter::Radius:
		{
			m_radius = value.toDouble();
			return true;
		}
		break;
	default:
		return false;
	};
		
	return false;
}

bool FilmgrainFilter::supportsOption(int option) const {

	
	return (option == QtImageFilter::Radius);
}

QList<int> FilmgrainFilter::options() const {
	QList<int> ret;
	ret << QtImageFilter::Radius;
	return ret;
}


QString FilmgrainFilter::name() const {
	
	return QObject::tr("B/W Coarse Grain Photo");
}

QString FilmgrainFilter::description() const {
	
	return QObject::tr("Add grain to your image ");
}

QImage FilmgrainFilter::init(const QSize &size) {
	
	QImage grain = QImage(size, QImage::Format_RGB32);
	
	// fill with noise	
	for (int y = 0; y < size.height(); ++y) {
		if (m_isAborting) {
			return grain;
		}
		QRgb* line = reinterpret_cast<QRgb*>(grain.scanLine(y));
		for (int x = 0; x < size.width(); ++x) {
			QRgb pixel = Noise::generateNoise(qRgb(128, 128, 128), Noise::MultiplicativeGaussianNoise);
			int gray = qGray(pixel);
			line[x] = qRgb(gray, gray, gray);
		}
		setProgress(int(y * 10.0 / size.height()));
	}
	
	// Blur
	QtImageFilter* blur = setCurrentSubFilter("GaussianBlur", 30);
	blur->setOption(QtImageFilter::FilterBorderPolicy, "extend");	
	blur->setOption(QtImageFilter::Radius, m_radius);
	grain = blur->apply(grain, grain.rect());	
	unsetCurrentSubFilter();
	return grain;
	
	return grain;
}

QWidget *FilmgrainFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
        l->setMargin(2);
        l->setSpacing(2);

	m_radiusSlider = new QSlider(w);
	m_radiusSlider->setMinimum(1);
	m_radiusSlider->setMaximum(30);
	m_radiusSlider->setOrientation(Qt::Horizontal);
	m_radiusSlider->setValue(m_radius);
	m_radiusSlider->setTracking(false);
	connect(m_radiusSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));
	
	l->addWidget(new QLabel(QObject::tr("Smoothness:"), w), 0, 0, 1, 1);
	l->addWidget(m_radiusSlider, 0, 1, 1, 1);
	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}

void FilmgrainFilter::handleControlValueChanged() {
	
	setOption(FilmgrainFilter::Radius, m_radiusSlider->value());
	
	QtImageFilter::handleControlValueChanged();
}
