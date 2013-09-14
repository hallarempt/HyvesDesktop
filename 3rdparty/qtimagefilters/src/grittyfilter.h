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
#ifndef GRITTYFILTER_H
#define GRITTYFILTER_H

#include "qtimagefilter.h"
#include <QtCore>
#include <QColor>
#include <QSlider>
#include "colorwell.h"
#include "colorconversions.h"


const QString GRITTY_ID = "Gritty";

/**
 * Gritty Filter: give a draganized look to the image
 *
 * @see: http://photoshopfrenzy.com/?p=94
 */
class GrittyFilter : public QtImageFilter
{

	Q_OBJECT
	
public:
	
	enum GrittyFilterOption {
		Desaturation = UserOption + 900,
		Contrast,
		Darkness
	};

	
	GrittyFilter();
	virtual ~GrittyFilter();	
	
	QVariant option(int option) const;
	bool setOption(int option, const QVariant &value);
	bool supportsOption(int option) const;
	QList<int> options() const;
	
	QImage apply(const QImage &image, const QRect& clipRect = QRect() );
	
	QString name() const;
	QString description() const;
	QString id() const { return GRITTY_ID; }
	
	QWidget* controlsWidget(QLabel *previewLabel, QImage originalPreviewImage);

public slots:	
	
	virtual void resetOptions();
	
private slots:
	
	void handleControlValueChanged();
			
private:
	QImage doHighPass(QImage src, QRect rc);
	
	int m_desaturation;
	int m_contrast;
	int m_darkness;
	QVector<quint16> m_curve; 
	
	QSlider* m_desaturationSlider;
	QSlider* m_contrastSlider;
	QSlider* m_darknessSlider;
};
#endif // GrittyFILTER_H
