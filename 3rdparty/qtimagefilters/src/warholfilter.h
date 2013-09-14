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

#ifndef WARHOLFILTER_H
#define WARHOLFILTER_H

#include "qtimagefilter.h"

#include <QtCore>
#include <QtGui>
#include "colorwell.h"

const QString WARHOLD_ID = "Warhol";

/**
 * Warhol Filter: instant popart.
 */
class WarholFilter : public QtImageFilter
{

	Q_OBJECT
	
public:

	// min and max are in the range 0-100	
	enum WarholFilterOption {
		Min = UserOption + 1100,
		Max,
		ColorTopLeft,
		ColorTopRight,
		ColorBottomLeft,
		ColorBottomRight
	};
	
	WarholFilter();
	virtual ~WarholFilter();	
	
	QVariant option(int option) const;
	bool setOption(int option, const QVariant &value);
	bool supportsOption(int option) const;
	QList<int> options() const;
	
	QImage apply(const QImage &image, const QRect& clipRect = QRect() );
	
	QString name() const;
	QString description() const;
	QString id() const { return WARHOLD_ID; }
	
	QWidget* controlsWidget(QLabel *previewLabel, QImage originalPreviewImage);

public slots:	
	
	virtual void resetOptions();
	

private slots:
	
	void handleControlValueChanged();
	
		
private:
	
	quint8 m_min;
	quint8 m_max;
	
	QColor m_colorTopLeft;
	QColor m_colorTopRight;
	QColor m_colorBottomLeft;
	QColor m_colorBottomRight;
	
	QMap<QString, QSlider*> m_sliders;
	QMap<QString, ColorWell*> m_colorWells;
	
};
#endif // WarholFILTER_H
