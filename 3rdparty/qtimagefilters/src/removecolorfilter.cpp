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

#include "removecolorfilter.h"

#include <QtGui>
#include <QtCore>

#include "colorconversions.h"	

RemoveColorFilter::RemoveColorFilter()
	: QtImageFilter()
	, m_colorWell(0)
	, m_separateThresholdsCheck(0)
{
	resetOptions();
}

void RemoveColorFilter::resetOptions()
{

	m_color = Qt::red;
	m_threshold = 128;
	m_thresholdRed = 128;
	m_thresholdGreen = 128;
	m_thresholdBlue = 128;
	m_separateThresholds = false;
	
	if (m_colorWell) {
		m_colorWell->setColor(m_color);
		foreach(QSlider *slider, m_sliders) {
			slider->blockSignals(true);
			slider->setValue(128);
			slider->blockSignals(false);
		}
		m_separateThresholdsCheck->blockSignals(true);
		m_separateThresholdsCheck->setChecked(m_separateThresholds);
		m_separateThresholdsCheck->blockSignals(false);
	}
	
	QtImageFilter::handleControlValueChanged();
}


RemoveColorFilter::~RemoveColorFilter()
{
}

QImage RemoveColorFilter::apply(const QImage& image, const QRect& clipRect)
{
        QMutexLocker locker(&m_mutex);

	
	QImage resultImg = image.convertToFormat(QImage::Format_ARGB32);	
	QRect rc = clipRect;
	if (rc.isEmpty()) {
		rc = image.rect();
	}
	m_isAborting = false;
	QRgb color = m_color.rgb();
	for (int y = rc.y(); y <= rc.bottom(); ++y) {
		if (m_isAborting) {
			return QImage();
		}
		QRgb* line = reinterpret_cast<QRgb*>(resultImg.scanLine(y));
		for (int x = rc.x(); x <= rc.right(); ++x) {

			QRgb pixel = line[x];
			
			if (m_separateThresholds) {
				quint8 red = qAbs(qRed(pixel) - qRed(color));
				quint8 green = qAbs(qGreen(pixel) - qGreen(color));
				quint8 blue = qAbs(qBlue(pixel) - qBlue(color));
				
				if (red < m_thresholdRed &&
				    green < m_thresholdGreen &&
				    blue < m_thresholdBlue ) {
					
					int gray = qGray(pixel);
					line[x] = qRgb(gray, gray, gray);
				}
			}
			else {
				if (ColorConversions::difference(pixel, color) < m_threshold) {

					int gray = qGray(pixel);
					line[x] = qRgb(gray, gray, gray);

				}
			}

		}
		setProgress(int((y - rc.y()) * 100.0 / rc.height()));
	}

	return resultImg;
}

QString RemoveColorFilter::name() const {
	
	return QObject::tr("Remove Color");
}

QString RemoveColorFilter::description() const {
	
	return QObject::tr("Remove the specified color from the image");
}

QVariant RemoveColorFilter::option(int filteroption) const
{
	switch ((RemoveColorFilterOption)filteroption) {
	case Color:
		if (m_color.isValid()) {
			return QVariant(m_color);
		}
		return QColor(Qt::red);
		break;
	case Threshold:
		return m_threshold;
		break;
	case ThresholdRed:
		return m_thresholdRed;
		break;
	case ThresholdGreen:
		return m_thresholdGreen;
		break;
	case ThresholdBlue:
		return m_thresholdBlue;
		break;
	case SeparateThresholds:
		return m_separateThresholds;
		break;
	default:
		break;
	}
	return QVariant();
}

bool RemoveColorFilter::setOption(int filteroption, const QVariant &value)
{
	switch ((RemoveColorFilterOption)filteroption) {
	case Color:
		m_color = value.value<QColor>();
		break;
	case Threshold:
		m_threshold = value.toInt();
		break;
	case ThresholdRed:
		m_thresholdRed = value.toInt();
		break;
	case ThresholdGreen:
		m_thresholdGreen = value.toInt();
		break;
	case ThresholdBlue:
		m_thresholdBlue = value.toInt();
		break;
	case SeparateThresholds:
		m_separateThresholds = value.toBool();
		break;
	default:
		break;
	}
	return true;
}

