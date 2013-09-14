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

#include <QDebug>
#include <QDomElement>
#include <QDomNode>
#include <QRect>
#include <QStringList>
#include <QUrl>
#include <QModelIndex>

#include "json_driver.hh"

#include "extender/MetaTypes.h"
#include "logger/Logger.h"
#include "Util.h"

QString Util::variantMapToJSON(const QVariantMap &map) {
	
	QString json;
	
	for (QVariantMap::ConstIterator it = map.begin(); it != map.end(); ++it) {
		if (!json.isEmpty()) {
			json += ", ";
		}
		json += jsStringEscape(it.key()) + ": " + variantToJSType(it.value());
	}
	
	return "{ " + json + " }";
}

QString Util::variantListToJSType(const QVariantList &list) {
	
	QString jsType;
	
	for (QVariantList::ConstIterator it = list.begin(); it != list.end(); ++it) {
		if (!jsType.isEmpty()) {
			jsType += ", ";
		}
		jsType += variantToJSType(*it);
	}
	
	return "[ " + jsType + " ]";
}

QString Util::variantToJSType(QVariant variant) {
	
	if (variant.type() == QVariant::Bool) {
		return (variant.toBool() ? "true" : "false");
	} else if (variant.type() == QVariant::Int) {
		return QString::number(variant.toInt());
	} else if (variant.type() == QVariant::List) {
		return variantListToJSType(variant.toList());
	} else if (variant.type() == QVariant::LongLong) {
		return QString::number(variant.toLongLong());
	} else if (variant.type() == QVariant::Map) {
		return variantMapToJSON(variant.toMap());
	} else if (variant.type() == QVariant::Rect) {
		QRect rect = variant.toRect();
		return "[ " + QString::number(rect.x()) + ", " + QString::number(rect.y()) + ", "
		            + QString::number(rect.width()) + ", " + QString::number(rect.height()) + " ]";
	} else if (variant.type() == QVariant::String) {
		return jsStringEscape(variant.toString());
	} else if (variant.type() == QVariant::Url) {
		return jsStringEscape(variant.toUrl().toString());
	} else if (variant.isNull()) {
		return "null";
	} else {
		Q_ASSERT(variant.typeName() != 0);
		QString typeName = variant.typeName();
		if (typeName == "QDomDocument") {
			QDomNode rootElement = variant.value<QDomDocument>().firstChild();
			while (rootElement.isProcessingInstruction()) {
				rootElement = rootElement.nextSibling();
			}
			return jsStringEscape(elementToJSON(rootElement.toElement()));
		} else if (typeName == "QModelIndex") {
			QModelIndex index = variant.value<QModelIndex>();
			return QString("{ row : %1, column : %2 }").arg(index.row()).arg(index.column());
		} else {
			Logger::Logger::instance()->debug(QString("variantToJSType(): Unsupported variant type: %1.").arg(typeName));
		}
	}
	
	return QString::null;
}

QString Util::jsStringEscape(const QString &string) {
	
	QString escapedString;
	escapedString.reserve(string.length());
	
	escapedString += '"';
	for (int i = 0; i < string.length(); i++) {
		QChar character = string[i];
		switch (character.unicode()) {
			case '"':
				escapedString += "\\\"";
				break;
			case '\\':
				escapedString += "\\\\";
				break;
			case '\b':
				escapedString += "\\b";
				break;
			case '\f':
				escapedString += "\\f";
				break;
			case '\n':
				escapedString += "\\n";
				break;
			case '\r':
				escapedString += "\\r";
				break;
			case '\t':
				escapedString += "\\t";
				break;
			default:
				escapedString += character;
				break;
		}
	}
	escapedString += '"';
	
	return escapedString;
}

static QString _elementToJSON(QDomElement element) {
	
	QDomNamedNodeMap attributes = element.attributes();
	QDomNodeList children = element.childNodes();
	
	QMap<QString, QStringList> data;
	
	for (uint i = 0; i < attributes.length(); i++) {
		QDomAttr attribute = attributes.item(i).toAttr();
		data["@" + attribute.name()] = QStringList(Util::jsStringEscape(attribute.value()));
	}
	
	for (uint i = 0; i < children.length(); i++) {
		if (!children.item(i).isElement()) {
			continue;
		}
		QDomElement element = children.item(i).toElement();
		if (!data.contains(element.tagName())) {
			data[element.tagName()] = QStringList(_elementToJSON(element));
		} else {
			data[element.tagName()] << _elementToJSON(element);
		}
	}
	
	if (data.isEmpty()) {
		return Util::jsStringEscape(element.text());
	}
	
	QString body;
	
	for (QMap<QString, QStringList>::ConstIterator it = data.begin();
	     it != data.end(); ++it) {
		if (!body.isEmpty()) {
			body += ", ";
		}
		body += Util::jsStringEscape(it.key()) + ": ";
		if (it.value().size() == 1) {
			body += it.value().at(0);
		} else {
			body += "[ ";
			foreach (QString string, it.value()) {
				if (!body.endsWith("[ ")) {
					body += ", ";
				}
				body += string;
			}
			body += " ]";
		}
	}
	
	return "{ " + body + " }";
}

QString Util::elementToJSON(QDomElement element) {
	
	return "{ " + jsStringEscape(element.tagName()) + ": " + _elementToJSON(element) + " }";
}

QVariantMap Util::jsonToVariantMap(const QString &json, bool *error) {
	
	JSonDriver driver;
	return driver.parse(json, error).toMap();
}

QString Util::platformPath(const QString &path) {
	
	QString dir = (path.lastIndexOf('/') >= 0 ? path.left(path.lastIndexOf('/') + 1) : "");
	QString fileName = path.mid(path.lastIndexOf('/') + 1);
	
	if (fileName.endsWith(".library")) {
#ifdef Q_WS_X11
		fileName = "lib" + fileName.mid(0, fileName.size() - 8) + ".so";
#endif
#ifdef Q_WS_WIN
		fileName = fileName.mid(0, fileName.size() - 8) + ".dll";
#endif			
#ifdef Q_WS_MAC
		fileName = "lib" + fileName.mid(0, fileName.size() - 8) + ".dylib";
#endif
	}
	
	return dir + fileName;
}
