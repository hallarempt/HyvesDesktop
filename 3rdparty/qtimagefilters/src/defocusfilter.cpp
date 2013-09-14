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

#include "defocusfilter.h"

QImage DefocusFilter::apply(const QImage &_image, const QRect& clipRect )
{
	QImage image = _image;
	if (image.width() > 800 || image.height() > 800) {
		image = _image.scaled(QSize(800, 800), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}

	m_kernels.clear();
	static int kernelmatrix[ 9 ] =
	{ 1,  1, 1,
	  1, -7, 1,
	  1,  1, 1 };
	DefocusFilter *localThis = const_cast<DefocusFilter*>(this);
	localThis->addKernel(QtConvolutionKernelMatrix(kernelmatrix, 3, 3), ConvolutionFilter::RGB, ConvolutionFilter::Extend);
	return ConvolutionFilter::apply(image, clipRect);
}
