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
#ifndef OILPAINTFILTER_H
#define OILPAINTFILTER_H

#include "qtimagefilter.h"
#include <QtCore>
#include <QColor>
#include <QSlider>

const QString OILPAINT_ID = "Oilpaint";

/**
 * Oilpaint Filter: take the set a pixel to the most-used color surrounding the pixel. 
 *
 * @see http://www.jasonwaltman.com/thesis/filter-oil.html
 */
class OilpaintFilter : public QtImageFilter
{

	Q_OBJECT
	
public:

	enum OilpaintFilterOption {
		BrushSize = UserOption + 600,
		Smooth
	};

	
	OilpaintFilter();
	virtual ~OilpaintFilter();	
	
	QVariant option(int option) const;
	bool setOption(int option, const QVariant &value);
	bool supportsOption(int option) const;
	QList<int> options() const;

	
	QImage apply(const QImage &image, const QRect& clipRect = QRect() );
	
	/**
	 * Figure out which color is the most frequent in a square around a certain point.
	 *
	 * @param src the source QImage
	 * @param bounds the area in which we are working
	 * @param center the center of the brush
	 * @param brushSize the radius around the center
	 * @param smooth the number of intensities we distinguish
	 */
	QRgb mostFrequentColor(const QImage& src, const QRect& bounds, const QPoint center, int brushSize, int smooth);

	QString name() const;
	QString description() const;
	QString id() const { return OILPAINT_ID; }
	
	QWidget* controlsWidget(QLabel *previewLabel, QImage originalPreviewImage);
	void setThumbnailCreationOptions();

public slots:	
	
	virtual void resetOptions();

private slots:
	
	void handleControlValueChanged();
	
		
private:
	
	QSlider *m_brushSizeSlider;
	QSlider *m_smoothSlider;

	int m_brushSize;
	int m_smooth;
	// cached values are used to optimize mostFrequentColor()
	int m_cachedWindowSize, m_cachedSmooth;
	QImage m_cachedImage;
	QVector<quint32> m_averageRed, m_averageGreen, m_averageBlue, m_intensityCount;
	int m_cachedStartX, m_cachedStartY;
	
};
#endif // OILPAINTFILTER_H
