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

#include "ditherfilter.h"

#include <QtGui>
#include <QtCore>

#include "colorconversions.h"
#include "noise.h"

DitherFilter::DitherFilter() 
		: QtImageFilter()
		, m_atkinson(0)
		, m_floydSteinberg(0)
		, m_random(0)
{
	resetOptions();
}

void DitherFilter::resetOptions()
{
	m_mode = DitherFilter::Atkinson;
	if (m_atkinson) {
		m_atkinson->blockSignals(true);
		m_atkinson->setChecked(true);
		m_atkinson->blockSignals(false);
	}
	QtImageFilter::handleControlValueChanged();
}

DitherFilter::~DitherFilter()
{
}

bool DitherFilter::ditherAtkinson(QImage& dst, const QRect& rc)
{
	
	int oldGray;
	int newGray;
	int errGray;
	int surroundingGray;
	
	QVector<QPoint> points;
	points << QPoint(1, 0) << QPoint(2, 0) << QPoint(-1, 1) << QPoint(0, 1) << QPoint(1, 1) << QPoint(0, 2);
	
	m_isAborting = false;
	for (int y = rc.y(); y <= rc.bottom(); ++y) {
		if (m_isAborting) {
			return false;
		}
		QRgb* line = reinterpret_cast<QRgb*>(dst.scanLine(y));
		
		for (int x = rc.x(); x <= rc.right(); ++x) {
			oldGray = qGray(line[x]);
			if (oldGray > 128) {
				newGray = 255;
			}
			else {
				newGray = 0;
			}
			errGray = (oldGray - newGray) >> 3;
			line[x] = qRgb(newGray, newGray, newGray);
			
			// distribute the error forwards
			foreach(QPoint pt, points) {
				if (rc.contains(x + pt.x(), y + pt.y())) {
					surroundingGray = qGray(dst.pixel(x + pt.x(), y + pt.y())) + errGray;
					dst.setPixel(x + pt.x(), y + pt.y(), qRgb(surroundingGray, surroundingGray, surroundingGray));
				}
			}
		}
		setProgress(int((y - rc.y()) * 100.0 / rc.height()));
	}
	
	return true;
	
}

bool DitherFilter::ditherFloydSteinberg(QImage& src, const QRect& rc)
{
	int oldGray;
	int newGray;
	int errGray;
	int surroundingGray;
	
	QVector<QPoint> points;
	points << QPoint(-1, 1) << QPoint(0, 1) << QPoint(1, 0) << QPoint(1, 1);
	
	QVector<int> weights;
	weights << 3 << 5 << 7 << 1;
	
	m_isAborting = false;
	for (int y = rc.y(); y <= rc.bottom(); ++y) {
		if (m_isAborting) {
			return false;
		}
		QRgb* line = reinterpret_cast<QRgb*>(src.scanLine(y));
		
		for (int x = rc.x(); x <= rc.right(); ++x) {
			oldGray = qGray(line[x]);

			if (oldGray > 128) {
				newGray = 255;
			}
			else {
				newGray = 0;
			}

			errGray = (oldGray - newGray);
			line[x] = qRgb(newGray, newGray, newGray);
			
			// distribute the error forwards
			double error = errGray / 16;
			
			for (int i = 0; i < points.size() && i < weights.size(); ++i) {
				QPoint pt = points.at(i);
				int weight = weights.at(i);
				
				if (rc.contains(x + pt.x(), y + pt.y())) {
					surroundingGray = qGray(src.pixel(x + pt.x(), y + pt.y())) + (weight * error); 
					src.setPixel(x + pt.x(), y + pt.y(), qRgb(surroundingGray, surroundingGray, surroundingGray));
				}
			}
		}
		setProgress(int((y - rc.y()) * 100.0 / rc.height()));
	}
	
	return true;
	
}


bool DitherFilter::ditherRandom(QImage& src, const QRect& rc)
{
		
	m_isAborting = false;
	for (int y = rc.y(); y <= rc.bottom(); ++y) {
		if (m_isAborting) {
			return false;
		}
		QRgb* line = reinterpret_cast<QRgb*>(src.scanLine(y));
		
		for (int x = rc.x(); x <= rc.right(); ++x) {
			int gray = qGray(line[x]);
			qreal threshold = Noise::doubleRandReentrant();
			if (gray > (threshold * 255)) {
				line[x] = qRgb(255, 255, 255);
			}
			else {
				line[x] = qRgb(0, 0, 0);
			}
		}
		setProgress(int((y - rc.y()) * 100.0 / rc.height()));
	}
	return true;
}



QImage DitherFilter::apply(const QImage& _image, const QRect& clipRect)
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
	
	m_isAborting = false;
	
	switch(m_mode) {
	case Atkinson:
		if(ditherAtkinson(resultImg, rc)) {
			return resultImg;
		}
		else {
			return QImage();
		}
		break;
	case FloydSteinberg:
		if(ditherFloydSteinberg(resultImg, rc)) {
			return resultImg;
		}
		else {
			return QImage();
		}
		break;
	case Random:
		if(ditherRandom(resultImg, rc)) {
			return resultImg;
		}
		else {
			return QImage();
		}
		break;
	default:
		return QImage();
	}
	
	return QImage();
	
}


QVariant DitherFilter::option(int option) const {
	switch (DitherFilterOption(option)) {
	case Mode:
		return QVariant(m_mode);
	}
	return QVariant();
}

bool DitherFilter::setOption(int option, const QVariant &value) {
	switch (DitherFilterOption(option)) {
	case Mode:
		m_mode = DitherFilterMode(value.toInt());
		return true;
	}
	return false;
}

bool DitherFilter::supportsOption(int option) const {
	switch (DitherFilterOption(option)) {
	case Mode:
		return true;
	}
	return false;
}

QList<int> DitherFilter::options() const {
	QList<int> ret;
	ret << DitherFilter::Mode;
	return ret;
}

QString DitherFilter::name() const {
	
	return QObject::tr("Dither");
}

QString DitherFilter::description() const {
	
	return QObject::tr("Dither your image (make every pixel black or white)");
}

QWidget *DitherFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
        l->setMargin(2);
        l->setSpacing(2);
	
	
	QGroupBox* box = new QGroupBox(tr("Dithering Mode:"), w);
	box->setFlat(false);
	
	l->addWidget(box);

	m_buttonGroup = new QButtonGroup(w);
	m_buttonGroup->setExclusive(true);
	
	m_atkinson = new QRadioButton(tr("Atkinson"));
	connect(m_atkinson, SIGNAL(clicked()), SLOT(handleControlValueChanged()));
	m_buttonGroup->addButton(m_atkinson, DitherFilter::Atkinson);

	m_floydSteinberg = new QRadioButton(tr("Floyd-Steinberg"));
	connect(m_floydSteinberg, SIGNAL(clicked()), SLOT(handleControlValueChanged()));
	m_buttonGroup->addButton(m_floydSteinberg, DitherFilter::FloydSteinberg);

	m_random = new QRadioButton(tr("Random"));
	connect(m_random, SIGNAL(clicked()), SLOT(handleControlValueChanged()));
	m_buttonGroup->addButton(m_random, DitherFilter::Random);
	
	m_atkinson->setChecked(true);
	
	QVBoxLayout *layout = new QVBoxLayout(w);
	layout->addWidget(m_atkinson);
	layout->addWidget(m_floydSteinberg);
	layout->addWidget(m_random);
	box->setLayout(layout);
	
	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}

void DitherFilter::handleControlValueChanged() {
	
	setOption(DitherFilter::Mode, m_buttonGroup->checkedId());
	QtImageFilter::handleControlValueChanged();
}
