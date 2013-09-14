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

#ifndef DEFOCUSFILTER_H
#define DEFOCUSFILTER_H

#include <math.h>
#include <QSlider>
#include "convolutionfilter.h"

const QString DEFOCUS_ID = "Defocus";

/* DefocusFilter: Defocus image
 */

class DefocusFilter : public ConvolutionFilter
{
	Q_OBJECT
	
public:
	DefocusFilter() : ConvolutionFilter() {}
	QString name() const { return QObject::tr("Defocus"); }
	QString description() const { return QObject::tr("Defocus", "Defocus"); }
	QString id() const { return DEFOCUS_ID; }
	QImage apply(const QImage &image, const QRect& clipRect );
};

#endif //DefocusFilter_H

