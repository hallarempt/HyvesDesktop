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
#ifndef IMAGESELECTION_H
#define IMAGESELECTION_H

#include <QImage>
#include <QMetaType>

class QPainterPath;

/**
 * Defines a selection mask for QImages. A selection is by definition
 * totally unselected. 
 */
class ImageSelection : public QImage
{
public:
	
	ImageSelection();
	
	ImageSelection(const QSize& size);
	
	/// sets the threshold: above the threshold, a pixel is selected,
	/// below, it's unselected.
	void setSelectionThreshold(quint8 threshold = 128);
	
	quint8 selectionThreshold() { return m_threshold; }
	
	bool isSelected(QPoint pt);
	
	quint8 selectedAt(QPoint pt);
	
	void clear();
	
	void addPainterPath(const QPainterPath& path);
	
	void subtractPainterPath(const QPainterPath& path);
	
	ImageSelection scaled(const QSize& size, Qt::AspectRatioMode mode = Qt::IgnoreAspectRatio) const;
	
private:
	
	quint8 m_threshold;
};

Q_DECLARE_METATYPE(ImageSelection);

#endif // IMAGESELECTION_H
