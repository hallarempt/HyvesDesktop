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

#ifndef LOGGER_H
#define LOGGER_H

#include <QString>

#include "hyveslib_export.h"

const qint64 MAX_USAGELOG_SIZE = 200 * 1024;	// maximum logfile size = 200kB

namespace Logger {
	
	/**
	 * Type describing how to anonymize sensitive data in Release builds:
	 * Remove:	will just return an empty string (default and fastest)
	 * Replace:	each char will be replaced by '*' like "abc" -> "***"
	 * EMail:	replaces each character '@' and '.' by '*' e.g. markus@hyves.nl -> "******@*****.**"
	 * IPAddr:	argument contains an IP address e.g. 192.168.0.10 -> ***.***.*.*
	 * File:	returns ''<file-extension>-file'' (up to a max of 4 chars) or ''unknown file'' if the file has no extension
	 */
	typedef enum {
		Remove = 0,
		Replace,
		EMail,
		IPAddr,
		File
	} AnonymizationType;
	
	/**
	 * Log message type
	 */
	typedef enum {
		Error = 0,
		Warning,
		Notice,
		Usage,
		Debug,
		Test
	} LogType;
	
class HYVESLIB_EXPORT Logger {
	
	public:
		/**
		 * Returns the global Logger instance.
		 *
		 * @return Logger instance.
		 */
		static Logger *instance();
		
		/**
		 * Destroys the global Logger instance.
		 */
		static void destroy();
		
		/**
		 * Constructor.
		 *
		 * You can create private instances for convenience, that will pass the
		 * given @p className to the global Logger.
		 *
		 * @param className Name of the class instantiating the Logger. This
		 *                  name will be used in log messages.
		 */
		Logger(const QString &className);
		
		/**
		 * Destructor.
		 */
		virtual ~Logger();
		
		/**
		 * Logs a debug message.
		 *
		 * @param message Message to log.
		 */
		void debug(const QString &message);
		
		/**
		 * Logs usage information
		 *
		 * @param message Usage information to log (please choose unique identifiers within your log-class)
		 */
		void usage(const QString &message);
		
		/**
		 * Logs a notice message.
		 */
		void notice(const QString &message);
		
		/**
		 * Logs a warning message.
		 */
		void warning(const QString &message);
		
		/**
		 * Logs an error message.
		 */
		void error(const QString &message);
		
		/**
		 * Writes a test message to the logfile only (use this to log huge amounts of data for testing)
		 *
		 * @param message message to be written to the log
		 * @note this log type gets special treatement compared to other log types:
		 * -> will not be printed to the console (except the logfile could not be opened)
		 * -> logfile will not be flushed
		 * -> not included in logging statistics
		 */
		void test(const QString &message);
		
		/**
		 * Returns the number of errors logged since the application was started
		 */
		unsigned int errorsLogged() const;
		
		/**
		 * Returns the number of warnings logged since the application was started
		 */
		unsigned int warningsLogged() const;
		
		/**
		 * Returns the number of notice messages logged since the application was started
		 */
		unsigned int noticesLogged() const;
		
		/**
		 * Returns the number of usage messages logged since the application was started
		 */
		unsigned int usagesLogged() const;
		
		/**
		 * Returns the number of debug messages logged since the application was started
		 */
		unsigned int debugsLogged() const;
		
		/**
		 * Signal handler used for catching and logging crashes.
		 */
		static void signalHandler(int signal);
		
		/**
		 * Moves current usage log to "usage.txt.last" and creates a new file for the current session
		 *
		 * @note If a file with that name already exists it will be overwritten.
		 */
		void rotateUsageLog();
		
		/**
		 * Use this to anonymize sensitive data (like email, filenames, usernames,...) for logging
		 *
		 * @param data QString containing the sensitive data
		 * @param type allows special treatment for email-/ip-addresses etc. to keep at least some information (optional)
		 * @returns anonymized string
		 * @note In debug builds 'data' is returned unchanged
		 */
		static QString anonymize(const QString& data, AnonymizationType type = Remove);
	
	protected:
		void log(const QString &className, LogType type, const QString &message);
	
	private:
		static Logger *s_instance;
		
		struct Private;
		Private *const m_d;
		QString const m_className;
		
		/**
		 * Private constructor used by instance().
		 */
		Logger();
};

} // namespace Logger

#endif // LOGGER_H
