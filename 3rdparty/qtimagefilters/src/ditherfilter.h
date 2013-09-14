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
#ifndef DITHERFILTER_H
#define DITHERFILTER_H

#include <QtCore>
#include <QtGui>

#include "qtimagefilter.h"

const QString DITHER_ID = "Dither";

/**
 * Implement dithering in various ways
 * 
 * @see http://www.tinrocket.com/projects/programming/graphics/00158/
 * @see http://mike.teczno.com/notes/atkinson.html
 * @see http://www.verlagmartinkoch.at/software/dither/index.html
 */
class DitherFilter : public QtImageFilter
{
	Q_OBJECT

public:
	
	enum DitherFilterMode {
		Atkinson,
		FloydSteinberg,
		Random
	};
	
	enum DitherFilterOption {
		Mode = UserOption + 1800
	};

	
	DitherFilter();
	virtual ~DitherFilter();	
	
	QVariant option(int option) const;
	bool setOption(int option, const QVariant &value);
	bool supportsOption(int option) const;
	QList<int> options() const;

	QImage apply(const QImage &image, const QRect& clipRect = QRect() );
	
	QString name() const;
	QString description() const;
	QString id() const { return DITHER_ID; }
	
	QWidget* controlsWidget(QLabel *previewLabel, QImage originalPreviewImage);

public slots:	
	
	virtual void resetOptions();	

private slots:
	
	void handleControlValueChanged();
		
private:
	
	bool ditherAtkinson(QImage& src, const QRect& rc);
	bool ditherFloydSteinberg(QImage& src, const QRect& rc);
	bool ditherRandom(QImage& src, const QRect& rc);

	DitherFilterMode m_mode;
	
	QButtonGroup *m_buttonGroup;
	
	QRadioButton *m_atkinson;
	QRadioButton *m_floydSteinberg;
	QRadioButton *m_random;
			
			
	
};

#endif // DITHERFILTER_H
