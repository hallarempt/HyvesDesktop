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

#include "texturefilter.h"
#include "qtimagefilter.h"

#include <QtCore>
#include <QtGui>
#include <QImage>

/*
 * Texture filter: Pulls a custom texture over the image
 */

TextureFilter::TextureFilter()
		: QtImageFilter(),
		m_scaleSlider(0),
		m_xOffsetSlider(0),
		m_yOffsetSlider(0)
{
	resetOptions();
}

void TextureFilter::resetOptions()
{
	m_scaleFactor = 0;
	m_xOffset = 0;
	m_yOffset = 0;
	if (m_scaleSlider) {
		m_scaleSlider->setValue(m_scaleFactor);
		m_xOffsetSlider->setValue(m_xOffset);
		m_yOffsetSlider->setValue(m_yOffset);
	}
	
	QtImageFilter::handleControlValueChanged();
}

QVariant TextureFilter::option(int option) const {
	switch (TextureFilterOption(option)) {
		case PatternSvg:
			return QVariant(m_patternFileName);
		case XOffset:
			return QVariant(m_xOffset);
		case YOffset:
			return QVariant(m_yOffset);
		case ScaleFactor:
			return QVariant(m_scaleFactor);
	}
	return QVariant();
}

bool TextureFilter::setOption(int option, const QVariant &value) {
	switch (TextureFilterOption(option)) {
		case PatternSvg:
			m_patternFileName = value.toString();
			return true;
		case XOffset:
			m_xOffset = value.toInt();
			return true;
		case YOffset:
			m_yOffset = value.toInt();
			return true;
		case ScaleFactor:
			m_scaleFactor = value.toInt();
			return true;
	}
	return false;
}

bool TextureFilter::supportsOption(int option) const {
	switch (TextureFilterOption(option)) {
		case PatternSvg:
		case XOffset:
		case YOffset:
		case ScaleFactor:
			return true;
	}
	return false;
}

QList<int> TextureFilter::options() const {
	QList<int> ret;
	ret << TextureFilter::PatternSvg << TextureFilter::XOffset << TextureFilter::YOffset << TextureFilter::ScaleFactor;
	return ret;
}

QImage TextureFilter::apply(const QImage &image, const QRect& clipRect) {

        QMutexLocker locker(&m_mutex);

	Q_UNUSED(clipRect);
	m_isAborting = false;

	QImage resultImg = image.convertToFormat(QImage::Format_ARGB32);

	setProgress(30);
	QPainter painter(&resultImg);
	QImage patternImg(m_patternFileName);
	patternImg = patternImg.scaled(resultImg.size() * (1 + m_scaleFactor / 100.0),
							Qt::KeepAspectRatioByExpanding,
							Qt::SmoothTransformation);
	setProgress(60);
	QRect rect = resultImg.rect();
	int xOffset = (patternImg.width() - rect.width()) / 2 - m_xOffset * rect.width() / 100;
	int yOffset = (patternImg.height() - rect.height()) / 2 + m_yOffset * rect.height() / 100;
	painter.drawImage(rect, patternImg,
						rect.adjusted(xOffset, yOffset, xOffset, yOffset));
	setProgress(99);
	return resultImg;
}

QWidget *TextureFilter::controlsWidget(QLabel *previewLabel, QImage originalPreviewImage) {

	QWidget *w = new QWidget;
	QGridLayout *l = new QGridLayout;
        l->setMargin(2);
        l->setSpacing(2);

	m_scaleSlider = new QSlider(w);
	m_scaleSlider->setValue(m_scaleFactor);
	m_scaleSlider->setMinimum(-50);
	m_scaleSlider->setMaximum(50);
	m_scaleSlider->setOrientation(Qt::Horizontal);
	connect(m_scaleSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));

	m_xOffsetSlider = new QSlider(w);
	m_xOffsetSlider->setValue(m_scaleFactor);
	m_xOffsetSlider->setMinimum(-50);
	m_xOffsetSlider->setMaximum(50);
	m_xOffsetSlider->setOrientation(Qt::Horizontal);
	connect(m_xOffsetSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));

	m_yOffsetSlider = new QSlider(w);
	m_yOffsetSlider->setValue(m_scaleFactor);
	m_yOffsetSlider->setMinimum(-50);
	m_yOffsetSlider->setMaximum(50);
	m_yOffsetSlider->setOrientation(Qt::Vertical);
	connect(m_yOffsetSlider, SIGNAL(valueChanged(int)), SLOT(handleControlValueChanged()));

	l->addWidget(new QLabel(QObject::tr("Zoom:"), w), 0, 0, 1, 1);
	l->addWidget(m_scaleSlider, 0, 1, 1, 2);
	l->addWidget(new QLabel(QObject::tr("Offset:"), w), 1, 0, 1, 1);
	l->addWidget(m_xOffsetSlider, 2, 0, 1, 2);
	l->addWidget(m_yOffsetSlider, 1, 2, 3, 1);

	w->setLayout(l);
	QtImageFilter::controlsWidget(previewLabel, originalPreviewImage);
	return w;
}

void TextureFilter::handleControlValueChanged() {
	setOption(TextureFilter::ScaleFactor, m_scaleSlider->value());
	setOption(TextureFilter::XOffset, m_xOffsetSlider->value());
	setOption(TextureFilter::YOffset, m_yOffsetSlider->value());
	QtImageFilter::handleControlValueChanged();
}
