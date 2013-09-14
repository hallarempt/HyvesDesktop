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

#include "charcoalfilter.h"

#include <QtGui>
#include <QtCore>

#include "QtImageFilterFactory"
#include "convolutionfilter.h"

CharcoalFilter::CharcoalFilter()
		: QtImageFilter()
		, m_radiusSlider(0)
		, m_sigmaSlider(0)
{
	setThreadedPreviewEnabled(true);
	resetOptions();
}

void CharcoalFilter::resetOptions()
{
	
	m_radius = 10;
	m_sigma = 10;
	if (m_radiusSlider) {
		m_radiusSlider->blockSignals(true);
		m_sigmaSlider->blockSignals(true);

		m_radiusSlider->setValue(m_radius);
		m_sigmaSlider->setValue(m_sigma);

		m_radiusSlider->blockSignals(false);
		m_sigmaSlider->blockSignals(false);
	}
	QtImageFilter::handleControlValueChanged();
	
}

CharcoalFilter::~CharcoalFilter()
{
}

QImage CharcoalFilter::apply(const QImage& image, const QRect& clipRect)
{
        QMutexLocker locker(&m_mutex);

	m_isAborting = false;
	QImage resultImg = image.convertToFormat(QImage::Format_ARGB32);	
	setProgress(1);
	
	QtImageFilter *gray = setCurrentSubFilter("Gray", 10);
	QImage grayImage = gray->apply(resultImg, clipRect);
	unsetCurrentSubFilter();
	if (m_isAborting) return QImage();
	
	QtImageFilter *edgeFilter = setCurrentSubFilter("EdgeDetect", 15);
	QImage edgeImage = edgeFilter->apply(grayImage, clipRect);
	unsetCurrentSubFilter();
	if (m_isAborting) return QImage();

	QtImageFilter *blurFilter = setCurrentSubFilter("GaussianBlur", 50);
	blurFilter->setOption(QtImageFilter::Radius, m_radius);
	blurFilter->setOption(QtImageFilter::Deviation, m_sigma);
	QImage blurImage = blurFilter->apply(edgeImage, clipRect);
	unsetCurrentSubFilter();
	if (m_isAborting) return QImage();
	
	QtImageFilter *normalizeFilter = setCurrentSubFilter("Normalize", 15);
	QImage normalImage = normalizeFilter->apply(blurImage, clipRect);
	unsetCurrentSubFilter();
	if (m_isAborting) return QImage();
	
	//QtImageFilter *invertFilter = setCurrentSubFilter("Invert", 10);
	//QImage invertImage = invertFilter->apply(normalImage, clipRect);
	//unsetCurrentSubFilter();
	
	if (m_isAborting) return QImage();	
	
	return normalImage;
	
}

QString CharcoalFilter::name() const {
	
	return QObject::tr("Charcoal Drawing");
}

QString CharcoalFilter::description() const {
	
	return QObject::tr("Highlight the edges to simulate a charcoal drawing");
}

QVariant CharcoalFilter::option(int filteroption) const
{
	switch ((CharcoalFilterOption)filteroption) {
	case Radius:
		return QVariant(m_radius);
		break;
	case Sigma:
		return QVariant(m_sigma);
		break;
	default:
		break;
	}
	return QVariant();
}

bool CharcoalFilter::setOption(int filteroption, const QVariant &value)
{
	switch ((CharcoalFilterOption)filteroption) {
	case Radius:
		m_radius = value.toDouble();
		break;
	case Sigma:
		m_sigma = value.toDouble();
		break;
	default:
		break;
	}
	return true;
}

bool CharcoalFilter::supportsOption(int option) const
{
	bool supports = false;
	switch ((CharcoalFilterOption)option) {
	case Radius:
	case Sigma:
		supports = true;
		break;
	default:
		break;
	}
	return supports;
}

QList<int> CharcoalFilter::options() const {
	QList<int> ret;
	ret << CharcoalFilter::Radius << CharcoalFilter::Sigma;
	return ret;
}

QWidget *CharcoalFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
        l->setMargin(2);
        l->setSpacing(2);

	
	m_radiusSlider = new QSlider(w);
	m_radiusSlider->setValue(m_radius);
	m_radiusSlider->setMinimum(0);
	m_radiusSlider->setOrientation(Qt::Horizontal);
	m_radiusSlider->setMaximum(40);
	m_radiusSlider->setTracking(false);
	connect(m_radiusSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));
	
	l->addWidget(new QLabel(QObject::tr("Radius:"), w), 0, 0, 1, 1);
	l->addWidget(m_radiusSlider, 0, 1, 1, 1);
	
	m_sigmaSlider = new QSlider(w);
	m_sigmaSlider->setValue(m_sigma);
	m_sigmaSlider->setOrientation(Qt::Horizontal);
	m_sigmaSlider->setMinimum(0);
	m_sigmaSlider->setMaximum(30);
	m_sigmaSlider->setTracking(false);
	connect(m_sigmaSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));
	
	l->addWidget(new QLabel(QObject::tr("Sigma:"), w), 1, 0, 1, 1);
	l->addWidget(m_sigmaSlider, 1, 1, 1, 1);
	
	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}

void CharcoalFilter::handleControlValueChanged() {
	
	setOption(CharcoalFilter::Radius, m_radiusSlider->value());
	setOption(CharcoalFilter::Sigma, m_sigmaSlider->value());	
	QtImageFilter::handleControlValueChanged();
}
