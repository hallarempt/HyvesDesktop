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
#include "pixelatefilter.h"

#include <QtGui>
#include <QtCore>

#include "colorconversions.h"

PixelateFilter::PixelateFilter()
		: QtImageFilter()
		, m_horizontalSizeSlider(0)
		, m_verticalSizeSlider(0)
{
	resetOptions();
}

void PixelateFilter::resetOptions()
{
	m_horizontalSize = 10;
	m_verticalSize = 10;
	m_isSquare = true;
	m_selection.clear();
	
	if (m_horizontalSizeSlider) {
		m_horizontalSizeSlider->blockSignals(true);
		m_verticalSizeSlider->blockSignals(true);

		m_horizontalSizeSlider->setValue(m_horizontalSize);
		m_verticalSizeSlider->setValue(m_verticalSize);

		m_horizontalSizeSlider->blockSignals(false);
		m_verticalSizeSlider->blockSignals(false);
	}
	
	QtImageFilter::handleControlValueChanged();
}

void PixelateFilter::setThumbnailCreationOptions() {
	m_horizontalSize = 50;
	m_verticalSize = 50;
	m_isSquare = true;
	m_selection.clear();
}

PixelateFilter::~PixelateFilter()
{
}

QImage PixelateFilter::apply(const QImage &_image, const QRect& clipRect)
{
        QMutexLocker locker(&m_mutex);

	QImage image = _image;
	
	bool hasSelection = !m_selection.isNull();
	
	if (image.width() > 800 || image.height() > 800) {
		image = _image.scaled(QSize(800, 800), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}
	
	ImageSelection selection = m_selection;
	if (hasSelection && selection.size() != image.size()) {
		selection = selection.scaled(image.size(), Qt::IgnoreAspectRatio);
	}
	
	QImage resultImg = image.convertToFormat(QImage::Format_ARGB32);	
	
	QRect rc = clipRect;
	if (rc.isEmpty()) {
		rc = image.rect();
	}
	rc &= resultImg.rect();
	
	int horizontalSize = qMax(1, m_horizontalSize * image.width() / 800);
	int verticalSize = qMax(1, m_verticalSize * image.width() / 800);
	m_isAborting = false;
	
	QPainter gc(&resultImg);
	
	for (int y = rc.y(); y <= rc.bottom(); y += verticalSize) {
		if (m_isAborting) {
			return QImage();
		}
		
		QRgb* sel = 0;
		if (hasSelection) sel = reinterpret_cast<QRgb*>(selection.scanLine(y));
		
		for (int x = rc.x(); x <= rc.right(); x += horizontalSize) {
			
			if (hasSelection) {
				if (qRed(sel[x]) < selection.selectionThreshold()) {
					continue;
				}
			}
			QRgb average = resultImg.pixel(x + (horizontalSize / 2), (y + horizontalSize / 2));
			gc.fillRect(x, y, horizontalSize, verticalSize, QColor(average));
			
		}
		setProgress(int((y - rc.y()) * 100.0 / rc.height()));
		
	}
	gc.end();
	
	return resultImg;
	
}

QString PixelateFilter::name() const
{
	return QObject::tr("Pixelate");
}

QString PixelateFilter::description() const
{
	return QObject::tr("Make the image blocky");
}


QVariant PixelateFilter::option(int filteroption) const
{
	switch ((PixelateFilterOption)filteroption) {
	case Selection:
		return QVariant(m_selection);
		break;
	case HorizontalSize:
		return QVariant(m_horizontalSize);
		break;
	case VerticalSize:
		return QVariant(m_verticalSize);
		break;		
	default:
		break;
	}
	return QVariant();
}

bool PixelateFilter::setOption(int filteroption, const QVariant &value)
{
	switch ((PixelateFilterOption)filteroption) {
	case Selection:
		m_selection = value.value<ImageSelection>();
		break;
	case VerticalSize:
		m_verticalSize = value.toInt();
		break;
	case HorizontalSize:
		m_horizontalSize = value.toInt();
	default:
		break;
	}
	return true;
}

bool PixelateFilter::supportsOption(int option) const
{
	bool supports = false;
	switch ((PixelateFilterOption)option) {
	case Selection:
	case HorizontalSize:
	case VerticalSize:
		supports = true;
		break;
	default:
		break;
	}
	return supports;
}

QList<int> PixelateFilter::options() const {
	QList<int> ret;
	ret << QtImageFilter::Selection << PixelateFilter::HorizontalSize << PixelateFilter::VerticalSize;
	return ret;
}

QWidget *PixelateFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {
	
	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
	l->setMargin(2);
	l->setSpacing(2);
	
	l->addWidget(new QLabel(QObject::tr("Block size:"), w), 0, 0, 1, 1);
	m_horizontalSizeSlider = new QSlider(w);
	m_horizontalSizeSlider->setOrientation(Qt::Horizontal);
	m_horizontalSizeSlider->setValue(m_horizontalSize);
	m_horizontalSizeSlider->setMaximum(30);
	m_horizontalSizeSlider->setMinimum(1);
	l->addWidget(new QLabel(QObject::tr("Horizontal:"), w), 1, 0, 1, 1);
	l->addWidget(m_horizontalSizeSlider, 1, 1, 1, 1);
	connect(m_horizontalSizeSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));
	
	m_verticalSizeSlider = new QSlider(w);
	m_verticalSizeSlider->setOrientation(Qt::Horizontal);
	m_verticalSizeSlider->setValue(m_verticalSize);
	m_verticalSizeSlider->setMaximum(30);
	m_verticalSizeSlider->setMinimum(1);
	l->addWidget(new QLabel(QObject::tr("Vertical:"), w), 2, 0, 1, 1);
	l->addWidget(m_verticalSizeSlider, 2, 1, 1, 1);
	connect(m_verticalSizeSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));
	
	QHBoxLayout *l2 = new QHBoxLayout;
	m_isSquareCheckBox = new QCheckBox;
	m_isSquareCheckBox->setChecked(m_isSquare);
	l2->addWidget(m_isSquareCheckBox);
	l2->addWidget(new QLabel(QObject::tr("Square blocks")));
	l->addLayout(l2, 3, 0, 1, 1);
	connect(m_isSquareCheckBox, SIGNAL(stateChanged(int)), this, SLOT(handleControlValueChanged()));
	
	QLabel *label = new QLabel(tr("Add shapes to select the area to pixelate"), w);
	label->setWordWrap(true);
	l->addWidget(label, 4, 0, 1, 2);
	
	m_ellipseButton = new QToolButton(w);
	m_ellipseButton->setFixedSize(48, 48);
	QIcon icon;
	icon.addPixmap(QPixmap(":/qtimagefilters/ellipse.png"), QIcon::Normal);
	icon.addPixmap(QPixmap(":/qtimagefilters/ellipse-active.png"), QIcon::Active);
	m_ellipseButton->setIcon(icon);
	connect(m_ellipseButton, SIGNAL(clicked()), SIGNAL(addEllipseToSelection()));
	l->addWidget(m_ellipseButton, 5, 0, 2, 1);
	
	m_rectButton = new QToolButton(w);
	m_rectButton->setFixedSize(48, 48);
	QIcon icon2;
	icon2.addPixmap(QPixmap(":/qtimagefilters/rectangle.png"), QIcon::Normal);
	icon2.addPixmap(QPixmap(":/qtimagefilters/rectangle-active.png"), QIcon::Active);
	m_rectButton->setIcon(icon2);
	connect(m_rectButton, SIGNAL(clicked()), SIGNAL(addRectToSelection()));
	l->addWidget(m_rectButton, 5, 1, 2, 1);
	
	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
	
}

void PixelateFilter::handleControlValueChanged() {
	
	int horizontal = m_horizontalSizeSlider->value();
	int vertical = m_verticalSizeSlider->value();
	setOption(PixelateFilter::HorizontalSize, horizontal);
	m_isSquare = m_isSquareCheckBox->isChecked();
	if (m_isSquare) {
		setOption(PixelateFilter::VerticalSize, horizontal);
		m_verticalSizeSlider->blockSignals(true);
		m_verticalSizeSlider->setValue(horizontal);
		m_verticalSizeSlider->blockSignals(false);
	} else {
		setOption(PixelateFilter::VerticalSize, vertical);
	}
	m_verticalSizeSlider->setEnabled(!m_isSquare);
	
	QtImageFilter::handleControlValueChanged();
}

bool PixelateFilter::hasSelection() const {
	return !m_selection.isNull();
}
