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


#include "warholfilter.h"

#include <math.h>

#include <QtGui>
#include <QtCore>
#include "colorconversions.h"
#include "thresholdfilter.h"

WarholFilter::WarholFilter() : QtImageFilter()
{
	resetOptions();
}

void WarholFilter::resetOptions() {	
	
	m_min = 0;
	m_max = 95;
	m_colorTopLeft = Qt::cyan;
	m_colorTopRight = Qt::magenta;
	m_colorBottomLeft = Qt::yellow;
	m_colorBottomRight = Qt::green;
	
	if (m_sliders.count() > 0 && m_colorWells.count() > 0) {
		QStringList colorWellNames;
		colorWellNames << "TopLeft" << "TopRight" << "BottomLeft" << "BottomRight";
		QStringList sliderNames;
		sliderNames << "Min" << "Max";
		foreach(QString cn, colorWellNames) m_colorWells[cn]->blockSignals(true);
		foreach(QString sn, sliderNames) m_sliders[sn]->blockSignals(true);
		
		m_colorWells["TopLeft"]->setColor(m_colorTopLeft);
		m_colorWells["TopRight"]->setColor(m_colorTopRight);
		m_colorWells["BottomLeft"]->setColor(m_colorBottomLeft);
		m_colorWells["BottomRight"]->setColor(m_colorBottomRight);
		
		m_sliders["Min"]->setValue(m_min);
		m_sliders["Max"]->setValue(m_max);
		
		foreach(QString cn, colorWellNames) m_colorWells[cn]->blockSignals(false);
		foreach(QString sn, sliderNames) m_sliders[sn]->blockSignals(false);
	}
	
	QtImageFilter::handleControlValueChanged();
}


WarholFilter::~WarholFilter()
{
}

QImage WarholFilter::apply(const QImage& image, const QRect& clipRect)
{
        QMutexLocker locker(&m_mutex);

	
	QImage img1 = image.scaled(image.size() / 2, Qt::KeepAspectRatio);
	QImage resultImg(img1.width() * 2, img1.height() * 2, QImage::Format_ARGB32);
	
	QPainter gc(&resultImg);
	
	gc.drawImage(QPoint(0, 0), img1);
	gc.drawImage(QPoint(0, img1.height()), img1);
	gc.drawImage(QPoint(img1.width(), 0), img1);
	gc.drawImage(QPoint(img1.width(), img1.height()), img1);
	gc.end();
	
	QRect rc = clipRect;
	if (rc.isEmpty()) {
		rc = resultImg.rect();
	}
	rc &= resultImg.rect();
	
	m_isAborting = false;
	for (int y = rc.y(); y <= rc.bottom(); ++y) {
		if (m_isAborting) {
			return QImage();
		}
		QRgb* line = reinterpret_cast<QRgb*>(resultImg.scanLine(y));
		for (int x = rc.x(); x <= rc.right(); ++x) {
			if (x < img1.width() && y < img1.height()) {
				line[x] = ThresholdFilter::threshold(line[x], m_min, m_max, m_colorTopLeft.rgb(), qRgb(0, 0, 0));
			}
			else if (x < img1.width() && y > img1.height()) {
				line[x] = ThresholdFilter::threshold(line[x], m_min, m_max, m_colorBottomLeft.rgb(), qRgb(0, 0, 0));
			}
			else if (x > img1.width() && y < img1.height()) {
				line[x] = ThresholdFilter::threshold(line[x], m_min, m_max, m_colorTopRight.rgb(), qRgb(0, 0, 0));
			}
			else {
				line[x] = ThresholdFilter::threshold(line[x], m_min, m_max, m_colorBottomRight.rgb(), qRgb(0, 0, 0));
			}
		}
		setProgress(int((y - rc.y()) * 100.0 / rc.height()));
	}
	return resultImg;

}


QVariant WarholFilter::option(int option) const {
	switch (WarholFilterOption(option)) {
	case Min:
		return QVariant(m_min);
	case Max:
		return QVariant(m_max);
	case ColorTopLeft:
		return QVariant(m_colorTopLeft);
	case ColorBottomLeft:
		return QVariant(m_colorBottomLeft);
	case ColorTopRight:
		return QVariant(m_colorTopRight);
	case ColorBottomRight:
		return QVariant(m_colorBottomRight);
	}
	return QVariant();
}

