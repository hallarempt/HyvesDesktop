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

#include "extender/Extender.h"
#include "jabber/Jabber.h"
#include "logger/Logger.h"
#include "Roster.h"
#include "RosterItem.h"
#include "Util.h"

namespace Roster {

Roster *Roster::s_instance = 0;

struct Roster::Private {
	
	Logger::Logger *log;
	
	bool initialized;
	
	QString jid;
	QString resource;
	QMap<QString, RosterItem *> items;
	
	QString itemsJSONCache;
	
	QMap<QString, QString> statusText;
	QMap<QString, QString> statusShow;
	
	Private() :
		log(0),
		initialized(false) {
		
		statusText["online"] = tr("Online");
		statusText["busy"] = tr("Busy");
		statusText["berightback"] = tr("Be right back");
		statusText["away"] = tr("Away");
		statusText["phone"] = tr("In a call");
		statusText["lunch"] = tr("Out to Lunch");
		statusText["offline"] = tr("Offline");
		
		statusShow["online"] = "normal";
		statusShow["busy"] = "dnd";
		statusShow["berightback"] = "away";
		statusShow["away"] = "away";
		statusShow["phone"] = "dnd";
		statusShow["lunch"] = "away";
		statusShow["offline"] = "offline";
	}
	
	~Private() {
		
		delete log;
	}
};

Roster *Roster::instance() {
	
	if (s_instance == 0) {
		s_instance = new Roster();
	}
	
	return s_instance;
}

void Roster::destroy() {
	
	delete s_instance;
	s_instance = 0;
}

void Roster::setInitialized() {
	
	if (!m_d->initialized) {
		emit initialized();
	}
	
	m_d->initialized = true;
}

void Roster::setOwnJid(const QString &jid) {
	
	m_d->jid = Jabber::Jabber::bare(jid);
	m_d->resource = Jabber::Jabber::resource(jid);
	
	RosterItem *ownItem = item(m_d->jid);
	ownItem->unlock();
	ownItem->resource(m_d->resource);
	ownItem->lock();
}

QStringList Roster::itemJids() const {
	
	return m_d->items.keys();
}

RosterItem *Roster::item(const QString &jid) {
	
	if (!m_d->items.contains(jid)) {
		m_d->items[jid] = new RosterItem(jid);
	}
	
	return m_d->items[jid];
}

bool Roster::hasItem(const QString &jid) const {
	
	return m_d->items.contains(jid);
}

void Roster::removeItem(const QString &jid) {
	
	if (!m_d->items.contains(jid)) {
		return;
	}
	
	delete m_d->items.take(jid);
}

void Roster::clear() {
	
	foreach (RosterItem *item, m_d->items) {
		delete item;
	}
	
	m_d->items.clear();
	
	m_d->initialized = false;
	m_d->jid = "";
	m_d->resource = "";
}

void Roster::emitItemChanged(const QVariantMap &item, const QVariantMap &changedProperties) {
	
	if (m_d->initialized) {
		emit itemChanged(item, changedProperties);
	}
	
	m_d->itemsJSONCache = "";
}

bool Roster::isInitialized() const {
	
	return m_d->initialized;
}

QString Roster::ownBareJid() const {
	
	return m_d->jid;
}

QString Roster::ownResource() const {
	
	return m_d->resource;
}

QString Roster::ownFullJid() const {
	
	if (!m_d->jid.isEmpty()) {
		return m_d->jid + "/" + m_d->resource;
	} else {
		return "";
	}
}

QString Roster::itemJSON(const QString &jid) {
	
	if (!hasItem(jid)) {
		return "{ }";
	}
	
	return Util::variantMapToJSON(item(jid)->variantMap());
}

QString Roster::itemsJSON() {
	
	if (!m_d->itemsJSONCache.isEmpty()) {
		return m_d->itemsJSONCache;
	}
	
	QVariantMap items;
	for (QMap<QString, RosterItem *>::ConstIterator it = m_d->items.constBegin();
	     it != m_d->items.constEnd(); ++it) {
		items[it.key()] = it.value()->variantMap();
	}
	m_d->itemsJSONCache = Util::variantMapToJSON(items);
	return m_d->itemsJSONCache;
}

QString Roster::ownItemJSON() {
	
	return itemJSON(m_d->jid);
}

bool Roster::ensureItem(const QString &jid) {
	
	if (hasItem(jid)) {
		return false;
	}
	
	RosterItem *newItem = item(jid);
	newItem->unlock();
	newItem->setProperty("nick", jid);
	newItem->addResource(Resource("unknown", "normal", "online", 0));
	newItem->lock();
	
	return true;
}

QString Roster::itemResourcesJSON(const QString &jid) {
	
	if (!hasItem(jid)) {
		return "{ \"resources\": [  ] }";
	}
	
	return Util::variantMapToJSON(item(jid)->resourcesVariantMap());
}

QString Roster::ownItemResourcesJSON() {
	
	return itemResourcesJSON(m_d->jid);
}

void Roster::setItemProperties(const QString &jid, const QString &propertiesJSON) {
	
	QVariantMap properties = Util::jsonToVariantMap(propertiesJSON);
	
	RosterItem *rosterItem = item(jid);
	rosterItem->unlock();
	for (QVariantMap::ConstIterator it = properties.constBegin();
	     it != properties.constEnd(); ++it) {
		rosterItem->setProperty(it.key(), it.value().toString());
	}
	rosterItem->lock();
}

bool Roster::statusExists(const QString &status) const {
	
	return m_d->statusShow.contains(status);
}
		
QString Roster::statusShow(const QString &status) const {
	
	if (m_d->statusShow.contains(status)) {
		return m_d->statusShow[status];
	} else {
		return m_d->statusShow["online"];
	}
}

QString Roster::statusText(const QString &status) const {
	
	if (m_d->statusText.contains(status)) {
		return m_d->statusText[status];
	} else {
		return m_d->statusText["online"];
	}
}

void Roster::setStatusText(const QString &status, const QString &text) {
	
	m_d->statusText[status] = text;
}

void Roster::setOwnStatus(const QString &status) {
	
	RosterItem *ownItem = item(m_d->jid);
	ownItem->unlock();
	Resource resource = ownItem->resource(m_d->resource);
	if (statusExists(status)) {
		resource.status = status;
	} else {
		resource.status = "online";
	}
	resource.show = statusShow(resource.status);
	ownItem->addResource(resource);
	ownItem->lock();
}

Roster::Roster() :
	QObject(),
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("Roster");
	
	Extender::Extender::instance()->registerObject("roster", this);
}

Roster::~Roster() {
	
	clear();
	
	Extender::Extender::instance()->unregisterObject("roster");
	
	delete m_d;
}

} // namespace Roster
