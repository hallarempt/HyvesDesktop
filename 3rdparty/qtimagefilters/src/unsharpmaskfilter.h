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
#ifndef UNSHARPMASKFILTER_H
#define UNSHARPMASKFILTER_H

#include <QtCore>
#include <QtGui>

#include "qtimagefilter.h"

const QString UNSHARPMASK_ID = "UnsharpMask";

/**
 * Implement unsharp mask aka sharpen with custom radius
 *
 * @see Graphics/ImageMagick's effect.c
 */
class UnsharpMaskFilter : public QtImageFilter
{
	Q_OBJECT

public:
	
	enum UnsharpMaskFilterOption {
		Amount = UserOption + 1900,
		Threshold,
		Radius,
		Sigma
	};

	
	UnsharpMaskFilter();
	virtual ~UnsharpMaskFilter();	
	
	QVariant option(int option) const;
	bool setOption(int option, const QVariant &value);
	bool supportsOption(int option) const;
	QList<int> options() const;

	QImage apply(const QImage &image, const QRect& clipRect = QRect() );
	
	QString name() const;
	QString description() const;
	QString id() const { return UNSHARPMASK_ID; }
	
	QWidget* controlsWidget(QLabel *previewLabel, QImage originalPreviewImage);

public slots:	
	
	virtual void resetOptions();
	

private slots:
	
	void handleControlValueChanged();
		
private:
	
	qreal m_amount;    // difference multiplier
	qreal m_threshold; // scaled to MAX_RGB/2
	qreal m_sigma;     // standard deviation of the gaussian, in pixels
	qreal m_radius;    // The percentage of the difference between the original and the
		   	   // blur image that is added back into the original.

	QDoubleSpinBox *m_amountSpinBox;
	QDoubleSpinBox *m_thresholdSpinBox;
	QDoubleSpinBox *m_sigmaSpinBox;
	QDoubleSpinBox *m_radiusSpinBox;	
	
};

#endif // UnsharpMaskFILTER_H
