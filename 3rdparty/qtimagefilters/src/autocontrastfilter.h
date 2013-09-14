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
#ifndef AUTOCONTRASTFILTER_H
#define AUTOCONTRASTFILTER_H

#include "qtimagefilter.h"
#include <QImage>
#include <QRect>
#include <QString>

const QString AUTOCONTRAST_ID = "Autocontrast";

class QRadioButton;

class AutoContrastFilter : public QtImageFilter
{
	
	Q_OBJECT
public:

	enum AutoContrastMode {
		RGB = 0,
		HSV
	};
	
	enum AutoContrastFilterOption {
		Mode = UserOption + 1500
	};
	
	
	AutoContrastFilter();
	virtual ~AutoContrastFilter();
	
	/// call this first if you want to use applyPixel
	void init(const QImage& image, const QRect& clipRect);
	
	/// stretch the contrast for the pixel at the give position. init() needs to have been called first
	QRgb applyPixel(QRgb color);
	
	QImage apply(const QImage &image, const QRect& clipRect = QRect() );
	
	QString id() const { return AUTOCONTRAST_ID; }
	QString name() const;
	QString description() const;
	
	QVariant option(int option) const;
	bool setOption(int option, const QVariant &value);
	bool supportsOption(int option) const;
	QList<int> options() const;
	
	QWidget* controlsWidget(QLabel *previewLabel, QImage originalPreviewImage);
	
public slots:	
	
	virtual void resetOptions();	
	
private slots:
	
	void handleControlValueChanged();
	
private:
	
	AutoContrastMode m_mode;
	
	QVector<quint8> m_lutRed;
	QVector<quint8> m_lutGreen;
	QVector<quint8> m_lutBlue;
	
	int m_rMin, m_gMin, m_bMin;
	int m_rMax, m_gMax, m_bMax;
	
	qreal m_sMin, m_vMin;
	qreal m_sMax, m_vMax;
	
	QRadioButton* m_rgb;
	QRadioButton* m_hsv;
		
};

#endif // AUTOCONTRASTFILTER_H
