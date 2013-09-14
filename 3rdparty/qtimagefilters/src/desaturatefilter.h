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
#ifndef DESATURATEFILTER_H
#define DESATURATEFILTER_H

#include "qtimagefilter.h"
#include <QtCore>
#include <QColor>
#include <QSlider>

#include "colorconversions.h"

const QString DESATURATE_ID = "Desaturate";

class DesaturateFilter : public QtImageFilter
{

	Q_OBJECT
public:
	enum DesaturateFilterOption {
		Desaturation = UserOption + 400
	};

	DesaturateFilter();
	virtual ~DesaturateFilter();

	QVariant option(int option) const;

	bool setOption(int option, const QVariant &value);

	bool supportsOption(int option) const;
	QList<int> options() const;

	QImage apply(const QImage &image, const QRect& clipRect = QRect() );
	QString name() const;
	QString description() const;
	QString id() const { return DESATURATE_ID; }
	
	QWidget* controlsWidget(QLabel *previewLabel, QImage originalPreviewImage);
 
public slots:	
	
	virtual void resetOptions();	

private slots:
	
	void handleControlValueChanged();
	
	
private:
	int m_desaturation;
	QSlider* m_desaturationSlider;
};
#endif // DESTAURATIONFILTER_H
