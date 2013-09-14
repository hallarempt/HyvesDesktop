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

#include <cstdlib>

#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QRegExp>
#include <tester/Tester.h>

#include "Logger.h"
#include "config.h"

namespace Logger {

Logger *Logger::s_instance = 0;

struct Logger::Private {
	bool sessionLogEnabled;
	bool usageLogEnabled;
	unsigned int errorCounter;
	unsigned int warningCounter;
	unsigned int noticeCounter;
	unsigned int usageCounter;
	unsigned int debugCounter;
	
	QFile *sessionLog;
	QFile *usageLog;
	
	Private() :
		sessionLogEnabled(false),
		usageLogEnabled(false),
		errorCounter(0),
		warningCounter(0),
		noticeCounter(0),
		usageCounter(0),
		debugCounter(0),
		sessionLog(0),
		usageLog(0) {
	}
};

Logger *Logger::instance() {
	
	if (s_instance == 0) {
		s_instance = new Logger();
	}
	
	return s_instance;
}

void Logger::destroy() {
	
	delete s_instance;
	s_instance = 0;
}

Logger::Logger(const QString &className) :
	m_d(0),
	m_className(className) {
}

Logger::~Logger() {
	
	if (m_d) {
		QString stats("Statistics: %1 errors, %2 warnings, %3 notices, %4 usages, %5 debugs logged");
		log("Logger", Debug, stats.arg(m_d->errorCounter).arg(m_d->warningCounter).arg(m_d->noticeCounter).arg(m_d->usageCounter).arg(m_d->debugCounter));
		log("Logger", Notice, "End of log.");
		
		if (m_d->sessionLog) {
			m_d->sessionLog->close();
			// preserve session log for StatsUploader if errors occurred (except when tester is running)
			if (m_d->errorCounter > 0 && !Tester::Tester::isTesting()) {
				QString preservedLogLocation = m_d->sessionLog->fileName().append(".last");
				if (QFile::exists(preservedLogLocation)) {
					QDir::root().remove(preservedLogLocation);
				}
				m_d->sessionLog->rename(preservedLogLocation);
			}
			delete m_d->sessionLog;
		}
		
		if (m_d->usageLog) {
			m_d->usageLog->close();
			delete m_d->usageLog;
		}
		
		delete m_d;
	}
}

void Logger::test(const QString &message) {
	
	instance()->log(m_className, Test, message);
}

void Logger::debug(const QString &message) {
	
	instance()->log(m_className, Debug, message);
}

void Logger::usage(const QString &message) {
	
	instance()->log(m_className, Usage, message);
}

void Logger::notice(const QString &message) {
	
	instance()->log(m_className, Notice, message);
}

void Logger::warning(const QString &message) {
	
	instance()->log(m_className, Warning, message);
}

void Logger::error(const QString &message) {
	
	instance()->log(m_className, Error, message);
}

Logger::Logger() :
	m_d(new Private()) {
	
	QString logLocation = QDesktopServices::storageLocation(QDesktopServices::DataLocation).append("/log");
#if QT_VERSION == 0x040500
#ifdef Q_WS_MAC
	logLocation = QDesktopServices::storageLocation(QDesktopServices::HomeLocation) + "/Library/Application Support/Hyves Desktop/log";
#endif
#endif

	QDir logDir(logLocation);
	if (!logDir.exists()) {
		logDir.mkpath(".");
	}
	
	// open session log
	QString sessionLogLocation = logDir.absoluteFilePath("session.txt");
	m_d->sessionLog = new QFile(sessionLogLocation);
	m_d->sessionLogEnabled = m_d->sessionLog->open(QIODevice::WriteOnly | QIODevice::Truncate);
	if (!m_d->sessionLogEnabled) {
		log("Logger", Warning, QString("Could not open log file: %1").arg(sessionLogLocation));
	}
	
	// open file for usage log (append new stats to old file content)
	QString usageLogLocation = logLocation + "/usage.txt";
	m_d->usageLog = new QFile(usageLogLocation);
	m_d->usageLogEnabled = m_d->usageLog->open(QIODevice::WriteOnly | QIODevice::Append);
	if (!m_d->usageLogEnabled) {
		log("Logger", Warning, QString("Could not open log file: %1").arg(usageLogLocation));
	}
}

void Logger::log(const QString &className, LogType type, const QString &message) {
	
	if (!m_d) {
		instance()->log(className, type, message);
		return;
	}
	
#ifdef DEBUG_BUILD
	QString color;
#endif
	QString typeStr;
	QString logMessage = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss: ");
	
	switch (type) {
		case Test:
			typeStr = "test";
			break;
		case Debug:
			m_d->debugCounter++;
			typeStr = "debug";
			break;
		case Usage:
#ifdef DEBUG_BUILD
			color = "\x1b\x5b;1;34;1m";
#endif
			m_d->usageCounter++;
			typeStr = "usage";
			break;
		case Notice:
#ifdef DEBUG_BUILD
			color = "\x1b\x5b;1;32;1m";
#endif
			m_d->noticeCounter++;
			typeStr = "NOTICE";
			break;
		case Warning:
#ifdef DEBUG_BUILD
			color = "\x1b\x5b;1;33;1m";
#endif
			m_d->warningCounter++;
			typeStr = "WARNING";
			break;
		case Error:
#ifdef DEBUG_BUILD
			color = "\x1b\x5b;1;31;1m";
#endif
			m_d->errorCounter++;
			typeStr = "ERROR";
			break;
		default:
			typeStr = "unknown";
			break;
	}
	
	logMessage.append(QString("%1: %2: " + message).arg(typeStr).arg(className.isEmpty() ? "unknown" : className));
	
	if (logMessage.endsWith('\n')) {
		logMessage.truncate(logMessage.size() - 1);
	}
	
	// write session log file
	if (m_d->sessionLogEnabled) {
		m_d->sessionLog->write((logMessage + '\n').toUtf8());
		if (type == Test) {	// if 'test' message we're done here
			return;
		}
		m_d->sessionLog->flush();
	}
	
	// write usage log file (except when Tester is running)
	if (m_d->usageLogEnabled && (type == Usage) && !Tester::Tester::isTesting()) {
		m_d->usageLog->write((logMessage + '\n').toUtf8());
		// no need to flush - if we crash the session log holds all information from the current session
		
		if (m_d->usageLog->size() > MAX_USAGELOG_SIZE) {	// rotate usage log files if reaching max size
			rotateUsageLog();
		}
	}
	
	// print message to console
#ifndef Q_WS_WIN
#ifdef DEBUG_BUILD
	logMessage.insert(0, color).append("\x1b\x5b;1;0;0m");
#endif
#endif
	QByteArray byteArray = logMessage.toUtf8();
	if (byteArray.size() >= 8192) {
		byteArray.truncate(8191); // make sure we never overflow qDebug() buffer
		byteArray[8191] = '\0';
	}
	qDebug("%s", byteArray.constData());
}

void Logger::rotateUsageLog() {
	
	Q_ASSERT(instance()->m_d);
	if (m_d) {
		QString filename = m_d->usageLog->fileName();
		m_d->usageLog->close();
		if (QFile::exists(filename + ".last")) {
			QDir::root().remove(filename + ".last");
		}
		m_d->usageLog->rename(filename + ".last");
		delete m_d->usageLog;
		m_d->usageLog = new QFile(filename);
		m_d->usageLogEnabled = m_d->usageLog->open(QIODevice::WriteOnly | QIODevice::Truncate);
	} else {
		instance()->rotateUsageLog();
	}
}

unsigned int Logger::errorsLogged() const {
	
	Q_ASSERT(instance()->m_d);
	return instance()->m_d->errorCounter;
}

unsigned int Logger::warningsLogged() const {
	
	Q_ASSERT(instance()->m_d);
	return instance()->m_d->warningCounter;
}

unsigned int Logger::noticesLogged() const {
	
	Q_ASSERT(instance()->m_d);
	return instance()->m_d->noticeCounter;
}

unsigned int Logger::usagesLogged() const {
	
	Q_ASSERT(instance()->m_d);
	return instance()->m_d->usageCounter;
}

unsigned int Logger::debugsLogged() const {
	
	Q_ASSERT(instance()->m_d);
	return instance()->m_d->debugCounter;
}

QString Logger::anonymize(const QString& data, AnonymizationType type) {
#ifdef DEBUG_BUILD
	return data;
#else
	if (Tester::Tester::isTesting()) {
		return data;
	}
	
	int index;
	switch (type) {
		case Remove:
			return QString();
		case Replace:
			return QString(data).fill('*');
		case EMail:
			//index = data.indexOf('@');
			//return QString(data).replace(0, index, QString('*').repeated(index));	// replaces all chars before '@" e.g. ******@hyves.nl
			return QString(data).replace(QRegExp("[^@^\\..]"), "*"); // replaces all chars except '@' and '.' e.g. ******@*****.**
		case IPAddr:
			//return data.left(data.lastIndexOf('.') + 1).append('*');	// removes only the last number e.g. 192.162.0.*
			return QString(data).replace(QRegExp("[^\\..]"), "*");	// replaces all digits by '*' e.g. ***.***.*.*
		case File:
			index = data.lastIndexOf('.');
			if (index < (data.size() - 5)) {	// make sure not to return more than 4 chars of the file extension
				index = data.size() - 5;
			}
			return QString("%1file").arg((index > 0) ? data.right(data.size() - 1 - index).append('-') : "unknown "); // e.g. <extension>-file
		default:
			return QString();
	}
#endif
}

} // namespace Logger
