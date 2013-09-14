/****************************************************************************
**
** This file is part of a Qt Solutions component.
** 
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
** 
** Contact:  Qt Software Information (qt-info@nokia.com)
** 
** Commercial Usage  
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Solutions Commercial License Agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
** 
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
** 
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
** 
** GNU General Public License Usage 
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
** 
** Please note Third Party Software included with Qt Solutions may impose
** additional restrictions and it is the user's responsibility to ensure
** that they have met the licensing requirements of the GPL, LGPL, or Qt
** Solutions Commercial license and the relevant license of the Third
** Party Software they are using.
** 
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** 
****************************************************************************/


#include "gaussfilter.h"

#include <QtGui>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif


static qreal Gauss2DFunction(int x, int y, qreal deviation)
{
	/**
    * A 2-D gaussian distribution has the shape:
    *
    *               1         -(x*x + y*y)/(2*dev*dev)
    * G(x,y) = ---------- * e^
    *          2*PI*dev^2
    *
    * see http://www.cee.hw.ac.uk/hipr/html/gsmooth.html for a more readable version
    */
	return exp(-(x*x + y*y)/(2*deviation*deviation))/(2*M_PI*deviation*deviation);
}

GaussBlurFilter::GaussBlurFilter() : ConvolutionFilter(), m_slider(0)
{
	setThreadedPreviewEnabled(true);
	resetOptions();
}

void GaussBlurFilter::resetOptions()
{
	m_deviation = -1;
	m_radius = 40;
	setOption(QtImageFilter::FilterChannels, "rgba");
	setOption(QtImageFilter::FilterBorderPolicy, "extend");

	if (m_slider) {
		m_slider->blockSignals(true);
		m_slider->setValue(m_radius);
		m_slider->blockSignals(false);
	}
	QtImageFilter::handleControlValueChanged();
}

QImage GaussBlurFilter::apply(const QImage &image, const QRect& clipRect )
{
	bool ok = true;
	qreal radius = m_radius * image.width() / 1000.0;  // hope we will never have the need for absolute radius
	if (radius > 0.0) {
		m_kernels.clear();
		int uRadius = (int)ceil(radius);
		
		
		double deviation = m_deviation;
		if (deviation < 0) {
			deviation = sqrt(-radius*radius/(2*log(1/255.0)));
		}
		
		QtMatrix<double> matLeft(2 * uRadius + 1, 1);
		
		for (int x = -uRadius; x <=uRadius; x++) {
			matLeft.setData(uRadius + x, 0, Gauss2DFunction(x, 0, deviation));
		}
		double scalar = matLeft.at(uRadius, 0);
		matLeft*=(255.0/scalar);
		QtMatrix<double> matRight = matLeft.transposed();
		
		QtConvolutionKernelMatrix integerMatrixLeft = convertMatrixBasetype<int,double>(matLeft);
		QtConvolutionKernelMatrix integerMatrixRight = convertMatrixBasetype<int,double>(matRight);
		
		GaussBlurFilter *localThis = const_cast<GaussBlurFilter*>(this);
		localThis->addKernel(integerMatrixLeft, m_channels, m_borderPolicy);
		localThis->addKernel(integerMatrixRight, m_channels, m_borderPolicy);
	} else {
		ok = false;
		return image;
	}
	return ConvolutionFilter::apply(image, clipRect);
}


QVariant GaussBlurFilter::option(int option) const
{
	if (option == QtImageFilter::Radius) return true;
	return ConvolutionFilter::option(option);
}

bool GaussBlurFilter::setOption(int option, const QVariant &value)
{
	bool ok = true;
	switch (option) {
	case QtImageFilter::Radius:
		{
			double radius = value.toDouble(&ok);
			if (ok) m_radius = radius;
		}
		break;
		
	default:
		ok = ConvolutionFilter::setOption(option, value);
		break;
	}
	return ok;
}

bool GaussBlurFilter::supportsOption(int option) const
{
	if (option == QtImageFilter::Radius) return true;
	return ConvolutionFilter::supportsOption(option);
}

QList<int> GaussBlurFilter::options() const {
	QList<int> ret;
	ret << QtImageFilter::Radius;
	return ret;
}

QWidget *GaussBlurFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;

	l->addWidget(new QLabel(QObject::tr("Blurriness:"), w), 0, 0, 1, 1);
	
	m_slider = new QSlider(w);
	m_slider->setMinimum(0);
	m_slider->setMaximum(99);
	m_slider->setValue(m_radius);
	m_slider->setOrientation(Qt::Horizontal);
	l->addWidget(m_slider, 0, 1, 1, 1);
	
	w->setLayout(l);
	m_slider->setTracking(false);
	connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(handleControlValueChanged()));
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}

void GaussBlurFilter::handleControlValueChanged() {
	
	setOption(QtImageFilter::Radius, m_slider->value());
	
	QtImageFilter::handleControlValueChanged();
}

	
QString GaussBlurFilter::name() const { return QObject::tr("Gaussian Blur"); }

QString GaussBlurFilter::description() const { return QObject::tr("A gaussian blur filter", "GaussBlurFilter"); }
