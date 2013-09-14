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

#ifndef JSLOGGER_H
#define JSLOGGER_H

#include <QObject>
#include "Logger.h"

#include "hyveslib_export.h"

namespace Logger {
	
class HYVESLIB_EXPORT JSLogger : public QObject, public Logger {
	
	Q_OBJECT
	
	public:
		/**
		 * Constructor for JS Logger
		 *
		 * JSLogger is a Wrapper class for exposing the Logger to JavaScript
		 */
		JSLogger(QObject *parent = 0);
		~JSLogger();
		
	public slots:
		/**
		 * Logs a debug message.
		 *
		 * @param className Name of the class
		 * @param message Message to log
		 */
		void debug(const QString &className, const QString &message);
		
		/**
		 * Logs usage information
		 *
		 * @param className Name of the class
		 * @param message Usage information to log (please choose unique identifiers within your log-class)
		 */
		void usage(const QString &className, const QString &message);
		
		/**
		 * Logs a notice message.
		 *
		 * @param className Name of the class
		 * @param message Message to log
		 */
		void notice(const QString &className, const QString &message);
		
		/**
		 * Logs a warning message.
		 *
		 * @param className Name of the class
		 * @param message Message to log
		 */
		void warning(const QString &className, const QString &message);
		
		/**
		 * Logs an error message.
		 *
		 * @param className Name of the class
		 * @param message Message to log
		 */
		void error(const QString &className, const QString &message);
		
		/**
		 * Writes a test message to the logfile only (use this to log huge amounts of data for testing)
		 *
		 * @param className Name of the class
		 * @param message message to be written to the log
		 * @note this log type gets special treatement compared to other log types:
		 * -> will not be printed to the console (except the logfile could not be opened)
		 * -> logfile will not be flushed
		 * -> not included in logging statistics
		 */
		void test(const QString &className, const QString &message);
		
		/**
		 * Anonymizes sensitive data for logging
		 *
		 * @param data QString containing the sensitive data
		 * @returns anonymized string
		 * @note In debug builds 'data' is returned unchanged
		 */
		QString anonymize(const QString &data);
		
		/**
		 * Anonymizes sensitive data for logging by replacing each char by '*'.
		 * So the lenght of the string remains the same.
		 *
		 * @param data QString containing the sensitive data
		 * @returns anonymized string
		 * @note In debug builds 'data' is returned unchanged
		 */
		QString anonymizeReplace(const QString &data);
		
		/**
		 * Anonymizes an email address for logging
		 *
		 * @param data the email address
		 * @returns anonymized string
		 * @note In debug builds 'data' is returned unchanged
		 */
		QString anonymizeEMail(const QString &data);
		
		/**
		 * Anonymizes an ip address for logging
		 *
		 * @param data the ip address
		 * @returns anonymized string
		 * @note In debug builds 'data' is returned unchanged
		 */
		QString anonymizeIPAddr(const QString &data);
		
		/**
		 * Anonymizes a filename for logging
		 *
		 * @param data the filename
		 * @returns anonymized string
		 * @note In debug builds 'data' is returned unchanged
		 */
		QString anonymizeFile(const QString &data);
};

} // namespace Logger

#endif // JSLOGGER_H
