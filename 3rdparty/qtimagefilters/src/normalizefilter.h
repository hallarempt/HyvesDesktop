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
#ifndef NORMALIZEFILTER_H
#define NORMALIZEFILTER_H

#include "qtimagefilter.h"

class QCheckBox;

const QString NORMALIZE_ID = "Normalize";

class NormalizeFilter : public QtImageFilter
{
	Q_OBJECT
public:

	enum NormalizeFilterOption {
		LimitToRect = UserOption + 300,
		NormalizeIndependently
			      
	};
	
	NormalizeFilter();
	virtual ~NormalizeFilter();	
	
	
	// inherited from QtImageFilter	
	QVariant option(int option) const;
	bool setOption(int option, const QVariant &value);
	bool supportsOption(int option) const;
	QList<int> options() const;
	
	
	QImage apply(const QImage &image, const QRect& clipRect = QRect() );
	
	QString name() const;
	QString description() const;
	QString id() const { return NORMALIZE_ID; }
	
	QWidget* controlsWidget(QLabel *previewLabel, QImage originalPreviewImage);

public slots:	
	
	virtual void resetOptions();

private slots:
	
	void handleControlValueChanged();
	
private:
	
	bool m_limitToRect; // whether the min/max should look at the whole image, or just the clipRect
	QCheckBox *m_limitToRectCheck;
	
	bool m_normalizeChannelsIndependently;
	QCheckBox *m_normalizeChannelsIndependentlyCheck;
};

#endif // NORMALIZEFILTER_H
