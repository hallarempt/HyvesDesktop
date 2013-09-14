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

#include "frostedglassfilter.h"

#include <math.h>

#include <QtGui>
#include <QtCore>

#include "colorconversions.h"
#include "blendingmodes.h"

FrostedGlassFilter::FrostedGlassFilter() : 
		QtImageFilter(),
		m_brushSizeSlider(0)
{
	setThreadedPreviewEnabled(true);
	resetOptions();
}

void FrostedGlassFilter::resetOptions()
{
	
	m_brushSize = 5;
	if (m_brushSizeSlider) {
		m_brushSizeSlider->blockSignals(true);
		m_brushSizeSlider->setValue(m_brushSize);
		m_brushSizeSlider->blockSignals(true);
	}
	QtImageFilter::handleControlValueChanged();
}

void FrostedGlassFilter::setThumbnailCreationOptions() {
	m_brushSize = 25;
}

FrostedGlassFilter::~FrostedGlassFilter()
{
}

/**
 * Return a pseudo-random pixel in the surroundings of center
 *
 * @param src the source QImage
 * @param bounds the area in which we are working
 * @param center the center of the brush
 * @param brushSize the radius around the center
 */
QRgb randomColor(const QImage& src, const QRect& bounds, const QPoint center, int brushSize) {


	quint32 intensityCount = 0;
	quint32 averageRed = 0;
	quint32 averageGreen = 0;
	quint32 averageBlue = 0;
	
	// determine the area we are scanning: actually, shouldn't
	// we use a circle around the center, instead of a square?
	int startX = qMax(qMax(center.x() - brushSize, bounds.left()), src.rect().left());
	int startY = qMax(qMax(center.y() - brushSize, bounds.top()), src.rect().top());
	int endX = qMin(qMin(center.x() + brushSize, bounds.right()), src.rect().right());
	int endY = qMin(qMin(center.y() + brushSize, bounds.bottom()), src.rect().bottom());
	
	// picking intensity of a random pixel from the whole pool will automatically be weighted-random
	int numPixels = (endX - startX + 1) * (endY - startY + 1);
	int picked = qrand() % numPixels;
	int pickedX = startX + (picked % (endX - startX + 1));
	int pickedY = startY + (picked / (endX - startX + 1));
	const QRgb* line = reinterpret_cast<const QRgb*>(src.scanLine(pickedY));
	quint8 pickedIntensity = ColorConversions::intensity(line[pickedX]);

	for (int y = startY; y <= endY; ++y) {
	
		const QRgb* line = reinterpret_cast<const QRgb*>(src.scanLine(y));

		for (int x = startX; x <= endX; ++x) {

			QRgb pixel = line[x];
			if (ColorConversions::intensity(line[x]) == pickedIntensity) {
				intensityCount++;
				averageRed += qRed(pixel);
				averageGreen += qGreen(pixel);
				averageBlue += qBlue(pixel);
			}
		}
		
	}
#if 0
	// weighted random choice for the mathematically challenged.
	QVector<int> v;
	for (int i = 0; i < 256; ++i) {
		int count = intensityCount[i];
		if (count > 0) {
			for (int j = 0; j < count; ++j) {
				v << i;
			}
		}
	}
	quint8 intensity = v[qrand() % v.size()];
#endif
	
	Q_ASSERT(intensityCount > 0);
	if (intensityCount == 0) {
		intensityCount = 1;
	}
	return qRgb(averageRed / intensityCount,
		    averageGreen / intensityCount,
		    averageBlue / intensityCount);
}


QImage FrostedGlassFilter::apply(const QImage& _image, const QRect& clipRect)
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
	
	m_isAborting = false;
	qreal brushSize = qreal(m_brushSize) * image.width() / 800;
	qreal floatBrush = qMax(0.0, qMin(1.0, (1 - (ceil(brushSize) - brushSize))));
	for (int y = rc.y(); y <= rc.bottom(); ++y) {
		if (m_isAborting) {
			return QImage();
		}
		QRgb* line = reinterpret_cast<QRgb*>(resultImg.scanLine(y));
		
		for (int x = rc.x(); x <= rc.right(); ++x) {
			QRgb pixel = randomColor(image, rc, QPoint(x,y), ceil(brushSize));
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


QVariant FrostedGlassFilter::option(int option) const {
	switch (FrostedGlassFilterOption(option)) {
	case BrushSize:
		return QVariant(m_brushSize);
	}
	return QVariant();
}

bool FrostedGlassFilter::setOption(int option, const QVariant &value) {
	switch (FrostedGlassFilterOption(option)) {
	case BrushSize:
		m_brushSize = value.toInt();
		return true;
	}
	return false;
}

bool FrostedGlassFilter::supportsOption(int option) const {
	switch (FrostedGlassFilterOption(option)) {
	case BrushSize:
		return true;
	}
	return false;
}

QList<int> FrostedGlassFilter::options() const {
	QList<int> ret;
	ret << FrostedGlassFilter::BrushSize;
	return ret;
}

QString FrostedGlassFilter::name() const {
	
	return QObject::tr("Frosted Glass");
}

QString FrostedGlassFilter::description() const {
	
	return QObject::tr("See your picture through a frozen windowpane.");
}

QWidget *FrostedGlassFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
        l->setMargin(2);
        l->setSpacing(2);

	m_brushSizeSlider = new QSlider(w);
	m_brushSizeSlider->setOrientation(Qt::Horizontal);
	m_brushSizeSlider->setMinimum(1);
	m_brushSizeSlider->setMaximum(30);
	m_brushSizeSlider->setValue(m_brushSize);
	m_brushSizeSlider->setTracking(false);
	connect(m_brushSizeSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));
	
	l->addWidget(new QLabel(QObject::tr("Brush Size:"), w), 0, 0, 1, 1);
	l->addWidget(m_brushSizeSlider, 0, 1, 1, 1);
	w->setLayout(l);

	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}

void FrostedGlassFilter::handleControlValueChanged() {
	
	setOption(FrostedGlassFilter::BrushSize, m_brushSizeSlider->value());
	QtImageFilter::handleControlValueChanged();
}
