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

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QTranslator>

#include "settingsmanager/SettingsManager.h"
#include "extender/Extender.h"
#include "logger/Logger.h"
#include "Translator.h"

namespace Translator {

Translator *Translator::s_instance = 0;

struct Translator::Private {
	Logger::Logger *log;
	
	QString currentLanguage;
	QList<QTranslator *> activeTranslators;
	
	Private() :
		log(0) {
	}
	
	~Private() {
		
		delete log;
	}
};

Translator *Translator::instance() {
	
	if (s_instance == 0) {
		s_instance = new Translator();
	}
	
	return s_instance;
}

void Translator::destroy() {
	
	delete s_instance;
	s_instance = 0;
}

QString Translator::tr(const QString &string) {
	
	// Note: we use qApp->translate() here because the context of our js strings is empty.
	QString translatedString =  qApp->translate("", string.toUtf8().data());//QObject::trUtf8(string.toUtf8().data());
	return translatedString;
}

QString Translator::iso6392Language() const {
	
	return m_d->currentLanguage;
}

QString Translator::locale() const {
	
	if (m_d->currentLanguage == "NLD") {
		return "nl_NL";
	} else {
		return "en_GB";
	}
}

void Translator::setLanguage(const QString &iso6392Language) {
	
	if (iso6392Language == m_d->currentLanguage) {
		return;
	}
	if (iso6392Language == "ENG") {
		// delete all translators
		foreach (QTranslator *translator, m_d->activeTranslators) {
			qApp->removeTranslator(translator);
			delete translator;
		}
		m_d->activeTranslators.clear();
	} else {
		QString translatedSuffix;
		if (iso6392Language.toUpper() == "NLD") {
			translatedSuffix = "nl";
		}
		
		QDir dir = QDir(qApp->applicationDirPath());
		QStringList nameFilters;
		nameFilters << QString("*_%1.qm").arg(translatedSuffix);
		dir.setNameFilters(nameFilters);
		QFileInfoList languageFiles = dir.entryInfoList();
		foreach (QFileInfo languageFile, languageFiles) {
			m_d->log->debug("Loading language file " + m_d->log->anonymize(languageFile.absoluteFilePath(), Logger::File));
			QTranslator *translator = new QTranslator();
			translator->load(languageFile.completeBaseName(), dir.path());
			qApp->installTranslator(translator);
			m_d->activeTranslators.append(translator);
		}
	}
	
	m_d->currentLanguage = iso6392Language;
}

static QString domNodeToJs(const QDomNode &n, int &v, bool prepend = false) {
	
	QString js("");
	if (!n.isNull()) {
		if (n.isText()) {
			js += QString("var __tr_var_%1 = document.createTextNode(\"" + n.toText().data() + "\");\n").arg(++v);
			if (prepend) {
				js += QString("%o.insertBefore(__tr_var_%1, %o.firstChild);\n").arg(v);
			} else {
				js += QString("%o.appendChild(__tr_var_%1);\n").arg(v);
			}
		} else if (n.isElement()) {
			QDomElement el = n.toElement();
			js += QString("var __tr_var_%1 = document.createElement(\"" + el.tagName() + "\");\n").arg(++v);
			int v_save = v;
			QDomNode daughter = n.firstChild();
			while (!daughter.isNull()) {
				js += domNodeToJs(daughter, v).replace("%o", QString("__tr_var_%1").arg(v_save));
				daughter = daughter.nextSibling();
			}
			if (prepend) {
				js += QString("%o.insertBefore(__tr_var_%1, %o.firstChild);\n").arg(v_save);
			} else {
				js += QString("%o.appendChild(__tr_var_%1);\n").arg(v_save);
			}
		}
	}
	return js;
}

static QString htmlToJs(const QString &htmlString) {
	
	QDomDocument doc("html");
	doc.setContent("<html>" + htmlString + "</html>");
	QDomElement html = doc.documentElement();
	QDomNode n = html.lastChild();
	QString js("");
	js += "if (%o.childNodes.length == 0) %o.appendChild(document.createElement(\"div\"));\n";
	int v = 0;
	while (!n.isNull()) {
		js += domNodeToJs(n, v, true);
		n = n.previousSibling();
	}
	return js;
}

QString Translator::trHtml(const QString &string) {

	QString str = tr(string);
#ifdef Q_WS_MAC
	// no shortkeys in the mac - plays havoc with existing Command-<X> shotkeys
	str = str.replace(QRegExp("&([a-zA-Z0-9])"), "\\1");
#else
	str = str.replace(QRegExp("&([a-zA-Z0-9])"), "<u>\\1</u>");
#endif

	str.replace(" & ", " &amp; ");
	return str;
}

QString Translator::trHtmlJs(const QString &string, const QString &jsObject) {

	return htmlToJs(trHtml(string)).replace("%o", jsObject);
}

QString Translator::trAccessKey(const QString &string) {
	
#ifdef Q_WS_MAC
	// no shortkeys in the mac - plays havoc with existing Command-<X> shotkeys
	return QString();
#else
	QRegExp ampRx("&([a-zA-Z0-9])");
	if (ampRx.indexIn(tr(string)) != -1) { // "&" found
		return ampRx.cap(1).toLower();
	}
	return QString();
#endif
}

Translator::Translator() :
	QObject(),
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("Translator");
	
	setLanguage(SettingsManager::SettingsManager::instance()->stringValue("language", "NLD"));
	
	Extender::Extender::instance()->registerObject("translator", this);
}

Translator::~Translator() {
	
	SettingsManager::SettingsManager::instance()->setValue("language", m_d->currentLanguage);
	
	delete m_d;
}

} // namespace Translator
