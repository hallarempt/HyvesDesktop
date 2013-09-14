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
#ifndef REMOVECOLORFILTER_H
#define REMOVEFILTER_H

#include "qtimagefilter.h"
#include <QtCore>
#include <QtGui>
#include "colorwell.h"

const QString REMOVECOLOR_ID = "RemoveColor";

/**
 * monochromatize the image in the specified color
 *
 * Technically, we take the hue from the given color, and take the saturation and value
 * of the current pixel. 
 *
 * XXX: speed this up tremendously with either a cache or a lookup table.
 */
class RemoveColorFilter : public QtImageFilter
{
	Q_OBJECT

public:
	
	enum RemoveColorFilterOption {
		Color = UserOption + 200,
		Threshold,
		ThresholdRed,
		ThresholdGreen,
		ThresholdBlue,
		SeparateThresholds
		
	};
	
	RemoveColorFilter();
	virtual ~RemoveColorFilter();	
	
	// inherited from QtImageFilter	
	QVariant option(int option) const;
	bool setOption(int option, const QVariant &value);
	bool supportsOption(int option) const;
	QList<int> options() const;
	
	QImage apply(const QImage &image, const QRect& clipRect = QRect() );
	
	QString name() const;
	QString description() const;
	QString id() const { return REMOVECOLOR_ID; }

	QWidget* controlsWidget(QLabel *previewLabel, QImage originalPreviewImage);

public slots:	
	
	virtual void resetOptions();
	

private slots:
	
	void handleControlValueChanged();
	
		
private:
	
	QColor m_color;
	
	int m_threshold;
	
	int m_thresholdRed;
	int m_thresholdGreen;
	int m_thresholdBlue;
	bool m_separateThresholds;
	
	ColorWell *m_colorWell;
	QMap<QString, QSlider*> m_sliders;
	QCheckBox *m_separateThresholdsCheck;
	
};
#endif // RemoveColorFilter_H
