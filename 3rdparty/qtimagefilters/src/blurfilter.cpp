/****************************************************************************
**
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Hyves Desktop, Copyright (C) 2009 Hyves (Startphone Ltd.)
** http://www.hyves.nl/
** 
** Commercial Usage  
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Solutions Commercial License Agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
** 
****************************************************************************/

#include "blurfilter.h"

#include <QtCore>
#include <QtGui>

QVariant BlurFilter::option(int option) const
{
	if (option == QtImageFilter::Radius) return true;
	return ConvolutionFilter::option(option);
}

QImage BlurFilter::apply(const QImage &image, const QRect& clipRect )
{
	if (m_radius > 0.0) {
		m_kernels.clear();
		int uRadius = (int)ceil(m_radius);
		QtMatrix<double> matLeft(2 * uRadius + 1, 1);
		for (int x = -uRadius; x <=uRadius; x++) {
			matLeft.setData(uRadius + x, 0, 1);
		}
		QtMatrix<double> matRight = matLeft.transposed();
		
		QtConvolutionKernelMatrix integerMatrixLeft = convertMatrixBasetype<int,double>(matLeft);
		QtConvolutionKernelMatrix integerMatrixRight = convertMatrixBasetype<int,double>(matRight);
		
		BlurFilter *localThis = const_cast<BlurFilter*>(this);
		localThis->addKernel(integerMatrixLeft, m_channels, m_borderPolicy);
		localThis->addKernel(integerMatrixRight, m_channels, m_borderPolicy);
	}
	return ConvolutionFilter::apply(image, clipRect);
}


QWidget *BlurFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
        l->setMargin(2);
        l->setSpacing(2);


	m_radiusSlider = new QSlider(w);
	m_radiusSlider->setMinimum(1);
	m_radiusSlider->setMaximum(15);
	m_radiusSlider->setValue(m_radius);
	connect(m_radiusSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));
	
	
	l->addWidget(new QLabel(QObject::tr("Blur Radius:"), w), 0, 0, 1, 1);
	l->addWidget(m_radiusSlider, 0, 1, 1, 1);
	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}

void BlurFilter::handleControlValueChanged() {
	
	setOption(QtImageFilter::Radius, m_radiusSlider->value());
	QtImageFilter::handleControlValueChanged();
}
