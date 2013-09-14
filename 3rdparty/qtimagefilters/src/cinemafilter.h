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
#ifndef CINEMAFILTER_H
#define CINEMAFILTER_H

#include "qtimagefilter.h"
#include <QtCore>
#include <QColor>
#include <QCheckBox>
#include <QSlider>

const QString CINEMA_ID = "Cinema";

/*
 * Cinema filter: Changes the image to look as if it's captured from a movie
 * Loosely based on: http://ebin.wordpress.com/2007/03/21/how-to-turn-your-photo-into-movie-like-effect-using-photoshop/
 */
class CinemaFilter : public QtImageFilter
{
	Q_OBJECT
public:
	enum CinemaFilterOption {
		Darkness = UserOption + 500,
		Contrast,
		Noise,
		FocusRegionSize,
		SpotlightSize,
		Blur,
		Desaturation,
		Cinemascope
	};

	CinemaFilter() : QtImageFilter() 
		, m_desatFilter(0)
		, m_blurFilter(0)
		, m_cinemascopeCheckBox(0)
	{
		m_cachedImage = QImage();
		m_cachedDesaturation = -1;
		m_cachedBlur = -1;
		setThreadedPreviewEnabled(true);
		resetOptions();
	}

	QVariant option(int option) const;
	bool setOption(int option, const QVariant &value);
	bool supportsOption(int option) const;
	QList<int> options() const;
	QImage apply(const QImage &image, const QRect& clipRect = QRect());

	QString name() const { return QObject::tr("Movie Still"); }
	QString description() const { return QObject::tr("A Movie-like effect"); }
	QString id() const { return CINEMA_ID; }

	QWidget* controlsWidget(QLabel *previewLabel, QImage originalPreviewImage);

public slots:	
	
	virtual void resetOptions() {
	
		m_cinemascope = false;
		m_desaturation = 73,
		m_blur = 30;
		m_darkness = 46;
		m_contrast = 80;
		m_focusRegionSize = 35;
		m_spotlightSize = 40;
		m_desatImageMean = 0;
		m_noise = 10;

		if (m_cinemascopeCheckBox) {
			m_cinemascopeCheckBox->blockSignals(true);
			QStringList sliderNames;
			sliderNames << "Desaturate" << "Blur" << "Darken" << "Contrast" << "Focus" << "Spotlight" << "Noise";
			foreach(QString sn, sliderNames) m_sliders[sn]->blockSignals(true);

			m_cinemascopeCheckBox->setChecked(m_cinemascope);
			m_sliders["Desaturate"]->setValue(m_desaturation);
			m_sliders["Blur"]->setValue(m_blur);
			m_sliders["Darken"]->setValue(m_darkness);
			m_sliders["Contrast"]->setValue(m_contrast);
			m_sliders["Focus"]->setValue(m_focusRegionSize);
			m_sliders["Spotlight"]->setValue(m_spotlightSize);
			m_sliders["Noise"]->setValue(m_noise);

			m_cinemascopeCheckBox->blockSignals(false);
			foreach(QString sn, sliderNames) m_sliders[sn]->blockSignals(false);
		}		
		QtImageFilter::handleControlValueChanged();

	}

private slots:

	void handleControlValueChanged();

private:
	int m_darkness;
	int m_contrast;
	int m_noise;
	int m_focusRegionSize;
	int m_blur;
	int m_desaturation;
	int m_spotlightSize;
	QImage m_cachedImage; // store last input image to determine whether we need to redo desaturation or not
	int m_cachedDesaturation, m_cachedBlur;
	QImage m_desatImage;
	QImage m_desatBlurredImage;
	int m_desatImageMean;
	bool m_cinemascope;
	QtImageFilter *m_desatFilter;
	QtImageFilter *m_blurFilter;
	QCheckBox *m_cinemascopeCheckBox;
	QMap<QString, QSlider*> m_sliders;
};
#endif // CINEMAFILTER_H
