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

#include <QDate>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTime>

#include "extender/Extender.h"
#include "filer/Filer.h"
#include "jabber/Jabber.h"
#include "logger/Logger.h"
#include "settingsmanager/SettingsManager.h"
#include "windowmanager/WindowManager.h"
#include "HistoryKeeper.h"
#include "Util.h"
#include "config.h"


namespace HistoryKeeper {

HistoryKeeper *HistoryKeeper::s_instance = 0;

struct HistoryKeeper::Private {
	Logger::Logger *log;
	
	QString historyLocation;
	
	QMap<QString, QFile *> historyFiles;
	
	QString searchJid;
	QStringList searchEntries;
	int searchIndex;
	
	Private() :
		log(0) {
	}
	
	~Private() {
		
		delete log;
	}
};

HistoryKeeper *HistoryKeeper::instance() {
	
	if (s_instance == 0) {
		s_instance = new HistoryKeeper();
	}
	
	return s_instance;
}

void HistoryKeeper::destroy() {
	
	delete s_instance;
	s_instance = 0;
}

void HistoryKeeper::clear(const QString &jid) {
	
	closeAllOpenFiles();
	
	QString path(m_d->historyLocation);
	if (!jid.isEmpty()) {
		QString bareJid = Jabber::Jabber::bare(jid);
		QString ownBareJid = Jabber::Jabber::instance()->bareJid();
		path += "/" + ownBareJid + "/" + bareJid;
	}
	
	QDir dir;
	if (!dir.exists(path)) {
		return;
	}
	
	bool result = Filer::removeDirectory(path);
	if (result) {
		m_d->log->debug(QString("History at %1 is cleared").arg(m_d->log->anonymize(path, Logger::Replace)));
	} else {
		m_d->log->error(QString("Could not clear history at %1").arg(m_d->log->anonymize(path, Logger::Replace)));
	}
}

void HistoryKeeper::logMessage(Direction direction, const QString &jid, QString message) {
	
	QFile *historyFile = getWritableHistoryFile(jid, QDate::currentDate());
	if (!historyFile->isWritable()) {
		m_d->log->debug("Cannot write chat history.");
		return;
	}
	
	QString line;
	line += QTime::currentTime().toString("HH:mm") + " ";
	line += QString(direction == DirectionIncoming ? "<" : ">") + " ";
	line += message.replace("\n", " ") + "\n";
	
	historyFile->write(line.toUtf8());
	historyFile->flush();
}

QString HistoryKeeper::lastMessagesJSON(const QString &jid, int maxNumMessages) {
	
	m_d->searchJid = "";
	
	QStringList lines;
	QFile *historyFile = 0;
	int numMessages = 0;
	while (numMessages < maxNumMessages &&
	       (historyFile = getNextReadableHistoryFile(jid)) != 0) {
		QFileInfo fileInfo(*historyFile);
		QString dateSuffix = fileInfo.baseName() + " ";
		
		QStringList fileLines;
		while (!historyFile->atEnd()) {
			fileLines += dateSuffix + QString::fromUtf8(historyFile->readLine().data());
			numMessages++;
		}
		lines = fileLines + lines;
		
		delete historyFile;
		
		if (numMessages >= maxNumMessages) {
			break;
		}
	}
	
	QVariantList messages;
	for (int i = qMax(lines.size() - 1 - maxNumMessages, 0); i < lines.size(); i++) {
		QString line = lines[i];
		
		QVariantMap message;
		message["date"] = line.left(10);
		message["time"] = line.mid(11, 5);
		message["direction"] = (line.mid(17, 1) == "<" ? "from" : "to");
		message["message"] = line.mid(19, line.length() - 20);
		
		messages << message;
	}
	
	QVariantMap result;
	result["messages"] = messages;
	return Util::variantMapToJSON(result);
}

QString HistoryKeeper::messagesFromDayJSON(const QString &jid, const QString &date) {
	
	QFile *historyFile = getReadableHistoryFile(jid, date);
	if (historyFile == 0) {
		QVariantMap result;
		result["messages"] = QVariantList();
		return Util::variantMapToJSON(result);
	}
	
	QVariantList messages;
	while (!historyFile->atEnd()) {
		QString line = QString::fromUtf8(historyFile->readLine().data());
		
		QVariantMap message;
		message["date"] = date;
		message["time"] = line.left(5);
		message["direction"] = (line.mid(6, 1) == "<" ? "from" : "to");
		message["message"] = line.mid(8, line.length() - 9);
		
		messages << message;
	}
	
	delete historyFile;
	
	QVariantMap result;
	result["messages"] = messages;
	return Util::variantMapToJSON(result);
}

QString HistoryKeeper::historyDates(const QString &jid, const QString &keyword) {
	
	QString bareJid = Jabber::Jabber::bare(jid);
	QString ownBareJid = Jabber::Jabber::instance()->bareJid();
	
	QString path(m_d->historyLocation + "/" + ownBareJid + "/" + bareJid);
	QDir dir(path);
	QStringList entries = dir.entryList(QDir::Files, QDir::Name | QDir::Reversed);
	
	QString legacyPath(m_d->historyLocation + "/" + bareJid);
	dir.setPath(legacyPath);
	entries.append(dir.entryList(QDir::Files, QDir::Name | QDir::Reversed));
	
	if (keyword.isEmpty()) {
		return entries.join(",");
	}
	
	QStringList matchingEntries;
	for (QStringList::const_iterator it = entries.constBegin(); it != entries.constEnd(); ++it) {
		QString entry = *it;
		QFile file(path + "/" + entry);
		bool opened = file.open(QFile::ReadOnly);
		if (!opened) {
			file.setFileName(legacyPath + "/" + entry);
			opened = file.open(QFile::ReadOnly);
			if (!opened) {
				continue;
			}
		}
		
		while (!file.atEnd()) {
			QString line = QString::fromUtf8(file.readLine().data());
			line = line.mid(8, line.length() - 9);
			if (line.contains(keyword, Qt::CaseInsensitive)) {
				matchingEntries << entry;
				break;
			}
		}
		
		file.close();
	}
	
	return matchingEntries.join(",");
}

void HistoryKeeper::logMessage(const QString &direction, const QString &jid, const QString &message) {
	
	if (SettingsManager::SettingsManager::instance()->boolValue("General/enableLocalHistory", true)) {
		logMessage(direction == "<" ? DirectionIncoming : DirectionOutgoing, jid, message);
	}
}

HistoryKeeper::HistoryKeeper() :
	QObject(),
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("HistoryKeeper");
	
