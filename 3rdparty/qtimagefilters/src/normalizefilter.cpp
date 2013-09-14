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
#include "normalizefilter.h"

#include <QtGui>
#include <QtCore>

NormalizeFilter::NormalizeFilter()
		: QtImageFilter()
		, m_limitToRectCheck(0)
		, m_normalizeChannelsIndependentlyCheck(0)
{
	resetOptions();
}

void NormalizeFilter::resetOptions()
{
	m_limitToRect = true;
	m_normalizeChannelsIndependently = true;
	if (m_limitToRectCheck) {
		m_limitToRectCheck->blockSignals(true);
		m_normalizeChannelsIndependentlyCheck->blockSignals(true);

		m_limitToRectCheck->setChecked(m_limitToRect);
		m_normalizeChannelsIndependentlyCheck->setChecked(m_normalizeChannelsIndependently);

		m_limitToRectCheck->blockSignals(false);
		m_normalizeChannelsIndependentlyCheck->blockSignals(false);
	}
	QtImageFilter::handleControlValueChanged();
}

NormalizeFilter::~NormalizeFilter()
{
}

QImage NormalizeFilter::apply(const QImage &image, const QRect& clipRect)
{
        QMutexLocker locker(&m_mutex);


	QRect rc = clipRect;
	if (rc.isEmpty()) {
		rc = image.rect();
	}
	
	QImage resultImg = image.convertToFormat(QImage::Format_ARGB32);	
	
	double minRed = 255;
	double maxRed = 0;
	
	double minGreen = 255;
	double maxGreen = 0;
	
	double minBlue = 255;
	double maxBlue = 0;
	
	quint8 lutRed[256];
	quint8 lutGreen[256];
	quint8 lutBlue[256];
	
	// First pass, determine the max/min for the channels
	QRect rc2;
	if (m_limitToRect) {
		rc2 = rc;
	}
	else {
		rc2 = image.rect();
	}
	
	for (int y = rc2.y(); y < rc2.height(); ++y) {
		for (int x = rc2.x(); x < rc2.width(); ++ x) {
			QRgb pixel = resultImg.pixel(x, y);
			int red = qRed(pixel);
			if (red < minRed) minRed = red;
			if (red > maxRed) maxRed = red;
			
			int green = qGreen(pixel);
			
			if (m_normalizeChannelsIndependently) {
				if (green < minGreen) minGreen = green;
				if (green > maxGreen) maxGreen = green;
			}
			else {
				if (green < minRed) minRed = green;
				if (green > maxRed) maxRed = green;
			}
			int blue = qBlue(pixel);
			if (m_normalizeChannelsIndependently) {
				if (blue < minBlue) minBlue = blue;
				if (blue > maxBlue) maxBlue = blue;
			}
			else {
				if (blue < minRed) minRed = blue;
				if (blue > maxRed) maxRed = blue;
			}
		}
		if (m_isAborting) return QImage();
		setProgress(int((y - rc.y()) * 25.0 / rc.height()));
	}
	
	quint8 rangeRed = maxRed - minRed;
	quint8 rangeGreen = maxGreen - minGreen;
	quint8 rangeBlue = maxBlue - minBlue;
	
	if (rangeRed != 0) {
		for ( int val = minRed; val <= maxRed; ++val ) {
			lutRed[val] = 255 * (val - minRed) / rangeRed;
		}
	}
	
	if (m_normalizeChannelsIndependently) {
		if (rangeGreen != 0) {
			for ( int val = minGreen; val <= maxGreen; ++val ) {
				lutGreen[val] = 255 * (val - minGreen) / rangeGreen;
			}	
		}
		
		if (rangeBlue != 0) {
			for ( int val = minBlue; val <= maxBlue; ++val ) {
				lutBlue[val] = 255 * (val - minBlue) / rangeBlue;
			}	
		}
	}
	
	for (int y = rc.y(); y < rc2.height(); ++y) {
		for (int x = rc.x(); x < rc.width(); ++ x) {

			QRgb pixel = resultImg.pixel(x, y);

			if (m_normalizeChannelsIndependently) {
				resultImg.setPixel(x, y, qRgb( lutRed[qRed(pixel)], 
							       lutGreen[qGreen(pixel)], 
							       lutBlue[qBlue(pixel)]));
			}
			else {
				resultImg.setPixel(x, y, qRgb( lutRed[qRed(pixel)], 
							       lutRed[qGreen(pixel)], 
							       lutRed[qBlue(pixel)]));
			}
		}
		if (m_isAborting) return QImage();
		setProgress(int((y - rc.y()) * 75.0 / rc.height()));
	}
	return resultImg;
}

QString NormalizeFilter::name() const
{
	return QObject::tr("Normalize");
}

QString NormalizeFilter::description() const
{
	return QObject::tr("Enhance the contrast of the image by making the pixels span the entire range of color");
}


QVariant NormalizeFilter::option(int filteroption) const
{
	switch ((NormalizeFilterOption)filteroption) {
	case LimitToRect:
		return QVariant(m_limitToRect);
		break;
	case NormalizeIndependently:
		return QVariant(m_normalizeChannelsIndependently);
		break;
	default:
		break;
	}
	return QVariant();
}

bool NormalizeFilter::setOption(int filteroption, const QVariant &value)
{
	switch ((NormalizeFilterOption)filteroption) {
	case LimitToRect:
		m_limitToRect = value.toBool();
		break;
	case NormalizeIndependently:
		m_normalizeChannelsIndependently = value.toBool();
		break;
	default:
		break;
	}
	return true;
}

bool NormalizeFilter::supportsOption(int option) const
{
	bool supports = false;
	switch ((NormalizeFilterOption)option) {
	case LimitToRect:
	case NormalizeIndependently:
		supports = true;
		break;
	default:
		break;
	}
	return supports;
}

QList<int> NormalizeFilter::options() const {
	QList<int> ret;
	ret << NormalizeFilter::LimitToRect << NormalizeFilter::NormalizeIndependently;
	return ret;
}

QWidget *NormalizeFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
        l->setMargin(2);
        l->setSpacing(2);

	m_normalizeChannelsIndependentlyCheck = new QCheckBox(tr("Normalize each channel independently"), w);
	m_normalizeChannelsIndependentlyCheck->setChecked(m_normalizeChannelsIndependently);
	l->addWidget(m_normalizeChannelsIndependentlyCheck, 0, 0, 1, 1);
	connect(m_normalizeChannelsIndependentlyCheck, SIGNAL(stateChanged(int)), SLOT(handleControlValueChanged()));

	m_limitToRectCheck = new QCheckBox(tr("Normalize only within the selected rectangle"), w);
	m_limitToRectCheck->setChecked(m_limitToRectCheck);
	l->addWidget(m_limitToRectCheck, 1, 0, 1, 1);
	connect(m_limitToRectCheck, SIGNAL(stateChanged(int)), SLOT(handleControlValueChanged()));
	
	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}

void NormalizeFilter::handleControlValueChanged() {
	
	setOption(NormalizeFilter::NormalizeIndependently, m_normalizeChannelsIndependentlyCheck->isChecked());
	setOption(NormalizeFilter::LimitToRect, m_normalizeChannelsIndependentlyCheck->isChecked());
	
	QtImageFilter::handleControlValueChanged();
}
