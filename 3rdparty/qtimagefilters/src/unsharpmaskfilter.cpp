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

#include "unsharpmaskfilter.h"

#include <QtGui>
#include <QtCore>

#include "colorconversions.h"
#include "noise.h"

UnsharpMaskFilter::UnsharpMaskFilter() 
		: QtImageFilter()
		, m_amountSpinBox(0)
		, m_thresholdSpinBox(0)
		, m_sigmaSpinBox(0)
		, m_radiusSpinBox(0)
{
	resetOptions();
}

void UnsharpMaskFilter::resetOptions() 
{
	
 	m_amount = 80;
	m_threshold = 1;
	m_sigma = -1; // if it's < 0, it will be autocalculated
	m_radius = 5;

	if (m_amountSpinBox) {
		QList<QDoubleSpinBox*> spinboxes;
		spinboxes << m_amountSpinBox << m_thresholdSpinBox << m_sigmaSpinBox << m_radiusSpinBox;
		foreach (QDoubleSpinBox* s, spinboxes) s->blockSignals(true);

		m_amountSpinBox->setValue(m_amount);
		m_thresholdSpinBox->setValue(m_threshold);
		m_sigmaSpinBox->setValue(m_sigma);
		m_radiusSpinBox->setValue(m_radius);

		foreach (QDoubleSpinBox* s, spinboxes) s->blockSignals(false);
	}

	QtImageFilter::handleControlValueChanged();
}

UnsharpMaskFilter::~UnsharpMaskFilter()
{
}

quint8 unsharp(const quint8 original, double sharpened, qreal amount, qreal threshold)
{
;
	quint8 channel = original;
	qreal value = original - sharpened;

	if (qAbs(value) >= threshold) {
		value = original + (value * amount);
		channel = roundDoubleToChar(value);
	}
	return channel;
}

void unsharpMaskPixels( qreal amount, qreal threshold, const QRgb* src, QRgb* dst, quint32 npixels)
{

	for ( quint32 i = 0; i < npixels; i++)
	{
		dst[i] = qRgba( unsharp(qRed(src[i]),   qRed(dst[i]), amount, threshold),
				unsharp(qGreen(src[i]), qGreen(dst[i]), amount, threshold),
				unsharp(qBlue(src[i]),  qBlue(dst[i]), amount, threshold),
				unsharp(qAlpha(src[i]), qAlpha(dst[i]), amount, threshold));
	}
}

QImage UnsharpMaskFilter::apply(const QImage& image, const QRect& clipRect)
{
        QMutexLocker locker(&m_mutex);

	QImage resultImg = image.convertToFormat(QImage::Format_ARGB32);	
	
	QRect rc = clipRect;
	if (rc.isEmpty()) {
		rc = image.rect();
	}
	
	// create a blurred copy 
	QtImageFilter* blur = setCurrentSubFilter("Blur", 50);
	blur->setOption(QtImageFilter::Radius, m_radius);
	blur->setOption(QtImageFilter::FilterChannels, "rgba");
	blur->setOption(QtImageFilter::FilterBorderPolicy, "wrap");
	blur->setOption(QtImageFilter::Deviation, m_sigma);
	
	QImage blurred = blur->apply(resultImg, rc);
	unsetCurrentSubFilter();
	
	qreal threshold = (MaxRGBFloat * m_threshold) / 2.0;
	
	m_isAborting = false;
	for (int y = rc.y(); y <= rc.bottom(); ++y) {
		if (m_isAborting) {
			return QImage();
		}
		QRgb* src  = reinterpret_cast<QRgb*>(resultImg.scanLine(y));
		QRgb* blur = reinterpret_cast<QRgb*>(blurred.scanLine(y));
		
		unsharpMaskPixels(m_amount, threshold, src, blur, rc.width());
		setProgress(int((y - rc.y()) * 50.0 / rc.height()) + 50);
	}
	return blurred;
}


