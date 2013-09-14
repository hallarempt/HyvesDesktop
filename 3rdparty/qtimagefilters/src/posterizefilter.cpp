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


#include "posterizefilter.h"

#include <math.h>

#include <QtGui>
#include <QtCore>

#include "colorconversions.h"

PosterizeFilter::PosterizeFilter() 
		: QtImageFilter()
		, m_levelsSlider(0)
{
	resetOptions();
}

void PosterizeFilter::resetOptions()
{
	
	m_levels = 2;
	m_mode = PosterizeFilter::CHROMA;
	if (m_levelsSlider) {
		m_levelsSlider->blockSignals(true);
		m_levelsSlider->setValue(m_levels);
		m_levelsSlider->blockSignals(false);
	}
	updateLut();
	
	QtImageFilter::handleControlValueChanged();
}

PosterizeFilter::~PosterizeFilter()
{
}

QImage PosterizeFilter::apply(const QImage& image, const QRect& clipRect)
{
        QMutexLocker locker(&m_mutex);

	
	if (m_lut.isEmpty()) {
		updateLut();
	}		
	
	QImage resultImg = image.convertToFormat(QImage::Format_ARGB32);	
	
	QRect rc = clipRect;
	if (rc.isEmpty()) {
		rc = image.rect();
	}
	m_isAborting = false;
	for (int y = rc.y(); y <= rc.bottom(); ++y) {
		if (m_isAborting) {
			return QImage();
		}
		QRgb* line = reinterpret_cast<QRgb*>(resultImg.scanLine(y));
		
		switch (m_mode) {
		case PosterizeFilter::RGB:
			for (int x = rc.x(); x <= rc.right(); ++x) {
				line[x] = qRgb(m_lut[qRed(line[x])],
					       m_lut[qGreen(line[x])],
					       m_lut[qBlue(line[x])]);
			}
			break;
		case PosterizeFilter::HSV:
			for (int x = rc.x(); x <= rc.right(); ++x) {
				QColor c(line[x]);
				int h, s, v;
				c.getHsv(&h, &s, &v);
				if (h < 0) h = 0;
				h = m_lut[h];
				c.setHsv(h,s,v);
				line[x] = c.rgba();
				
			}
			break;
		case PosterizeFilter::CHROMA:
		case PosterizeFilter::LUMA:
			for (int x = rc.x(); x <= rc.right(); ++x) {
				int y, u, v;
				ColorConversions::rgbToYuv(line[x], &y, &u, &v);
				
				if (m_mode == PosterizeFilter::CHROMA) {
					y = m_lut[y];
				}
				else {
					u = m_lut[u];
					v = m_lut[v];
				}
				
				line[x] = ColorConversions::yuvToRgb(y, u, v);
				
			}
				
		};
		setProgress(int((y - rc.y()) * 100.0 / rc.height()));
	}
	
	return resultImg;
}


QVariant PosterizeFilter::option(int option) const {
	switch (PosterizeFilterOption(option)) {
	case Levels:
		return QVariant(m_levels);
	case Mode:
		return QVariant(m_mode);
	}
	return QVariant();
}

bool PosterizeFilter::setOption(int option, const QVariant &value) {
	switch (PosterizeFilterOption(option)) {
	case Levels:
		m_levels = value.toInt();
		m_levels=qBound(2, (int)m_levels, 25);
		updateLut();
		return true;
	case Mode:
		m_mode = (PosterizeFilterMode)value.toInt();
		updateLut();
		return true;
	}
	return false;
}

bool PosterizeFilter::supportsOption(int option) const {
	switch (PosterizeFilterOption(option)) {
	case Levels:
	case Mode:
		return true;
	}
	return false;
}

QList<int> PosterizeFilter::options() const {
	QList<int> ret;
	ret << PosterizeFilter::Levels << PosterizeFilter::Mode;
	return ret;
}

QString PosterizeFilter::name() const {
	
	return QObject::tr("Posterize");
}

QString PosterizeFilter::description() const {
	
	return QObject::tr("Reduce the number of colours: posterize.");
}

void PosterizeFilter::updateLut() {
	
	m_lut.clear();
	switch(m_mode) {
	case PosterizeFilter::HSV:
		{
			int chunksize = 360 / m_levels;
			int value = 0 ;
			for (int i = 0; i <= m_levels; ++i) {
				value = (i * chunksize);// + (chunksize / 2); 	
				for (int j = 0; j < chunksize; ++j) {
					m_lut  << value;
				}
			}
			m_lut << value;
		}
		break;
	case PosterizeFilter::RGB: 
	case PosterizeFilter::CHROMA:
	case PosterizeFilter::LUMA:
	default:
		{
			int chunksize = 256 / m_levels;
			quint8 value = 0;
			for (int i = 0; i <= m_levels; ++i) {
				value = (i * chunksize) + (chunksize / 2); 	
				for (int j = 0; j < chunksize; ++j) {
					m_lut  << value;
				}
			}
			m_lut << value;
		}
		break;
	}
}

QWidget *PosterizeFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
        l->setMargin(2);
        l->setSpacing(2);
	
	
	m_levelsSlider = new QSlider(w);
	m_levelsSlider->setValue(m_levels);
	m_levelsSlider->setMinimum(2);
	m_levelsSlider->setMaximum(25);
	m_levelsSlider->setOrientation(Qt::Horizontal);
	connect(m_levelsSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));
	l->addWidget(new QLabel(QObject::tr("Levels:"), w), 0, 0, 1, 1);
	l->addWidget(m_levelsSlider, 0, 1, 1, 1);
	/*
	m_groupBox = new QGroupBox(w);
	m_buttonGroup = new QButtonGroup(m_groupBox);
	m_buttonGroup->setExclusive(true);
	
	m_RGB = new QRadioButton(tr("RGB"));
	m_buttonGroup->addButton(m_RGB, PosterizeFilter::RGB);
	m_HSV = new QRadioButton(tr("HSV"));
	m_buttonGroup->addButton(m_HSV, PosterizeFilter::HSV);
	m_CHROMA = new QRadioButton(tr("Color"));
	m_buttonGroup->addButton(m_CHROMA, PosterizeFilter::CHROMA);
	m_LUMA = new QRadioButton(tr("Luminance"));
	m_buttonGroup->addButton(m_LUMA, PosterizeFilter::LUMA);
	
	m_RGB->setChecked(true);
	
	QVBoxLayout *layout = new QVBoxLayout(m_groupBox);
	layout->addWidget(m_RGB);
	layout->addWidget(m_HSV);
	layout->addWidget(m_CHROMA);
	layout->addWidget(m_LUMA);
	
	m_groupBox->setLayout(layout);
	
	connect(m_buttonGroup, SIGNAL(buttonClicked(int)), SIGNAL(handleControlValueChanged()));
	
	l->addWidget(m_groupBox, 1, 0, 2, 1);
	*/
	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}

void PosterizeFilter::handleControlValueChanged() {

	setOption(PosterizeFilter::Levels, m_levelsSlider->value());
	//setOption(PosterizeFilter::Mode, m_buttonGroup->checkedId());
	
	QtImageFilter::handleControlValueChanged();
}
