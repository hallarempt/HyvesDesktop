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
#include "desaturatefilter.h"

#include <QtGui>
#include <QtCore>

#include "colorconversions.h"

DesaturateFilter::DesaturateFilter()
	: QtImageFilter()
	, m_desaturationSlider(0)
{
	resetOptions();
}


void DesaturateFilter::resetOptions()
{
	m_desaturation = 50;
	if (m_desaturationSlider) {
		m_desaturationSlider->blockSignals(true);
		m_desaturationSlider->setValue(m_desaturation);
		m_desaturationSlider->blockSignals(false);
	}
	QtImageFilter::handleControlValueChanged();
}

DesaturateFilter::~DesaturateFilter() {}

QVariant DesaturateFilter::option(int option) const {
	if (DesaturateFilterOption(option) == Desaturation) {
		return QVariant(m_desaturation);
	}
	return QVariant();
}

bool DesaturateFilter::setOption(int option, const QVariant &value) {
	if (DesaturateFilterOption(option) == Desaturation) {
		m_desaturation = value.toInt();
		return true;
	}
	return false;
}

bool DesaturateFilter::supportsOption(int option) const {
	if (DesaturateFilterOption(option) == Desaturation) {
		return true;
	}
	return false;
}

QList<int> DesaturateFilter::options() const {
	QList<int> ret;
	ret << DesaturateFilter::Desaturation;
	return ret;
}

QImage DesaturateFilter::apply(const QImage &image, const QRect& clipRect ) {


        QMutexLocker locker(&m_mutex);

	QImage resultImg = image.convertToFormat(QImage::Format_ARGB32);
	
	QRect rc = clipRect;
	if (rc.isEmpty()) {
		rc = image.rect();
	}
	m_isAborting = false;
	for (int y = rc.y(); y <= rc.bottom(); ++y) {
		if (m_isAborting) {
			return QImage();
		}
		QRgb* line = reinterpret_cast<QRgb*>(resultImg.scanLine(y));
		for (int x = rc.x(); x <= rc.right(); ++x) {
			line[x] = ColorConversions::desaturate(line[x], m_desaturation);
		}
		setProgress(int((y - rc.y()) * 100.0 / rc.height()));
	}
	
	return resultImg;
}

QString DesaturateFilter::name() const { return QObject::tr("Desaturate"); }

QString DesaturateFilter::description() const { return QObject::tr("Change the color saturation of the image"); }

QWidget *DesaturateFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
        l->setMargin(2);
        l->setSpacing(2);

	m_desaturationSlider = new QSlider(w);
	m_desaturationSlider->setMaximum(100);
	m_desaturationSlider->setMinimum(0);
	m_desaturationSlider->setOrientation(Qt::Horizontal);
	m_desaturationSlider->setValue(m_desaturation);
	connect(m_desaturationSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));
	
	l->addWidget(new QLabel(QObject::tr("Desaturation:"), w), 0, 0, 1, 1);
	l->addWidget(m_desaturationSlider, 0, 1, 1, 1);
	
	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}

void DesaturateFilter::handleControlValueChanged() {
	
	m_desaturation = m_desaturationSlider->value();
	QtImageFilter::handleControlValueChanged();
}


