/*
 * Hyves Desktop, Copyright (C) 2008-2009 Hyves (Startphone Ltd.)
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

#include <QDir>
#include <QUrl>
#include <QtGui>

#include "TextDocument.h"

namespace RichText {

ImageHandler::ImageHandler(TextDocument *doc) : 
	QObject(doc) {
	
	m_defaultImageHandler = doc->documentLayout()->handlerForObject(QTextFormat::ImageObject);
}

QSizeF ImageHandler::intrinsicSize(QTextDocument *doc, int posInDoc, const QTextFormat &format) {

	QTextImageFormat imageFormat = format.toImageFormat();
	QString name = imageFormat.name();
	if (!name.endsWith(".gif")) {
		return m_defaultImageHandler->intrinsicSize(doc, posInDoc, format);
	}
	
	TextDocument *td = qobject_cast<TextDocument *>(doc);
	Q_ASSERT(td);
	QMovie *movie = td->movie(imageFormat.name(), posInDoc);
	return movie->currentImage().size();
}

void ImageHandler::drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDoc, const QTextFormat &format) {
	
	QTextImageFormat imageFormat = format.toImageFormat();
	QString name = imageFormat.name();
	if (!name.endsWith(".gif")) {
		m_defaultImageHandler->drawObject(painter, rect, doc, posInDoc, format);
	} else {
		TextDocument *td = qobject_cast<TextDocument *>(doc);
		Q_ASSERT(td);
		QMovie *movie = td->movie(imageFormat.name(), posInDoc);
		Q_ASSERT(movie);
		painter->drawPixmap(rect.topLeft(), movie->currentPixmap());
	}
}

TextDocument::TextDocument(QObject *parent) :
	QTextDocument(parent) {
	
	documentLayout()->registerHandler(QTextFormat::ImageObject, new ImageHandler(this));
}

QMovie *TextDocument::movie(const QString &remoteName, int position) {
	
	QDir dir(QApplication::applicationDirPath());
	QUrl url(remoteName);
	QString name = dir.path() + "/bundle" + url.path();
	
	QMovie *movie = m_movies[name].movie;
	if (movie) {
		return movie;
	}
	
	movie = new QMovie(this);
	movie->setFileName(name);
	//connect(movie, SIGNAL(finished()), movie, SLOT(start()));
	movie->start();
	connect(movie, SIGNAL(frameChanged(int)), this, SLOT(movieFrameChanged()));
	
	QTextCursor cursor(this);
	cursor.setPosition(position);
	cursor.setPosition(position + 1, QTextCursor::KeepAnchor);
	
	TextDocument::Movie m;
	m.movie = movie;
	m.cursor = cursor;
	m_movies[name] = m;
	
	return movie;
}

void TextDocument::movieFrameChanged() {
	
	QMovie *movie = qobject_cast<QMovie *>(sender());
	Q_ASSERT(movie);
	Q_ASSERT(m_movies.contains(movie->fileName()));
	Movie m = m_movies.value(movie->fileName());
	if (m.cursor.hasSelection()) {
		QTextCharFormat format;
		format.setProperty(QTextFormat::UserFormat + 1, movie->currentFrameNumber());
		m.cursor.mergeCharFormat(format);
	} else {
		movie->stop();
		m_movies.remove(movie->fileName());
		movie->deleteLater();
	}
}

TextDocument::~TextDocument() {
}

} // namespace RichText
