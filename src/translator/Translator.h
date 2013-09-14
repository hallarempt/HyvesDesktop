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

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QObject>

#include "hyveslib_export.h"

namespace Translator {

class HYVESLIB_EXPORT Translator : public QObject {
	
	Q_OBJECT
	
	public:
		static Translator *instance();
		static void destroy();
		
	public slots:
		/**
		 * Translates the given @p string.
		 * 
		 * @return The translated string. 
		 */
		QString tr(const QString &string);
		
		/**
		 * Returns the ISO 639-2 code for the current language, like ENG or NLD.
		 */
		QString iso6392Language() const;
		
		/**
		 * Returns the language code as a combination of language and locale,
		 * like en_GB or nl_NL.
		 */
		QString locale() const;
		
		/**
		 * Set the language of the application, plugins and dynamically
		 * loaded .ui files to the language by their ISO 639-2 code.
		 *
		 * All language files in the install dir of the application will be loaded,
		 * one after another, in an unspecified order.
		 *
		 * This _will_ lead to a flurry of change events and flickering.
		 *
		 * Note: in the future, will make language files bundling with plugins and
		 * content bundles possible.
		 */
		void setLanguage(const QString &iso6392Language);

		/**
		 * Returns the translated rich text in html
		 */
		QString trHtml(const QString& string);

		/**
		 * Returns the javascript code that can add the
		 * translated rich text as leading children of jsObject
		 */
		QString trHtmlJs(const QString& string, const QString& jsObject);
		
		/**
		 * Returns the accessKey for the translated string.
		 * Returns null if there isn't any accessKey.
		 */
		QString trAccessKey(const QString& string);

	private:
		static Translator *s_instance;
		
		struct Private;
		Private *const m_d;
		
		Translator();
		virtual ~Translator();
};

} // namespace Translator

#endif // TRANSLATOR_H
