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
#ifndef COLORIZEFILTER_H
#define COLORIZEFILTER_H

#include "qtimagefilter.h"
#include <QtCore>
#include <QColor>

class ColorWell;

const QString COLORIZE_ID = "Colorize";

/**
 * monochromatize the image in the specified color
 *
 * Technically, we take the hue from the given color, and take the saturation and value
 * of the current pixel. 
 *
 * XXX: speed this up tremendously with either a cache or a lookup table.
 */
class ColorizeFilter : public QtImageFilter
{

	Q_OBJECT
	
public:
	
	enum ColorizeFilterOption {
		Color = UserOption + 200
		
	};
	
	ColorizeFilter();
	virtual ~ColorizeFilter();	
	
	// inherited from QtImageFilter	
	QVariant option(int option) const;
	bool setOption(int option, const QVariant &value);
	bool supportsOption(int option) const;
	QList<int> options() const;
	
	QImage apply(const QImage &image, const QRect& clipRect = QRect() );
	
	QString name() const;
	QString id() const { return COLORIZE_ID; }
	QString description() const;
	
	QWidget* controlsWidget(QLabel *previewLabel, QImage originalPreviewImage);

private slots:
	
	void handleControlValueChanged();

public slots:	
	
	virtual void resetOptions();	
	
private:
	
	ColorWell* m_colorWell;
	QColor m_color;
	
};
#endif // COLORIZEFILTER_H