bool WarholFilter::setOption(int option, const QVariant &value) {
	switch (WarholFilterOption(option)) {
	case Min:
		{
			int i = value.toInt();
			i = qBound(0, i, 100);
			m_min = i;
			
			return true;
		}
	case Max:
		{
			int i = value.toInt();
			i = qBound(0, i, 100);
			m_max = i;
			return true;
		}
	case ColorTopLeft:
		m_colorTopLeft = value.value<QColor>();
		return true;
	case ColorBottomLeft:
		m_colorBottomLeft = value.value<QColor>();
		return true;
	case ColorTopRight:
		m_colorTopRight = value.value<QColor>();
		return true;
	case ColorBottomRight:
		m_colorBottomRight = value.value<QColor>();
		return true;
	}
	return false;
}

bool WarholFilter::supportsOption(int option) const {
	switch (WarholFilterOption(option)) {
	case Min:
	case Max:
	case ColorTopLeft:
	case ColorTopRight:
	case ColorBottomLeft:
	case ColorBottomRight:
		return true;
	}
	return false;
}

QList<int> WarholFilter::options() const {
	QList<int> ret;
	ret << WarholFilter::Min << WarholFilter::Max
		<< WarholFilter::ColorTopLeft << WarholFilter::ColorTopRight
		<< WarholFilter::ColorBottomLeft << WarholFilter::ColorBottomRight;
	return ret;
}

QString WarholFilter::name() const {
	
	return QObject::tr("Andy Warhol Effect");
}

QString WarholFilter::description() const {
	
	return QObject::tr("Instant popart.");
}

QWidget *WarholFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
        l->setMargin(2);
        l->setSpacing(2);

	
	typedef QPair<QString,QString> ID;
	
	QMap<ID, int> defaultValues;
	
	defaultValues[ID(tr("Minimum Level:"), "Min")] = m_min;
	defaultValues[ID(tr("Maximum Level:"), "Max")] = m_max;
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
		m_sliders[pair.second] = slider;
	}
	

	QList<ID> colorIDs;
	colorIDs << ID(tr("Top Left:"), "TopLeft") <<
			ID(tr("Top Right:"), "TopRight") <<
			ID(tr("Bottom Left:"), "BottomLeft") <<
			ID(tr("Bottom Right:"), "BottomRight");

	
	
	QList<QColor> defaultColors;
	defaultColors << m_colorTopLeft << m_colorTopRight << m_colorBottomLeft << m_colorBottomRight;
	
	for (int i = 0; i < 4; ++i) {
		++row;
		ID pair = colorIDs[i];
		ColorWell *well = new ColorWell(w);
		well->setColor(defaultColors[i]);
		l->addWidget(new QLabel(pair.first, w), row, 0, 1, 1);
		l->addWidget(well, row, 1, 1, 2);
		connect(well, SIGNAL(colorChanged(QColor)), SLOT(handleControlValueChanged()));
		m_colorWells[pair.second] = well;
	}
	
	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}

void WarholFilter::handleControlValueChanged() {

	int min = m_sliders["Min"]->value();
	int max = m_sliders["Max"]->value();
	if (min > max) {
		if (sender() == m_sliders["Min"]) {
			max = min;
		} else if (sender() == m_sliders["Max"]) {
			min = max;
		}
		m_sliders["Max"]->blockSignals(true);
		m_sliders["Min"]->blockSignals(true);
		m_sliders["Max"]->setValue(max);
		m_sliders["Min"]->setValue(min);
		m_sliders["Max"]->blockSignals(false);
		m_sliders["Min"]->blockSignals(false);
	}
	setOption(WarholFilter::Min, min);
	setOption(WarholFilter::Max, max);

	setOption(WarholFilter::ColorTopLeft, m_colorWells["TopLeft"]->selectedColor());
	setOption(WarholFilter::ColorTopRight, m_colorWells["TopRight"]->selectedColor());
	setOption(WarholFilter::ColorBottomLeft, m_colorWells["BottomLeft"]->selectedColor());
	setOption(WarholFilter::ColorBottomRight, m_colorWells["BottomRight"]->selectedColor());

	QtImageFilter::handleControlValueChanged();
}
