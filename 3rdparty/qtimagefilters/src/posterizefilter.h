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

#ifndef POSTERIZEFILTER_H
#define POSTERIZEFILTER_H

#include "qtimagefilter.h"

#include <QtCore>
#include <QColor>
#include <QVector>
#include <QSlider>
#include <QRadioButton>
#include <QGroupBox>
#include <QButtonGroup>

const QString POSTERIZE_ID = "Posterize";

/**
 * Posterize Filter: set a pixel to weighted random color from the colors
 * surrounding the pixel. 
 *
 * @see http://www.jasonwaltman.com/thesis/filter-frost.html
 */
class PosterizeFilter : public QtImageFilter
{

	Q_OBJECT
	
public:

	enum PosterizeFilterMode {
		RGB,
		HSV,
		CHROMA,
		LUMA
	};
	
	enum PosterizeFilterOption {
		Levels = UserOption + 800,
		Mode
	};

	
	PosterizeFilter();
	virtual ~PosterizeFilter();	
	
	QVariant option(int option) const;
	bool setOption(int option, const QVariant &value);
	bool supportsOption(int option) const;
	QList<int> options() const;

	QImage apply(const QImage &image, const QRect& clipRect = QRect() );
	
	QString name() const;
	QString description() const;
	QString id() const { return POSTERIZE_ID; }	
	
	QWidget* controlsWidget(QLabel *previewLabel, QImage originalPreviewImage);

public slots:	
	
	virtual void resetOptions();

private slots:
	
	void handleControlValueChanged();
	
		
private:
	
	void updateLut();

	qint8 m_levels;
	PosterizeFilterMode  m_mode;
	
	QSlider *m_levelsSlider;
	QButtonGroup *m_buttonGroup;
	QGroupBox *m_groupBox;
	QRadioButton *m_RGB;
	QRadioButton *m_HSV;
	QRadioButton *m_CHROMA;
	QRadioButton *m_LUMA;
	
	QVector<quint8> m_lut;
	
};
#endif // PosterizeFILTER_H
