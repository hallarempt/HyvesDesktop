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

#include "oilpaintfilter.h"
#include "blendingmodes.h"

#include <QtGui>
#include <QtCore>

#include "colorconversions.h"

OilpaintFilter::OilpaintFilter() 
		: QtImageFilter()
		, m_brushSizeSlider(0)
		, m_smoothSlider(0)
{
	setThreadedPreviewEnabled(true);
	resetOptions();
}

void OilpaintFilter::resetOptions()
{
	
	m_brushSize = 5;
	m_smooth = 20;
	
	if (m_brushSizeSlider) {
		m_brushSizeSlider->blockSignals(true);
		m_smoothSlider->blockSignals(true);

		m_brushSizeSlider->setValue(m_brushSize);
		m_smoothSlider->setValue(m_smooth);

		m_brushSizeSlider->blockSignals(false);
		m_smoothSlider->blockSignals(false);
	}
	
	QtImageFilter::handleControlValueChanged();
}

void OilpaintFilter::setThumbnailCreationOptions() {
	m_brushSize = 30;
	m_smooth = 20;
}

OilpaintFilter::~OilpaintFilter()
{
}


QRgb OilpaintFilter::mostFrequentColor(const QImage& src, const QRect& bounds, const QPoint center, int brushSize, int smooth) {
	
	Q_ASSERT(smooth > 0);
	qreal scale = qreal(smooth) / 255;
	m_intensityCount.resize(smooth + 1);
	m_averageRed.resize(smooth + 1);
	m_averageGreen.resize(smooth + 1);
	m_averageBlue.resize(smooth + 1);

	// determine the area we are scanning: actually, shouldn't
	// we use a circle around the center, instead of a square?
	int startX = qMax(center.x() - brushSize, bounds.left());
	int startY = qMax(center.y() - brushSize, bounds.top());
	int windowSize = (2 * brushSize + 1);
	
	bool useCached = 	(m_cachedImage == src) &&
						(m_cachedSmooth == smooth) &&
						(m_cachedWindowSize == windowSize) &&
						(m_cachedStartY == startY) &&
						(startX - m_cachedStartX == 1); // caching assumes that we trace the image as for (y) { for (x) {}}
	if (useCached) {
		// we can arrive at the average channel data using the previous pixel's data
		for (int y = startY; y < startY + windowSize && y < src.height(); ++y) {
			const QRgb* line = reinterpret_cast<const QRgb*>(src.scanLine(y));
			quint8 intensityOut = ColorConversions::intensity(line[m_cachedStartX]) * scale;	
			m_intensityCount[intensityOut]--;
			m_averageRed[intensityOut]   -= qRed(line[m_cachedStartX]);
			m_averageGreen[intensityOut] -= qGreen(line[m_cachedStartX]);
			m_averageBlue[intensityOut]  -= qBlue(line[m_cachedStartX]);
			int xIn = startX + windowSize - 1;
			if (xIn < src.width()) {
				quint8 intensityIn = ColorConversions::intensity(line[xIn]) * scale;
				m_intensityCount[intensityIn]++;
				m_averageRed[intensityIn]   += qRed(line[xIn]);
				m_averageGreen[intensityIn] += qGreen(line[xIn]);
				m_averageBlue[intensityIn]  += qBlue(line[xIn]);
			}
		}
	}
	else {
		// not using cache, let's compute things from scratch
		for (int i = 0; i <= smooth; ++i) {
			m_intensityCount[i] = 0;
			m_averageRed[i] = 0;
			m_averageGreen[i] = 0;
			m_averageBlue[i] = 0;
		}
		for (int y = startY; y < startY + windowSize && y < src.height(); ++y) {
			const QRgb* line = reinterpret_cast<const QRgb*>(src.scanLine(y));
			for (int x = startX; x < startX + windowSize && x < src.width(); ++x) {
				quint8 intensity = ColorConversions::intensity(line[x]) * scale;	
				m_intensityCount[intensity]++;
				m_averageRed[intensity]   += qRed(line[x]);
				m_averageGreen[intensity] += qGreen(line[x]);
				m_averageBlue[intensity]  += qBlue(line[x]);
			}
		}
		m_cachedImage = src;
		m_cachedSmooth = smooth;
		m_cachedStartX = startX;
		m_cachedWindowSize = windowSize;
		m_cachedStartY = startY;
	}
	
	// Now figure out which intensity occurs most often
	quint32 mostFrequentIntensity = 0;
	quint32 maxFrequentIntensity = 0;
	
	for (int i = 0; i <= smooth; ++i) {
		if (m_intensityCount[i] > maxFrequentIntensity) {
			maxFrequentIntensity = m_intensityCount[i];
			mostFrequentIntensity = i;
		}
	}
	
	Q_ASSERT(maxFrequentIntensity > 0);
	if (maxFrequentIntensity <= 0) {
		maxFrequentIntensity = 1;
	}
	return qRgb(m_averageRed[mostFrequentIntensity]   / maxFrequentIntensity,
		    m_averageGreen[mostFrequentIntensity] / maxFrequentIntensity,
		    m_averageBlue[mostFrequentIntensity]  / maxFrequentIntensity);
}



