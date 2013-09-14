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

#ifndef UTIL_H
#define UTIL_H

#include <QDomDocument>
#include <QMap>
#include <QString>
#include <QVariant>

#include "hyveslib_export.h"

class HYVESLIB_EXPORT Util {
	
	public:
		/**
		 * Takes a QVariantMap and converts it to JSON syntax.
		 */
		static QString variantMapToJSON(const QVariantMap &map);
		
		/**
		 * Takes a QVariantList and converts it to JavaScript syntax.
		 */
		static QString variantListToJSType(const QVariantList &list);
		
		/**
		 * Takes a QVariant and converts it to JavaScript syntax.
		 *
		 * Currently the following types are supported: bool, int, qlonglong,
		 * QVariantList, QVariantMap, QRect, QString, QUrl, and QDomDocument.
		 */
		static QString variantToJSType(QVariant variant);
		
		/**
		 * Takes a QDomElement and converts it to JSON syntax.
		 */
		static QString elementToJSON(QDomElement element);
		
		/**
		 * Takes a string and escapes it using JavaScript syntax.
		 */
		static QString jsStringEscape(const QString &string);
		
		/**
		 * Takes a JSON string and converts it to a QVariantMap.
		 */
		static QVariantMap jsonToVariantMap(const QString &json, bool *error = 0);
		
		/**
		 * Takes a platform independent path to a library file, ending with a
		 * .library extension, and converts it to a platform dependent path.
		 */
		static QString platformPath(const QString &path);
};

#endif // UTIL_H
