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

#ifndef HISTORYKEEPER_H
#define HISTORYKEEPER_H

#include <QObject>
#include <QVariantMap>

#include "hyveslib_export.h"

class QDate;
class QFile;

namespace HistoryKeeper {

enum Direction {
	DirectionIncoming = 0,
	DirectionOutgoing
};

class HYVESLIB_EXPORT HistoryKeeper : public QObject {
	
	Q_OBJECT
	
	public:
		static HistoryKeeper *instance();
		static void destroy();
		
		/**
		 * Logs a single chat message.
		 *
		 * @param direction The direction in which the message was sent.
		 * @param jid JID of the other contact in the chat session.
		 * @param message Content of the message sent.
		 */
		void logMessage(Direction direction, const QString &jid, QString message);
		
	public slots:
		/**
		 * Clears the chat history.
		 *
		 * @param jid Optional JID. If a JID is given, only the chat
		 *            history for that contact is cleared. If no JID is
		 *            given, all history is cleared.
		 */
		void clear(const QString &jid = QString::null);
		
		/**
		 * Returns the last messages to or from a contact encoded in
		 * JSON.
		 *
		 * @param jid JID of the contact for which to retrieve the last
		 *            messages.
		 * @param maxNumMessages The maximum amount of messages to
		 *                       return.
		 * @return The last messages sent to or from the given contact,
		 *         with a maximum of @p maxNumMessages, encoded in JSON.
		 *
		 * Example JSON string returned:
		 *
		 * @code
		 * { "messages": [ { "date": "2009-02-24", "time": "17:46", "direction": "to", "message": "ping" },
		 *                 { "date": "2009-02-24", "time": "18:01", "direction": "to", "message": "pong" } ] }
		 * @endcode
		 */
		QString lastMessagesJSON(const QString &jid, int maxNumMessages = 3);
		
		/**
		 * Returns all messages from a given day with a given contact.
		 *
		 * @param jid JID of the contact for which to retrieve the
		 *            messages.
		 * @param date Date of the messages to retrieve, in the form
		 *             "yyyy-mm-dd".
		 * @return The messages sent to or from the given contact on the
		 *         given day, encoded in JSON. For the format of the
		 *         JSON object, see lastMessagesJSON().
		 */
		QString messagesFromDayJSON(const QString &jid, const QString &date);
		
		/**
		 * Returns the dates on which messages to or from a contact were
		 * logged.
		 *
		 * @param jid JID of the contact for which to retrieve the
		 *            history dates.
		 * @param keyword Keyword to search for. If given, only dates
		 *                where this keyword occurs in the messages are
		 *                returned.
		 * @return A comma-separated list of dates, in descending order.
		 *
		 * Example string returned:
		 *
		 * @code
		 * "2009-02-25,2009-02-24,2009-01-21,2009-01-12"
		 * @endcode
		 */
		QString historyDates(const QString &jid, const QString &keyword = QString::null);
		
		/**
		 * Logs a single chat message.
		 *
		 * @param direction Either "<" for an incoming message, or ">"
		 *                  for an outgoing message.
		 * @param jid JID of the contact in the conversation.
		 * @param message The message to log.
		 *
		 * This method will not do anything is logging is disabled.
		 */
		void logMessage(const QString &direction, const QString &jid, const QString &message);
		
	private:
		static HistoryKeeper *s_instance;
		
		struct Private;
		Private *const m_d;
		
		HistoryKeeper();
		virtual ~HistoryKeeper();
		
		QFile *getWritableHistoryFile(const QString &jid, const QDate &date);
		QFile *getReadableHistoryFile(const QString &jid, const QString &date);
		QFile *getNextReadableHistoryFile(const QString &jid);
		void closeAllOpenFiles();
};

} // namespace HistoryKeeper

#endif // HISTORYKEEPER_H
