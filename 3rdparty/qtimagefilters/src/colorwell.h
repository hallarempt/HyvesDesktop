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
#ifndef COLORWELL_H
#define COLORWELL_H

#include <QToolButton>

class ColorWell : public QToolButton
{
	
	Q_OBJECT
	
public:
    
	ColorWell(QWidget* parent = 0);
	virtual ~ColorWell();

	QColor selectedColor();
	void setColor(QColor color);
	
public slots:
	
	void showMoreColorsDialog();

signals:
	
	void colorChanged(QColor color);

	
private slots:

	void updateIcon();
	
private:
			
	QColor m_color;

    
    
};

#endif // COLORWELL_H
