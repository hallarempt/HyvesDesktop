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

#ifndef GAUSSBLURFILTER_H
#define GAUSSBLURFILTER_H

#include <QVBoxLayout>
#include <QLabel>
#include <QSlider>
#include <math.h>
#include "convolutionfilter.h"

const QString GAUSSBLUR_ID = "GaussianBlur";

class GaussBlurFilter : public ConvolutionFilter
{
	Q_OBJECT
	
public:
	
	GaussBlurFilter();
	
	////
	// INHERITED FROM ConvolutionFilter
	////
	QVariant option(int option) const;
	bool setOption(int option, const QVariant &value);
	bool supportsOption(int option) const;
	QList<int> options() const;
	
	QImage apply(const QImage &image, const QRect& clipRect );
	
	QString name() const;
	QString description() const;
	QString id() const { return GAUSSBLUR_ID; }
	
	QWidget* controlsWidget(QLabel *previewLabel, QImage originalPreviewImage);
	
public slots:	
	
	virtual void resetOptions();

private slots:
	
	void handleControlValueChanged();
	
private:

	qreal m_radius;
	double m_deviation; // what's called sigma in GraphicsMagick
	QSlider *m_slider;
};
#endif //GAUSSBLURFILTER_H