bool RemoveColorFilter::supportsOption(int option) const
{
	bool supports = false;
	switch ((RemoveColorFilterOption)option) {
	case Color:
	case Threshold:
	case ThresholdRed:
	case ThresholdGreen:
	case ThresholdBlue:
	case SeparateThresholds:
		supports = true;
		break;
	default:
		break;
	}
	return supports;
}

QList<int> RemoveColorFilter::options() const {
	QList<int> ret;
	ret << RemoveColorFilter::Color << RemoveColorFilter::Threshold
		<< RemoveColorFilter::ThresholdRed << RemoveColorFilter::ThresholdGreen << RemoveColorFilter::ThresholdBlue
		<< RemoveColorFilter::SeparateThresholds;
	return ret;
}

QWidget *RemoveColorFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;

	m_colorWell = new ColorWell(w);
	m_colorWell->setColor(m_color);
	connect(m_colorWell, SIGNAL(colorChanged(QColor)), SLOT(handleControlValueChanged()));
	l->addWidget(new QLabel(QObject::tr("Color:"), w), 0, 0, 1, 1);
	l->addWidget(m_colorWell, 0, 1, 1, 1);
	
	m_separateThresholdsCheck = new QCheckBox(w);
	m_separateThresholdsCheck->setChecked(m_separateThresholds);
	l->addWidget(new QLabel(QObject::tr("Separate RGB:"), w), 1, 0, 1, 1);
	l->addWidget(m_separateThresholdsCheck, 1, 1, 1, 1);
	connect(m_separateThresholdsCheck, SIGNAL(stateChanged(int)), this, SLOT(handleControlValueChanged()));

	typedef QPair<QString,QString> ID;
	
	QMap<ID, int> defaultValues;
	
	defaultValues[ID(tr("Threshold:"), "Threshold")] = m_threshold;
	defaultValues[ID(tr("Red:"), "Red")] = m_thresholdRed;
	defaultValues[ID(tr("Green:"), "Green")] = m_thresholdGreen;
	defaultValues[ID(tr("Blue:"), "Blue")] = m_thresholdBlue;
	int row = 2;
	foreach (ID pair, defaultValues.keys()) {
		++row;
		QSlider *slider = new QSlider(w);
		slider->setMinimum(0);
		slider->setMaximum(255);
		slider->setOrientation(Qt::Horizontal);
		slider->setValue(defaultValues[pair]);
		l->addWidget(new QLabel(pair.first, w), row, 0, 1, 1);
		l->addWidget(slider, row, 1, 1, 1);
		connect(slider, SIGNAL(valueChanged(int)), this, SLOT(handleControlValueChanged()));
		m_sliders[pair.second] = slider;
	}
	m_sliders["Threshold"]->setEnabled(false);
	
	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}

void RemoveColorFilter::handleControlValueChanged() {
	
	setOption(RemoveColorFilter::Color, m_colorWell->selectedColor());
	setOption(RemoveColorFilter::SeparateThresholds, m_separateThresholdsCheck->isChecked());
	
	m_sliders["Red"]->setEnabled(m_separateThresholds);
	m_sliders["Green"]->setEnabled(m_separateThresholds);
	m_sliders["Blue"]->setEnabled(m_separateThresholds);
	m_sliders["Threshold"]->setEnabled(!m_separateThresholds);

	setOption(RemoveColorFilter::Threshold, m_sliders["Threshold"]->value());
	setOption(RemoveColorFilter::ThresholdRed, m_sliders["Red"]->value());
	setOption(RemoveColorFilter::ThresholdGreen, m_sliders["Green"]->value());
	setOption(RemoveColorFilter::ThresholdBlue, m_sliders["Blue"]->value());

	QtImageFilter::handleControlValueChanged();
}
