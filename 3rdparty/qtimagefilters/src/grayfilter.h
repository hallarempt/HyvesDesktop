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
#ifndef GRAYFILTER_H
#define GRAYFILTER_H

#include "qtimagefilter.h"

#include <QtCore>
#include <QtGui>

#include "imageselection.h"

const QString GRAY_ID = "Gray";

/**
 * monochromatize the image in the specified color
 */
class GrayFilter : public QtImageFilter
{
		
	Q_OBJECT
	
public:
	
	GrayFilter();
	virtual ~GrayFilter();	
	
	QImage apply(const QImage &image, const QRect& clipRect = QRect() );
	
	QString name() const;
	QString description() const;
	QString id() const { return GRAY_ID; }
	
	// inherited from QtImageFilter	
	QVariant option(int option) const;
	bool setOption(int option, const QVariant &value);
	bool supportsOption(int option) const;
	QList<int> options() const;
	
	QWidget* controlsWidget(QLabel *previewLabel, QImage originalPreviewImage);
	bool hasSelection() const;

public slots:
	
	void resetOptions();
	
private:

	ImageSelection m_selection;
	
	QAbstractButton *m_ellipseButton;
	QAbstractButton *m_rectButton;
	
};
#endif // GRAYFILTER_H
