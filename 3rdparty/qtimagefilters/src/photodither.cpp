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

#include "photodither.h"

#include <math.h>

#include <QtGui>
#include <QtCore>

#include "blendingmodes.h"
#include "colorconversions.h"
#include "noise.h"
#include "desaturatefilter.h"
#include "curvesfilter.h"
#include "unsharpmaskfilter.h"
#include "colorwell.h"

PhotoDitherFilter::PhotoDitherFilter() : QtImageFilter(), m_desaturationSlider(0), m_contrastSlider(0)
{
	setThreadedPreviewEnabled(true);
	resetOptions();
}

void PhotoDitherFilter::resetOptions()
{
	m_desaturation = 50;
	m_contrast = 5;
	if (m_desaturationSlider) {
		m_desaturationSlider->blockSignals(true);
		m_desaturationSlider->setValue(m_desaturation);
		m_desaturationSlider->blockSignals(false);
	}
	if (m_contrastSlider) {
		m_contrastSlider->blockSignals(true);
		m_contrastSlider->setValue(m_contrast);
		m_contrastSlider->blockSignals(false);
	}
	
	QtImageFilter::handleControlValueChanged();
}

PhotoDitherFilter::~PhotoDitherFilter()
{
}

QImage PhotoDitherFilter::apply(const QImage& _image, const QRect& clipRect)
{
        QMutexLocker locker(&m_mutex);
	
	QImage image = _image;
	if (image.width() > 800 || image.height() > 800) {
		image = _image.scaled(QSize(800, 800), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}
	QImage resultImg = image.convertToFormat(QImage::Format_ARGB32);	
	QRect rc = clipRect;
	if (rc.isEmpty()) {
		rc = image.rect();
	}
	setProgress(0);
	m_isAborting = false;
	
	// Desat
	QtImageFilter *desaturate = setCurrentSubFilter("Desaturate", 10);
	desaturate->setOption(DesaturateFilter::Desaturation, m_desaturation);
	resultImg = desaturate->apply(resultImg, rc);
	unsetCurrentSubFilter();
	if (m_isAborting) return QImage();
	
	// create a sharpened version of the image
	QtImageFilter *unsharp = setCurrentSubFilter("Sharpen", 80);
	QImage sharp = unsharp->apply(resultImg, rc);
	unsetCurrentSubFilter();
	if (m_isAborting) return QImage();
	
	// dither the sharpened image
	QtImageFilter *dither = setCurrentSubFilter("Dither", 10);
	sharp = dither->apply(sharp);
	unsetCurrentSubFilter();
	if (m_isAborting) return QImage();
	
	QPainter gc(&resultImg);
	gc.setCompositionMode(QPainter::CompositionMode_Screen);
	gc.drawImage(rc.topLeft(), sharp, rc);
	if (m_isAborting) return QImage();
		
	return resultImg;
}


QVariant PhotoDitherFilter::option(int option) const {
	switch (PhotoDitherFilterOption(option)) {
	case Desaturation:
		return QVariant(m_desaturation);
	case Contrast:
		return QVariant(m_contrast);
	}
	return QVariant();
}

bool PhotoDitherFilter::setOption(int option, const QVariant &value) {
	switch (PhotoDitherFilterOption(option)) {
	case Desaturation:
		m_desaturation = value.toInt();
		return true;
	case Contrast:
		m_contrast = value.toInt();
		return true;
	}
	return false;
}

bool PhotoDitherFilter::supportsOption(int option) const {
	switch (PhotoDitherFilterOption(option)) {
	case Desaturation:
	case Contrast:
		return true;
	}
	return false;
}

QList<int> PhotoDitherFilter::options() const {
	QList<int> ret;
	ret << PhotoDitherFilter::Desaturation << PhotoDitherFilter::Contrast;
	return ret;
}

QString PhotoDitherFilter::name() const {
	
	return QObject::tr("Photo Dither");
}

QString PhotoDitherFilter::description() const {
	
	return QObject::tr("Transform your image into coloured dots.");
}

QWidget *PhotoDitherFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;

	QGridLayout *l = new QGridLayout;
	l->setAlignment(Qt::AlignLeft);
	l->setSpacing(1);
	l->setMargin(1);

	
	m_desaturationSlider = new QSlider(w);
	m_desaturationSlider->setValue(m_desaturation);
	m_desaturationSlider->setMinimum(0);
	m_desaturationSlider->setMaximum(100);
	m_desaturationSlider->setOrientation(Qt::Horizontal);
	m_desaturationSlider->setTracking(false);
	connect(m_desaturationSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));
	l->addWidget(new QLabel(QObject::tr("Desaturation:"), w), 0, 0, 1, 1);
	l->addWidget(m_desaturationSlider, 0, 1, 1, 1);
	
/*	
	m_contrastSlider = new QSlider(w);
	m_contrastSlider->setValue(m_contrast);
	m_contrastSlider->setMinimum(0);
	m_contrastSlider->setOrientation(Qt::Horizontal);
	m_contrastSlider->setMaximum(100);
	connect(m_contrastSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));
	l->addWidget(new QLabel(QObject::tr("contrast:"), w), 1, 0, 1, 1);
	l->addWidget(m_contrastSlider, 1, 1, 1, 1);
*/	
	w->setLayout(l);

	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	
	return w;
}

void PhotoDitherFilter::handleControlValueChanged() {

	setOption(PhotoDitherFilter::Desaturation, m_desaturationSlider->value());
//	setOption(PhotoDitherFilter::Contrast, m_contrastSlider->value());	
	QtImageFilter::handleControlValueChanged();
}