QVariant UnsharpMaskFilter::option(int option) const {
	switch (UnsharpMaskFilterOption(option)) {
	case Amount:
		return QVariant(m_amount);
	case Radius:
		return QVariant(m_radius);
	case Threshold:
		return QVariant(m_threshold);
	case Sigma:
		return QVariant(m_sigma);
	}
	return QVariant();
}

bool UnsharpMaskFilter::setOption(int option, const QVariant &value) {
	switch (UnsharpMaskFilterOption(option)) {
	case Amount:
		m_amount = value.toDouble();
		return true;
	case Radius:
		m_radius = value.toDouble();
		if (m_radius < 1.0) m_radius = 1.0;
		return true;
	case Threshold:
		m_threshold = qBound(0, value.toInt(), 255);
		return true;
	case Sigma:
		m_sigma = qBound(-1, value.toInt(), 100);
		return true;
	}
	return false;
}

bool UnsharpMaskFilter::supportsOption(int option) const {
	switch (UnsharpMaskFilterOption(option)) {
	case Amount:
	case Radius:
	case Threshold:
	case Sigma:
		return true;
	}
	return false;
}

QList<int> UnsharpMaskFilter::options() const {
	QList<int> ret;
	ret << UnsharpMaskFilter::Amount << UnsharpMaskFilter::Radius << UnsharpMaskFilter::Threshold << UnsharpMaskFilter::Sigma;
	return ret;
}


QString UnsharpMaskFilter::name() const {
	
	return QObject::tr("Unsharp Mask");
}

QString UnsharpMaskFilter::description() const {
	
	return QObject::tr("Sharpen with custom radius");
}

QWidget *UnsharpMaskFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
        l->setMargin(2);
        l->setSpacing(2);

	m_amountSpinBox = new QDoubleSpinBox(w);
	m_amountSpinBox->setMinimum(1);
	m_amountSpinBox->setMaximum(100);
	m_amountSpinBox->setValue(m_amount);
	connect(m_amountSpinBox, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));
	
	m_thresholdSpinBox = new QDoubleSpinBox(w);
	m_thresholdSpinBox->setMinimum(0);
	m_thresholdSpinBox->setMaximum(255);
	m_thresholdSpinBox->setValue(m_threshold);
	connect(m_thresholdSpinBox, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));
	
	m_sigmaSpinBox = new QDoubleSpinBox(w);
	m_sigmaSpinBox->setMinimum(1);
	m_sigmaSpinBox->setMaximum(250);
	m_sigmaSpinBox->setValue(m_sigma);
	connect(m_sigmaSpinBox, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));
	
	m_radiusSpinBox = new QDoubleSpinBox(w);	
	m_radiusSpinBox->setMinimum(1);
	m_radiusSpinBox->setMaximum(250);
	m_radiusSpinBox->setValue(m_radius);
	connect(m_radiusSpinBox, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));
	
	l->addWidget(new QLabel(QObject::tr("Amount:"), w), 0, 0, 1, 1);
	l->addWidget(m_amountSpinBox, 0, 1, 1, 1);

	l->addWidget(new QLabel(QObject::tr("Threshold:"), w), 1, 0, 1, 1);
	l->addWidget(m_thresholdSpinBox, 1, 1, 1, 1);

	l->addWidget(new QLabel(QObject::tr("Gaussian Deviation:"), w), 2, 0, 1, 1);
	l->addWidget(m_sigmaSpinBox, 2, 1, 1, 1);
		
	l->addWidget(new QLabel(QObject::tr("Radius:"), w), 3, 0, 1, 1);
	l->addWidget(m_radiusSpinBox, 3, 1, 1, 1);
		
	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	
	return w;
}

void UnsharpMaskFilter::handleControlValueChanged() {
	
	setOption(UnsharpMaskFilter::Amount, m_amountSpinBox->value());
	setOption(UnsharpMaskFilter::Threshold, m_thresholdSpinBox->value());
	setOption(UnsharpMaskFilter::Radius, m_radiusSpinBox->value());
	
	QtImageFilter::handleControlValueChanged();
}
