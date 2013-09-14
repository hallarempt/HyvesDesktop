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

#include "logger/Logger.h"
#include "DelayedJabberMessage.h"
#include "JabberMock.h"

namespace Jabber {

DelayedJabberMessage::DelayedJabberMessage(const QString &message, int msec) :
	QTimer(),
	m_message(message) {
	
	connect(this, SIGNAL(timeout()), SLOT(deliverMessage()));
	
	start(msec);
}

DelayedJabberMessage::~DelayedJabberMessage() {
}

void DelayedJabberMessage::deliverMessage() {
	
	JabberMock *jabberMock = dynamic_cast<JabberMock *>(Jabber::instance());
	jabberMock->incomingXml(m_message);
	
	deleteLater();
}

} // namespace Jabber
