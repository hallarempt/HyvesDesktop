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

#ifndef THRESHOLDFILTER_H
#define THRESHOLDFILTER_H

#include "qtimagefilter.h"

#include <QtCore>
#include <QColor>
#include <QVector>
#include <QSlider>

const QString THRESHOLD_ID = "Threshold";

/**
 * Threshold Filter: remove all pixels with a gray value below min
 * or above max
 */
class ThresholdFilter : public QtImageFilter
{

	Q_OBJECT
	
public:

	// min and max are in the range 0-100	
	enum ThresholdFilterOption {
		Min = UserOption + 1100,
		Max
	};

	static QRgb threshold(QRgb pixel, quint8 min, quint8 max, QRgb background, QRgb foreground);
	
	ThresholdFilter();
	virtual ~ThresholdFilter();	
	
	QVariant option(int option) const;
	bool setOption(int option, const QVariant &value);
	bool supportsOption(int option) const;
	QList<int> options() const;
	
	QImage apply(const QImage &image, const QRect& clipRect = QRect() );
	
	QString name() const;
	QString description() const;
	QString id() const { return THRESHOLD_ID; }
	
	QWidget* controlsWidget(QLabel *previewLabel, QImage originalPreviewImage);

public slots:	
	
	virtual void resetOptions();	

private slots:
	
	void handleControlValueChanged();
	
		
private:
	
	quint8 m_min;
	quint8 m_max;
	
	QMap<QString, QSlider*> m_sliders;
	
	
};
#endif // ThresholdFILTER_H
