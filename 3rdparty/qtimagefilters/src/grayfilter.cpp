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

#include "grayfilter.h"


#include <QtGui>
#include <QtCore>

GrayFilter::GrayFilter()
	: QtImageFilter()
{
}

GrayFilter::~GrayFilter()
{
}

void GrayFilter::resetOptions() {
	
	m_selection = ImageSelection();
	QtImageFilter::handleControlValueChanged();
	
}

QImage GrayFilter::apply(const QImage& image, const QRect& clipRect)
{
        QMutexLocker locker(&m_mutex);

	QImage resultImg = image.convertToFormat(QImage::Format_ARGB32);	
	
	QRect rc = clipRect;
	if (rc.isEmpty()) {
		rc = image.rect();
	}
	m_isAborting = false;
	
	bool hasSelection = !m_selection.isNull();
	ImageSelection selection = m_selection;
	if (hasSelection && selection.size() != image.size()) {
		selection = selection.scaled(image.size(), Qt::IgnoreAspectRatio);
	}
	
	for (int y = rc.y(); y <= rc.bottom(); ++y) {
		if (m_isAborting) {
			return QImage();
		}
		
		QRgb* sel = 0;
		if (hasSelection) sel = reinterpret_cast<QRgb*>(selection.scanLine(y));

		QRgb* line = reinterpret_cast<QRgb*>(resultImg.scanLine(y));
		for (int x = rc.x(); x <= rc.right(); ++x) {
			
			if (hasSelection) {
				if (qRed(sel[x]) > selection.selectionThreshold()) {
					continue;
				}
			}

			
			int gray = qGray(line[x]);
			int alpha = qAlpha(line[x]);
			line[x] = qRgba(gray, gray, gray, alpha);
		}
		setProgress(int((y - rc.y()) * 100.0 / rc.height()));
	}

	return resultImg;
}

QString GrayFilter::name() const {
	
	return QObject::tr("Nine filter");
}

QString GrayFilter::description() const {
	
	return QObject::tr("Make (partially) gray: selected areas will stay colored");
}

QVariant GrayFilter::option(int filteroption) const
{
	switch ((QtImageFilter::FilterOption)filteroption) {
	case Selection:
		return QVariant(m_selection);
		break;
	default:
		break;
	}
	return QVariant();
}

bool GrayFilter::setOption(int filteroption, const QVariant &value)
{
	switch ((QtImageFilter::FilterOption)filteroption) {
	case Selection:
		m_selection = value.value<ImageSelection>();
		break;
	default:
		break;
	}
	return true;
}

bool GrayFilter::supportsOption(int option) const
{
	bool supports = false;
	switch ((QtImageFilter::FilterOption)option) {
	case Selection:
		supports = true;
		break;
	default:
		break;
	}
	return supports;
}

QList<int> GrayFilter::options() const {
	QList<int> ret;
	ret << QtImageFilter::Selection;
	return ret;
}

QWidget *GrayFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {

	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
        l->setMargin(2);
        l->setSpacing(2);

	QLabel *label = new QLabel(tr("Add selection shapes to keep parts colored"), w);
	label->setWordWrap(true);
	l->addWidget(label, 0, 0, 0, 2);
	
	m_ellipseButton = new QToolButton(w);
	m_ellipseButton->setFixedSize(48, 48);
	QIcon icon;
	icon.addPixmap(QPixmap(":/qtimagefilters/ellipse.png"), QIcon::Normal);
 	icon.addPixmap(QPixmap(":/qtimagefilters/ellipse-active.png"), QIcon::Active);
	m_ellipseButton->setIcon(icon);
	connect(m_ellipseButton, SIGNAL(clicked()), SIGNAL(addEllipseToSelection()));
	l->addWidget(m_ellipseButton, 1, 0, 1, 1);

	m_rectButton = new QToolButton(w);
	m_rectButton->setFixedSize(48, 48);
	QIcon icon2;
	icon2.addPixmap(QPixmap(":/qtimagefilters/rectangle.png"), QIcon::Normal);
 	icon2.addPixmap(QPixmap(":/qtimagefilters/rectangle-active.png"), QIcon::Active);
	m_rectButton->setIcon(icon2);
	connect(m_rectButton, SIGNAL(clicked()), SIGNAL(addRectToSelection()));
	l->addWidget(m_rectButton, 1, 1, 1, 1);
	
	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;

}

bool GrayFilter::hasSelection() const {
	return !m_selection.isNull();
}