	QString dataLocation = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#if QT_VERSION == 0x040500
#ifdef Q_WS_MAC
	dataLocation = QDesktopServices::storageLocation(QDesktopServices::HomeLocation) + "/Library/Application Support/Hyves Desktop";
#endif
#endif
	m_d->historyLocation = dataLocation + "/History";
	
	Extender::Extender::instance()->registerObject("historyKeeper", this);
}

HistoryKeeper::~HistoryKeeper() {
	
	Extender::Extender::instance()->unregisterObject("historyKeeper");
	
	closeAllOpenFiles();
	
	delete m_d;
}

QFile *HistoryKeeper::getWritableHistoryFile(const QString &jid, const QDate &date) {
	
	QString bareJid = Jabber::Jabber::bare(jid);
	QString ownBareJid = Jabber::Jabber::instance()->bareJid();
	QString path(m_d->historyLocation + "/" + ownBareJid + "/" + bareJid);
	QDir dir;
	dir.mkpath(path);
	QString fileName(path + "/" + date.toString("yyyy-MM-dd"));
	
	QFile *historyFile = 0;
	if (m_d->historyFiles.contains(bareJid)) {
		historyFile = m_d->historyFiles[bareJid];
		
		if (historyFile->fileName() != fileName) {
			delete historyFile;
			historyFile = 0;
		}
	}
	if (historyFile == 0) {
		historyFile = new QFile(fileName, this);
		historyFile->open(QFile::WriteOnly | QFile::Append);
		m_d->historyFiles[bareJid] = historyFile;
	}
	
	Q_ASSERT(historyFile != 0);
	return historyFile;
}

QFile *HistoryKeeper::getReadableHistoryFile(const QString &jid, const QString &date) {
	
	QString bareJid = Jabber::Jabber::bare(jid);
	QString ownBareJid = Jabber::Jabber::instance()->bareJid();
	QString path(m_d->historyLocation + "/" + ownBareJid + "/" + bareJid);
	QString fileName(path + "/" + date);
	
	QFile *historyFile = new QFile(fileName, this);
	bool opened = historyFile->open(QFile::ReadOnly);
	if (!opened) {
		QString legacyPath(m_d->historyLocation + "/" + bareJid);
		historyFile->setFileName(legacyPath + "/" + date);
		
		opened = historyFile->open(QFile::ReadOnly);
		if (!opened) {
			delete historyFile;
			return 0;
		}
	}
	
	return historyFile;
}

QFile *HistoryKeeper::getNextReadableHistoryFile(const QString &jid) {
	
	QString bareJid = Jabber::Jabber::bare(jid);
	QString ownBareJid = Jabber::Jabber::instance()->bareJid();
	QString path(m_d->historyLocation + "/" + ownBareJid + "/" + bareJid);
	
	if (m_d->searchJid != bareJid) {
		QDir dir(path);
		if (!dir.exists()) {
			m_d->log->debug("No history found for " + m_d->log->anonymize(jid, Logger::EMail));
			return 0;
		}
		
		m_d->searchJid = bareJid;
		m_d->searchEntries = dir.entryList(QDir::Files, QDir::Name | QDir::Reversed);
		m_d->searchIndex = 0;
	}
	
	while (m_d->searchIndex < m_d->searchEntries.size()) {
		QFile *historyFile = new QFile(path + "/" + m_d->searchEntries[m_d->searchIndex], this);
		m_d->searchIndex++;
		
		bool opened = historyFile->open(QFile::ReadOnly);
		if (!opened) {
			m_d->log->debug("Could not open history file " + m_d->log->anonymize(historyFile->fileName(), Logger::Replace));
			delete historyFile;
			continue;
		}
		return historyFile;
	}
	
	return 0;
}

void HistoryKeeper::closeAllOpenFiles() {
	
	foreach(QFile *file, m_d->historyFiles) {
		delete file;
	}
	
	m_d->historyFiles.clear();
}

} // namespace HistoryKeeper
