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

#include <QByteArray>
#include <QList>
#include <QMetaMethod>
#include <QMetaObject>

#include "logger/Logger.h"
#include "Environment.h"
#include "Extender.h"
#include "JSSlotWrapper.h"

namespace Extender {

struct Environment::Private {
	Logger::Logger *log;
	QList<JSSlotWrapper *> slotWrappers;
	
	int id;
	
	Private() :
		log(0) {
		
		static int s_id = 0;
		id = s_id++;
	}
	
	~Private() {
		
		delete log;
	}
};

Environment::Environment() :
	QObject(),
	webFrame(0),
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("Environment");
}

Environment::~Environment() {
	
	disconnectAll();
	delete m_d;
}

int Environment::id() {
	
	return m_d->id;
}

void Environment::connect(const QString &sender, const QString &signal, const QString &slot) {
	
	QObject *senderObject = Extender::instance()->registeredObject(sender);
	if (senderObject == 0) {
		return;
	}
	
	QByteArray normalizedSignal = QMetaObject::normalizedSignature(signal.toAscii().data());
	int signalIndex = senderObject->metaObject()->indexOfSignal(normalizedSignal.data());
	if (signalIndex < 0) {
		m_d->log->debug(QString("connect(): No such signal: %1.")
		                .arg(QString(normalizedSignal)));
		return;
	}
	
	JSSlotWrapper *slotWrapper = new JSSlotWrapper(this, m_d->id, senderObject->metaObject()->method(signalIndex), slot);
	m_d->slotWrappers.append(slotWrapper);
	
#ifdef Q_CC_BOR
	const int memberOffset = slotWrapper->metaObject()->methodCount();
#else
	static const int memberOffset = slotWrapper->metaObject()->methodCount();
#endif
	
	if (!QMetaObject::connect(senderObject, signalIndex, slotWrapper, memberOffset, Qt::DirectConnection, 0)) {
		m_d->log->warning("connect(): Failed to connect signal.");
		return;
	}
}

void Environment::disconnectAll() {
	
	foreach (JSSlotWrapper *slotWrapper, m_d->slotWrappers) {
		delete slotWrapper;
	}
	m_d->slotWrappers.clear();
}

} // namespace Extender
