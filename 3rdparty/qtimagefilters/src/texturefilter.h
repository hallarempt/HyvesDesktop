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
#ifndef TEXTUREFILTER_H
#define TEXTUREFILTER_H

#include "qtimagefilter.h"
#include <QtCore>
#include <QColor>
#include <QCheckBox>
#include <QSlider>


const QString TEXTURE_ID = "Texture";
const QString JIGSAW_ID = "Jigsaw";
const QString POLAROID_ID = "PolaroidCollage";

/*
 * Texture filter: Changes the image to look as if it's captured from a movie
 * Loosely based on: http://ebin.wordpress.com/2007/03/21/how-to-turn-your-photo-into-movie-like-effect-using-photoshop/
 */
class TextureFilter : public QtImageFilter
{
	
	Q_OBJECT
	
public:
	enum TextureFilterOption {
		PatternSvg = UserOption + 1700,
		XOffset,
		YOffset,
		ScaleFactor
	};

	TextureFilter();

	QVariant option(int option) const;
	bool setOption(int option, const QVariant &value);
	bool supportsOption(int option) const;
	QList<int> options() const;
	QImage apply(const QImage &image, const QRect& clipRect = QRect());

	QString id() const { return TEXTURE_ID; }
	QString name() const { return QObject::tr("Texture"); }
	QString description() const { return QObject::tr("Overlays a texture on the image"); }

	QWidget *controlsWidget(QLabel *previewLabel, QImage originalPreviewImage);
	void handleControlValueChanged();

public slots:
	virtual void resetOptions();

private:
	QString m_patternFileName;
	int m_xOffset, m_yOffset; // as percentages [-50, 50]
	int m_scaleFactor; // as percentage [-50, 50]
	QSlider *m_scaleSlider, *m_xOffsetSlider, *m_yOffsetSlider;
};

class JigsawFilter : public TextureFilter
{
public:
	JigsawFilter() : TextureFilter() {
		setOption(TextureFilter::PatternSvg, ":/qtimagefilters/textures/jigsaw.svg");
	}
	QString name() const { return QObject::tr("Jigsaw"); }
	QString description() const { return QObject::tr("A solved jigsaw puzzle"); }
	QString id() const { return TEXTURE_ID; }
};

class PolaroidCollageFilter : public TextureFilter
{
public:
	PolaroidCollageFilter() : TextureFilter() {
		setOption(TextureFilter::PatternSvg, ":/qtimagefilters/textures/polaroidcollage.png");
	}
	QString name() const { return QObject::tr("Polaroid Collage"); }
	QString description() const { return QObject::tr("A collage of Polaroid photos"); }
	QString id() const { return POLAROID_ID; }
};
#endif // TEXTUREFILTER_H
