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

#include <QMetaMethod>

#include "Extender.h"
#include "JSSlotWrapper.h"

namespace Extender {

JSSlotWrapper::JSSlotWrapper(QObject *parent, int environmentId,
                             const QMetaMethod &signalMethod, const QString &slotName) :
	QObject(parent),
	environmentId(environmentId),
	slot(slotName) {
	
	QList<QByteArray> parameterTypes = signalMethod.parameterTypes();
	for (int i = 0; i < parameterTypes.count(); i++) {
		int type = QMetaType::type(parameterTypes.at(i).constData());
		if (type == QMetaType::Void) {
			qWarning("Don't know how to handle '%s', use qRegisterMetaType to register it.",
			         parameterTypes.at(i).constData());
		}
		m_argumentTypes.append(type);
	}
}

JSSlotWrapper::~JSSlotWrapper() {
}

int JSSlotWrapper::qt_metacall(QMetaObject::Call call, int id, void **a) {
	
	id = QObject::qt_metacall(call, id, a);
	if (id < 0) {
		return id;
	}
	
	if (call == QMetaObject::InvokeMetaMethod) {
		if (id == 0) {
			callSlot(a);
		}
		--id;
	}
	return id;
}

void JSSlotWrapper::callSlot(void **a) {
	
	QList<QVariant> argumentList;
	for (int i = 0; i < m_argumentTypes.count(); i++) {
		QMetaType::Type type = static_cast<QMetaType::Type>(m_argumentTypes.at(i));
		argumentList.append(QVariant(type, a[i + 1]));
	}
	
	Extender::instance()->jsCall(environmentId, slot, argumentList);
}

} // namespace Extender
