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

#include "grittyfilter.h"
#include "highpassfilter.h"

#include <math.h>

#include <QtGui>
#include <QtCore>

#include "blendingmodes.h"
#include "colorconversions.h"
#include "noise.h"
#include "desaturatefilter.h"
#include "curvesfilter.h"
#include "colorwell.h"

GrittyFilter::GrittyFilter() 
	: QtImageFilter()
	, m_desaturationSlider(0)
	, m_contrastSlider(0)
	, m_darknessSlider(0)
{
	setThreadedPreviewEnabled(true);
	resetOptions();
}

void GrittyFilter::resetOptions()
{
	m_desaturation = 70;
	m_darkness = 46;
	m_contrast = 40;
	
	m_curve << 0     << 1059 << 2128 << 3207 << 4295 << 5392 << 6496 << 7607 << 8725 << 9849 << 10977 << 12110 
		<< 13248 << 14388 << 15531 << 16676 << 17822 << 18969 << 20115 << 21262 << 22406 << 23549 << 24690
		<< 25827 << 26960 << 28089 << 29212 << 30330 << 31442 << 32546 << 33642 << 34730 << 35809 << 36879 
		<< 37938 << 38986 << 40022 << 41047 << 42058 << 43056 << 44039 << 45008 << 45962 << 46899 << 47819 
		<< 48722 << 49607 << 50473 << 51320 << 52147 << 52953 << 53738 << 54487 << 55189 << 55844 << 56456 
		<< 57026 << 57557 << 58052 << 58511 << 58939 << 59336 << 59705 << 60049 << 60369 << 60668 << 60948 
		<< 61211 << 61460 << 61697 << 61924 << 62143 << 62356 << 62567 << 62776 << 62987 << 63201 << 63420 
		<< 63648 << 63886 << 64137 << 64402 << 64684 << 64986 << 65309;
	for (int i = m_curve.size(); i < 256; ++i) {
		m_curve << 65535;
	}	
	
	
	if (m_contrastSlider) {
		QList<QSlider*> sliders;
		sliders << m_contrastSlider << m_desaturationSlider << m_darknessSlider;
		foreach (QSlider *s, sliders) s->blockSignals(true);

		m_contrastSlider->setValue(m_contrast);
		m_desaturationSlider->setValue(m_desaturation);
		m_darknessSlider->setValue(m_darkness);

		foreach (QSlider *s, sliders) s->blockSignals(false);
	}
	QtImageFilter::handleControlValueChanged();
}

GrittyFilter::~GrittyFilter()
{
}

QImage GrittyFilter::doHighPass(QImage src, QRect rc) {
	
	// high pass a copy of the image, radius 5
	//qDebug() << "highpass start";
	QtImageFilter *highpass = setCurrentSubFilter("Highpass", 40);
	highpass->setOption(HighPassFilter::Radius, 5);
	QImage highpassImg = highpass->apply(src, rc);
	unsetCurrentSubFilter();
	//qDebug() << "highpass end";
	return highpassImg;
}

QImage GrittyFilter::apply(const QImage& image, const QRect& clipRect)
{
        QMutexLocker locker(&m_mutex);
	
	QImage resultImg = image.convertToFormat(QImage::Format_ARGB32);	
	QRect rc = clipRect;
	if (rc.isEmpty()) {
		rc = image.rect();
	}
	
	m_isAborting = false;
	
	setProgress(0);
	// create a highpass version of the original image
	QImage highpassImg = doHighPass(resultImg, rc);
	
	//qDebug() << "start desat";
	// create a desaturated version of the original image
	QtImageFilter *desaturate = setCurrentSubFilter("Desaturate", 10);
	desaturate->setOption(DesaturateFilter::Desaturation, m_desaturation);
	QImage desatImage = desaturate->apply(resultImg, rc);
	unsetCurrentSubFilter();
	if (m_isAborting) return QImage();
	
	//qDebug() << "overlay";
	QPainter gc(&resultImg);
	gc.setCompositionMode(QPainter::CompositionMode_Overlay);
	gc.drawImage(rc.topLeft(), highpassImg, rc);
	if (m_isAborting) return QImage();
	
	
	//qDebug() << "hardlight";
	gc.setCompositionMode(QPainter::CompositionMode_HardLight);
	gc.drawImage(rc.topLeft(), desatImage, rc);
	if (m_isAborting) return QImage();
	
	// Now, make another high-pass copy of the result
	highpassImg = doHighPass(resultImg, rc);
	gc.setCompositionMode(QPainter::CompositionMode_Overlay);
	gc.drawImage(rc.topLeft(), highpassImg, rc);
	gc.end();	
	if (m_isAborting) return QImage();
	
	//qDebug() << "blending";
	int desatImageMean = ColorConversions::meanPixelValue(resultImg, rc);
	qreal bright_f =   (-m_darkness + 100) / 100.0;
	qreal contrast_f = ( m_contrast * 5 + 100) / 100.0;
	qreal desatBrightImageMean = desatImageMean * bright_f;
	if (m_isAborting) return QImage();
	for (int y = 0; y < rc.height(); ++y) {
		if (m_isAborting) {
			return QImage();
		}
		QRgb* line = reinterpret_cast<QRgb*>(resultImg.scanLine(y));
		for (int x = 0; x < rc.width(); ++x) {
			QRgb p = line[x];
			// line[x] = Blending::over(qRgb(desatBrightImageMean, desatBrightImageMean, desatBrightImageMean), p, contrast_f);
			p = Blending::interp(qRgb(0, 0, 0), p, bright_f * 255);
			p = Blending::interp(qRgb(desatBrightImageMean, desatBrightImageMean, desatBrightImageMean), p, contrast_f * 255);
			line[x] = p;
		}	
		setProgress(int(y * 10.0 / image.height()) + 90);
	}
	//qDebug() << "done";
	if (m_isAborting) return QImage();
	return resultImg;
}


