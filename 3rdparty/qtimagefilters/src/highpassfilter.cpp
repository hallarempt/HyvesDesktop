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


#include "highpassfilter.h"

#include <math.h>

#include <QtGui>
#include <QtCore>

#include "colorconversions.h"

HighPassFilter::HighPassFilter() 
	: QtImageFilter(), m_radiusSlider(0)
{
	resetOptions();
}

void HighPassFilter::resetOptions()
{
	m_radius = 7;
	if (m_radiusSlider) {
		m_radiusSlider->blockSignals(true);
		m_radiusSlider->setValue(m_radius);
		m_radiusSlider->blockSignals(false);
	}
	QtImageFilter::handleControlValueChanged();
}

HighPassFilter::~HighPassFilter()
{
}

QImage HighPassFilter::apply(const QImage& image, const QRect& clipRect)
{
        QMutexLocker locker(&m_mutex);

	
	QRect rc = clipRect;
	if (rc.isEmpty()) {
		rc = image.rect();
	}
	
	QImage resultImg = image.convertToFormat(QImage::Format_ARGB32);	

	// First copy the image twice
	QImage blurred = resultImg;
	
	// Blur one copy
	QtImageFilter* blur = setCurrentSubFilter("GaussianBlur", 90);
	blur->setOption(QtImageFilter::FilterBorderPolicy, "extend");	
	blur->setOption(QtImageFilter::Radius, m_radius);
	blurred = blur->apply(blurred, rc);	
	unsetCurrentSubFilter();
	if (m_isAborting) return QImage();
	
	// Invert it
	QtImageFilter* invert = setCurrentSubFilter("Invert", 10);
	blurred = invert->apply(blurred, rc);
	unsetCurrentSubFilter();
	if (m_isAborting) return QImage();
	
	QPainter gc(&resultImg);
	gc.setOpacity(0.5);
	gc.setCompositionMode(QPainter::CompositionMode_SourceOver);
	gc.drawImage(rc.topLeft(), blurred, rc);
	gc.end();
	if (m_isAborting) return QImage();
	
	return resultImg;
}


QVariant HighPassFilter::option(int option) const {
	switch (HighPassFilterOption(option)) {
		case Radius:
			return m_radius;
	}
	return QVariant();
}

bool HighPassFilter::setOption(int option, const QVariant &value) {
	switch (HighPassFilterOption(option)) {
		case Radius:
			m_radius = value.toInt();
			return true;
	}
	return false;
}

bool HighPassFilter::supportsOption(int option) const {
	switch (HighPassFilterOption(option)) {
		case Radius:
		return true;
	}
	return false;
}

QList<int> HighPassFilter::options() const {
	QList<int> ret;
	ret << HighPassFilter::Radius;
	return ret;
}

QString HighPassFilter::name() const {
	
	return QObject::tr("Highpass");
}

QString HighPassFilter::description() const {
	
	return QObject::tr("Filter out the lowers frequency bands in the image.");
}
QWidget *HighPassFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
        l->setMargin(2);
        l->setSpacing(2);

	m_radiusSlider = new QSlider(w);
	m_radiusSlider->setValue(m_radius);
	m_radiusSlider->setMinimum(0);
	m_radiusSlider->setMaximum(40);
	connect(m_radiusSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));

	l->addWidget(new QLabel(QObject::tr("Radius:"), w), 0, 0, 1, 1);
	l->addWidget(m_radiusSlider, 0, 1, 1, 1);

	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}

void HighPassFilter::handleControlValueChanged() {

	setOption(HighPassFilter::Radius, m_radiusSlider->value());
	
	QtImageFilter::handleControlValueChanged();
}
