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

#ifndef TEXTDOCUMENT_H
#define TEXTDOCUMENT_H

#include <QTextDocument>
#include <QTextCursor>
#include <QHash>
#include <QMovie>

namespace RichText {

/**
 * TextDocument is a QTextDocument that supports animated gif.
 *
 * @author Girish Ramakrishnan <girish@forwardbias.in>
 */
class TextDocument : public QTextDocument {
	
	Q_OBJECT
	
	public:
		TextDocument(QObject *parent = 0);
		~TextDocument();
		
		/*
		 * Creates a movie object with source as filename and
		 * for the position posInDoc.
		 */
		QMovie *movie(const QString &filename, int posInDoc);
		
	private slots:
		void movieFrameChanged();
		
	private:
		struct Movie {
			Movie() : movie(0) { }
			QMovie *movie;
			QTextCursor cursor;
		};
		QHash<QString, Movie> m_movies;
};

} // namespace RichText

#include <QObject>
#include <QTextObjectInterface>

namespace RichText {

/*
 * ImageHandler provides layout information for gif images embedded
 * in the TextDocument.
 *
 * ImageHandler replaces the original ImageHandler (QTextImageHandler)
 * of QTextDocument and takes over processing of all image objects.
 * If the image is not a gif, it proxies the call to the original
 * handler.
 */
class ImageHandler : public QObject, public QTextObjectInterface {
	
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)
    
	public:
		ImageHandler(TextDocument *doc);
		
		QSizeF intrinsicSize(QTextDocument *doc, int posInDoc, const QTextFormat &format);
		
		void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDoc, const QTextFormat &format);
		
	private:
		QTextObjectInterface *m_defaultImageHandler;
};

} // namespace RichText

#endif // TEXTDOCUMENT_H
