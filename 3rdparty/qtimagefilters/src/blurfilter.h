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

#ifndef BLURFILTER_H
#define BLURFILTER_H

#include <math.h>
#include <QSlider>
#include "convolutionfilter.h"

const QString BLUR_ID = "Blur";

/* BlurFilter: Blur image based on a simple mean of adjacent pixels
 * For blur based on Gaussian distribution, use GaussianBlurFilter
 */

class BlurFilter : public ConvolutionFilter
{
	Q_OBJECT
	
public:
	BlurFilter() : ConvolutionFilter(), m_radiusSlider(0) { resetOptions(); }
	
	////
	// INHERITED FROM ConvolutionFilter
	////
	QVariant option(int option) const;
	
	bool setOption(int option, const QVariant &value)
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
	
	bool supportsOption(int option) const
	{
		if (option == QtImageFilter::Radius) return true;
		return ConvolutionFilter::supportsOption(option);
	}
	
	QImage apply(const QImage &image, const QRect& clipRect );
	
	QString name() const { return QObject::tr("Blur"); }
	QString description() const { return QObject::tr("A uniform blur filter", "BlurFilter"); }
	QString id() const { return BLUR_ID; }
	
	QWidget* controlsWidget(QLabel *previewLabel, QImage originalPreviewImage);
	
public slots:	
	
	virtual void resetOptions() {
		
		m_radius = 4.0;
		if (m_radiusSlider) {
			m_radiusSlider->blockSignals(true);
			m_radiusSlider->setValue(m_radius);
			m_radiusSlider->blockSignals(false);
		}
	}
	
private slots:
	
	void handleControlValueChanged();
	
private:
	
	QSlider* m_radiusSlider;
	qreal m_radius;
};

#endif //BLURFILTER_H