QImage OilpaintFilter::apply(const QImage& _image, const QRect& clipRect)
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
	rc &= resultImg.rect();
	qreal brushSize = qreal(m_brushSize) * image.width() / 800;
	m_isAborting = false;
	qreal floatBrush = qMax(0.0, qMin(1.0, (1 - (ceil(brushSize) - brushSize))));
	for (int y = rc.y(); y <= rc.bottom(); ++y) {
		if (m_isAborting) {
			return QImage();
		}
		QRgb* line = reinterpret_cast<QRgb*>(resultImg.scanLine(y));
		
		for (int x = rc.x(); x <= rc.right(); ++x) {
			QRgb pixel = mostFrequentColor(image, rc, QPoint(x,y), ceil(brushSize), m_smooth);
			if (brushSize < 1.0) {
				line[x] = Blending::interp(line[x], pixel, 255 * floatBrush);
			} else {
				line[x] = pixel;
			}
		}
		setProgress(int((y - rc.y()) * 100.0 / rc.height()));
	}
	return resultImg;
}


QVariant OilpaintFilter::option(int option) const {
	switch (OilpaintFilterOption(option)) {
	case BrushSize:
		return QVariant(m_brushSize);
	case Smooth:
		return QVariant(m_smooth);
	}
	return QVariant();
}

bool OilpaintFilter::setOption(int option, const QVariant &value) {
	switch (OilpaintFilterOption(option)) {
	case BrushSize:
		m_brushSize = value.toInt();
		return true;
	case Smooth:
		m_smooth = value.toInt();
		return true;
	}
	return false;
}

bool OilpaintFilter::supportsOption(int option) const {
	switch (OilpaintFilterOption(option)) {
	case BrushSize:
	case Smooth:
		return true;
	}
	return false;
}

QList<int> OilpaintFilter::options() const {
	QList<int> ret;
	ret << OilpaintFilter::BrushSize << OilpaintFilter::Smooth;
	return ret;
}

QString OilpaintFilter::name() const {
	
	return QObject::tr("Oilpaint");
}

QString OilpaintFilter::description() const {
	
	return QObject::tr("Turn your picture into an impressionist oilpainting");
}

QWidget *OilpaintFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
        l->setMargin(2);
        l->setSpacing(2);


	m_brushSizeSlider = new QSlider(w);
	m_brushSizeSlider->setMinimum(1);
	m_brushSizeSlider->setOrientation(Qt::Horizontal);
	m_brushSizeSlider->setMaximum(30);
	m_brushSizeSlider->setValue(m_brushSize);
	m_brushSizeSlider->setTracking(false);
	connect(m_brushSizeSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));
	
	l->addWidget(new QLabel(QObject::tr("Brush Size:"), w), 0, 0, 1, 1);
	l->addWidget(m_brushSizeSlider, 0, 1, 1, 1);

	m_smoothSlider = new QSlider(w);
	m_smoothSlider->setOrientation(Qt::Horizontal);
	m_smoothSlider->setMinimum(1);
	m_smoothSlider->setMaximum(30);
	m_smoothSlider->setValue(m_smooth);
	m_smoothSlider->setTracking(false);
	connect(m_smoothSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));
	
	l->addWidget(new QLabel(QObject::tr("Smoothness:"), w), 1, 0, 1, 1);
	l->addWidget(m_smoothSlider, 1, 1, 1, 1);

	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}

void OilpaintFilter::handleControlValueChanged() {
	
	setOption(OilpaintFilter::BrushSize, m_brushSizeSlider->value());

	QtImageFilter::handleControlValueChanged();
}
