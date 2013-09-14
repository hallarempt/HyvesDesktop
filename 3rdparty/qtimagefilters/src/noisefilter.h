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
#ifndef NOISEFILTER_H
#define NOISEFILTER_H

#include "qtimagefilter.h"
#include <QtCore>
#include <QColor>
#include <QSlider>

#include "noise.h"

const QString NOISE_ID = "Noise";

/**
 * Noise filter: apply noise to pixels
 */
class NoiseFilter : public QtImageFilter
{

	Q_OBJECT
	
public:

	enum NoiseFilterOption {
		NoiseType = UserOption + 1600, // NoiseType unused right now
		Level,
		Opacity
			    
	};

	
	NoiseFilter();
	virtual ~NoiseFilter();	
	
	QVariant option(int option) const;
	bool setOption(int option, const QVariant &value);
	bool supportsOption(int option) const;
	QList<int> options() const;

	
	QImage apply(const QImage &image, const QRect& clipRect = QRect() );
	
	QString name() const;
	QString description() const;
	QString id() const { return NOISE_ID; }
	
	QWidget* controlsWidget(QLabel *previewLabel, QImage originalPreviewImage);

public slots:	
	
	virtual void resetOptions();
	
private slots:
	
	void handleControlValueChanged();
	
	
private:
	
	QSlider *m_levelSlider;
	QSlider *m_opacitySlider;
	
	Noise::NoiseType m_noiseType;
	int m_level;
	quint8 m_opacity;
	

};
#endif // NoiseFILTER_H
