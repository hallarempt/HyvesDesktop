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
#ifndef SEPIAFILTER_H
#define SEPIAFILTER_H

#include "qtimagefilter.h"
#include <QtCore>
#include <QColor>

const QString SEPIA_ID = "Sepia";

/**
 * sepia or siver oxide effect
 */
class SepiaFilter : public QtImageFilter
{
	Q_OBJECT
public:
	
	SepiaFilter();
	virtual ~SepiaFilter();
	
	QImage apply(const QImage &image, const QRect& clipRect = QRect() );
	
	QString name() const;
	QString description() const;
	QString id() const { return SEPIA_ID; }
	
};
#endif // SEPIAFILTER_H