QVariant GrittyFilter::option(int option) const {
	switch (GrittyFilterOption(option)) {
	case Desaturation:
		return QVariant(m_desaturation);
	case Contrast:
		return QVariant(m_contrast);
	case Darkness:
		return QVariant(m_darkness);
	}
	return QVariant();
}

bool GrittyFilter::setOption(int option, const QVariant &value) {
	switch (GrittyFilterOption(option)) {
	case Desaturation:
		m_desaturation = value.toInt();
		return true;
	case Contrast:
		m_contrast = value.toInt();
		return true;
	case Darkness:
		m_darkness = value.toInt();
		return true;
	}
	return false;
}

bool GrittyFilter::supportsOption(int option) const {
	switch (GrittyFilterOption(option)) {
	case Desaturation:
	case Contrast:
	case Darkness:
		return true;
	}
	return false;
}

QList<int> GrittyFilter::options() const {
	QList<int> ret;
	ret << GrittyFilter::Desaturation << GrittyFilter::Contrast << GrittyFilter::Darkness;
	return ret;
}

QString GrittyFilter::name() const {
	
	return QObject::tr("Gritty effect");
}

QString GrittyFilter::description() const {
	
	return QObject::tr("Create a gritty look for your image.");
}

QWidget *GrittyFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
        l->setMargin(2);
        l->setSpacing(2);
	
	m_desaturationSlider = new QSlider(w);
	m_desaturationSlider->setValue(m_desaturation);
	m_desaturationSlider->setMinimum(0);
	m_desaturationSlider->setMaximum(100);
	m_desaturationSlider->setOrientation(Qt::Horizontal);
	m_desaturationSlider->setTracking(false);
	connect(m_desaturationSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));
	l->addWidget(new QLabel(QObject::tr("Desaturation:"), w), 0, 0, 1, 1);
	l->addWidget(m_desaturationSlider, 0, 1, 1, 1);
	
	m_contrastSlider = new QSlider(w);
	m_contrastSlider->setValue(m_contrast);
	m_contrastSlider->setMinimum(0);
	m_contrastSlider->setOrientation(Qt::Horizontal);
	m_contrastSlider->setMaximum(100);
	m_contrastSlider->setTracking(false);
	connect(m_contrastSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));
	l->addWidget(new QLabel(QObject::tr("Contrast:"), w), 1, 0, 1, 1);
	l->addWidget(m_contrastSlider, 1, 1, 1, 1);

	m_darknessSlider = new QSlider(w);
	m_darknessSlider->setValue(m_darkness);
	m_darknessSlider->setMinimum(0);
	m_darknessSlider->setOrientation(Qt::Horizontal);
	m_darknessSlider->setMaximum(100);
	m_darknessSlider->setTracking(false);
	connect(m_darknessSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));
	l->addWidget(new QLabel(QObject::tr("Darkness:"), w), 2, 0, 1, 1);
	l->addWidget(m_darknessSlider, 2, 1, 1, 1);

	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}

void GrittyFilter::handleControlValueChanged() {

	setOption(GrittyFilter::Desaturation, m_desaturationSlider->value());
	setOption(GrittyFilter::Contrast, m_contrastSlider->value());	
	setOption(GrittyFilter::Darkness, m_darknessSlider->value());
	QtImageFilter::handleControlValueChanged();
}
