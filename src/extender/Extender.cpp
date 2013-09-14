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

#include <QMap>
#include <QStringList>
#include <QWebFrame>

#include "logger/Logger.h"
#include "Environment.h"
#include "Extender.h"
#include "Util.h"

namespace Extender {

Extender *Extender::s_instance = 0;

struct Extender::Private {
	Logger::Logger *log;
	
	QMap<QString, QObject *> registeredObjects;
	QMap<int, Environment *> registeredEnvironments;
	
	QString currentExecutingStatement;
	
	Private() :
		log(0) {
	}
	
	~Private() {
		for (QMap<int, Environment *>::ConstIterator it = registeredEnvironments.begin();
			 it != registeredEnvironments.end(); ++it) {
			delete *it;
		}
		
		delete log;
	}
};

Extender *Extender::instance() {
	
	if (s_instance == 0) {
		s_instance = new Extender();
	}
	
	return s_instance;
}

void Extender::destroy() {
	
	delete s_instance;
	s_instance = 0;
}

void Extender::registerEnvironment(QWebFrame *frame) {
	
	connect(frame, SIGNAL(javaScriptWindowObjectCleared()), SLOT(javaScriptWindowObjectCleared()));
	
	Environment *environment = new Environment();
	environment->webFrame = frame;
	frame->addToJavaScriptWindowObject("extender", environment);
	m_d->registeredEnvironments[environment->id()] = environment;
	
	for (QMap<QString, QObject *>::ConstIterator it = m_d->registeredObjects.begin();
		 it != m_d->registeredObjects.end(); ++it) {
		QString name = it.key();
		QObject *object = it.value();
		frame->addToJavaScriptWindowObject(name, object);
	}
}

void Extender::unregisterEnvironment(QWebFrame *frame) {
	
	// TODO: Should registered objects be explicitly unregistered?
	
	frame->disconnect(SIGNAL(javaScriptWindowObjectCleared()), this);
	
	for (QMap<int, Environment *>::Iterator it = m_d->registeredEnvironments.begin();
		 it != m_d->registeredEnvironments.end(); ++it) {
		Environment *environment = it.value();
		if(environment->webFrame == frame) {
			delete environment;
			m_d->registeredEnvironments.erase(it);
			return;
		}
	}
	
	m_d->log->debug("unregisterEnvironment(): Web frame not registered.");
}

void Extender::registerObject(const QString &name, QObject *object) {
	
	if (name == "extender") {
		m_d->log->debug("registerObject(): Extender object is reserved.");
		return;
	}
	
	if (m_d->registeredObjects.contains(name)) {
		m_d->log->debug(QString("registerObject(): Object %1 already registered.").arg(name));
		return;
	}
	
	for (QMap<int, Environment *>::ConstIterator it = m_d->registeredEnvironments.begin();
	     it != m_d->registeredEnvironments.end(); ++it) {
		Q_ASSERT((*it)->webFrame != 0);
		(*it)->webFrame->addToJavaScriptWindowObject(name, object);
	}
	
	connect(object, SIGNAL(destroyed(QObject *)), SLOT(objectDestroyed(QObject *)));
	m_d->registeredObjects[name] = object;
}

void Extender::unregisterObject(const QString &name) {
	
	if (!m_d->registeredObjects.contains(name)) {
		m_d->log->debug(QString("unregisterObject(): Object %1 not registered.").arg(name));
		return;
	}
	
	m_d->registeredObjects[name]->disconnect(this);
	m_d->registeredObjects.remove(name);
}

QObject *Extender::registeredObject(const QString &name) {
	
	if (!m_d->registeredObjects.contains(name)) {
		m_d->log->debug(QString("registeredObject(): Object %1 not registered.").arg(name));
		return 0;
	}
	
	return m_d->registeredObjects[name];
}

QVariant Extender::jsCall(int environmentId, const QString &function,
                          QVariantList argumentList) {
	
	if (!m_d->registeredEnvironments.contains(environmentId)) {
		m_d->log->debug("jsCall(): Environment does not exist.");
		return QVariant();
	}
	
	QString arguments;
	foreach (QVariant argument, argumentList) {
		if (!arguments.isEmpty()) {
			arguments += ",";
		}
		arguments += Util::variantToJSType(argument);
	}
	
	QString statement = function + "(" + arguments + ")";
	
	QVariant result;
	Environment *environment = m_d->registeredEnvironments[environmentId];
	
	m_d->currentExecutingStatement = statement;	
	Q_ASSERT(environment->webFrame != 0);
	result = environment->webFrame->evaluateJavaScript(statement);
	m_d->currentExecutingStatement = QString::null;
	
	return result;
}

QString Extender::currentExecutingStatement() const {
	
	return m_d->currentExecutingStatement;
}

Extender::Extender() :
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("Extender");
	
	registerMetaTypes();
}

Extender::~Extender() {
	
	delete m_d;
}

void Extender::javaScriptWindowObjectCleared() {
	
	QWebFrame *frame = qobject_cast<QWebFrame *>(sender());
	Q_ASSERT(frame != 0);
	
	for (QMap<QString, QObject *>::ConstIterator it = m_d->registeredObjects.begin();
		 it != m_d->registeredObjects.end(); ++it) {
		QString name = it.key();
		QObject *object = it.value();
		frame->addToJavaScriptWindowObject(name, object);
	}
	
	for (QMap<int, Environment *>::ConstIterator it = m_d->registeredEnvironments.begin();
		 it != m_d->registeredEnvironments.end(); ++it) {
		Environment *infoObject = it.value();
		if(infoObject->webFrame == frame) {
			infoObject->disconnectAll();
			frame->addToJavaScriptWindowObject("extender", infoObject);
		}
	}
}

void Extender::objectDestroyed(QObject *object) {
	
	QString name = m_d->registeredObjects.key(object);
	
	if (name.isNull()) {
		m_d->log->debug("objectDestroyed(): Got destroyed() signal for non-registered object.");
		return;
	}
	
	m_d->registeredObjects.remove(name);
}

} // namespace Extender
