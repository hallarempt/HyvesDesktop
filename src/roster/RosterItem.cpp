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
#include "settingsmanager/SettingsManager.h"
#include "Roster.h"
#include "RosterItem.h"

namespace Roster {

const Resource Resource::null = Resource();

RosterItem::RosterItem(const QString &jid) :
	m_jid(jid),
	m_locked(true) {
	
	m_username = m_jid.left(m_jid.indexOf("@"));
	m_userhome = m_username + "." + SettingsManager::SettingsManager::instance()->baseServer();
	
	m_resources.append(Resource());
}

const QString &RosterItem::jid() const {
	
	return m_jid;
}

const QString &RosterItem::show() const {
	
	return resource().show;
}

const QString &RosterItem::status() const {
	
	return resource().status;
}

const QString &RosterItem::userhome() const {
	
	return m_userhome;
}

const QString &RosterItem::username() const {
	
	return m_username;
}

bool RosterItem::hasAvailableResource() const {
	
	return (m_resources.size() > 1 || !m_resources[0].id.isEmpty());
}

const Resource &RosterItem::resource() const {
	
	return m_resources[0];
}

const Resource &RosterItem::resource(const QString &id) {
	
	for (QList<Resource>::ConstIterator it = m_resources.constBegin();
	     it != m_resources.constEnd(); ++it) {
		if (id == (*it).id) {
			return *it;
		}
	}
	
	if (m_locked) {
		Logger::Logger::instance()->debug("Trying to modify a locked roster item.");
		return Resource::null;
	}
	
	Resource newResource;
	newResource.id = id;
	addResource(newResource);
	return resource(id);
}

const QList<Resource> &RosterItem::resources() const {
	
	return m_resources;
}

void RosterItem::addResource(const Resource &resource) {
	
	if (m_locked) {
		Logger::Logger::instance()->debug("Trying to modify a locked roster item.");
		return;
	}
	if (resource.id.isEmpty()) {
		Logger::Logger::instance()->debug("Cannot add a resource without a valid ID.");
		return;
	}
	
	if (!hasAvailableResource()) {
		m_changedProperties["available"] = false;
	}
	m_changedProperties["status"] = status();
	
	if (m_resources.isEmpty() || m_resources[0].id.isEmpty()) {
		m_resources.clear();
	} else {
		for (QList<Resource>::iterator it = m_resources.begin();
		     it != m_resources.end(); ++it) {
			if (resource.id == (*it).id) {
				*it = resource;
				return;
			}
		}
		for (QList<Resource>::iterator it = m_resources.begin();
		     it != m_resources.end(); ++it) {
			if (resource.priority > (*it).priority) {
				m_resources.insert(it, resource);
				return;
			}
		}
	}
	
	m_resources.append(resource);
}

void RosterItem::removeResource(const QString &id) {
	
	if (m_locked) {
		Logger::Logger::instance()->debug("Trying to modify a locked roster item.");
		return;
	}
	
	m_changedProperties["status"] = status();
	
	for (QList<Resource>::iterator it = m_resources.begin();
	     it != m_resources.end(); ++it) {
		if ((*it).id == id) {
			m_resources.removeAll(*it);
			break;
		}
	}
	
	if (m_resources.size() == 0) {
		m_changedProperties["available"] = true;
		m_resources.append(Resource());
	}
}

int RosterItem::clientResourceBits() const {
	
	int bits = 0;
	for (QList<Resource>::ConstIterator it = m_resources.constBegin();
	     it != m_resources.constEnd(); ++it) {
		QString id = (*it).id;
		if (id.isEmpty()) {
			// is the dummy resource
		} else if (id == "siteClient") {
			bits |= Resource::siteClient;
		} else if (id == "webClient") {
			bits |= Resource::webClient;
		} else if (id.startsWith("desktopClient")) {
			bits |= Resource::desktopClient;
		} else {
			bits |= Resource::otherClient;
		}
	}
	return bits;
}

const QStringList &RosterItem::groups() const {
	
	return m_groups;
}

void RosterItem::addGroup(const QString &group) {
	
	if (m_locked) {
		Logger::Logger::instance()->debug("Trying to modify a locked roster item.");
		return;
	}
	
	if (!m_groups.contains(group)) {
		m_groups.append(group);
	}
}

void RosterItem::removeGroup(const QString &group) {
	
	if (m_locked) {
		Logger::Logger::instance()->debug("Trying to modify a locked roster item.");
		return;
	}
	
	m_groups.removeAll(group);
}

QString RosterItem::property(const QString &key) const {
	
	return m_properties[key];
}

const QMap<QString, QString> &RosterItem::properties() const {
	
	return m_properties;
}

void RosterItem::setProperty(const QString &key, const QString &value) {
	
	if (m_locked) {
		Logger::Logger::instance()->debug("Trying to modify a locked roster item.");
		return;
	}
	
	m_changedProperties[key] = m_properties[key];
	m_properties[key] = value;
}

void RosterItem::removeProperty(const QString &key) {
	
	if (m_locked) {
		Logger::Logger::instance()->debug("Trying to modify a locked roster item.");
		return;
	}
	
	m_changedProperties[key] = m_properties[key];
	m_properties.remove(key);
}

QVariantMap RosterItem::variantMap() const {
	
	QVariantMap variantMap;
	variantMap["jid"] = m_jid;
	
	for (QMap<QString, QString>::ConstIterator it = m_properties.constBegin();
	     it != m_properties.constEnd(); ++it) {
		variantMap[it.key()] = it.value();
	}
	
	QString itemStatus = status();
	variantMap["available"] = hasAvailableResource();
	variantMap["availableClients"] = clientResourceBits();
	variantMap["show"] = show();
	variantMap["status"] = itemStatus;
	variantMap["statusText"] = Roster::instance()->statusText(itemStatus);
	variantMap["userhome"] = userhome();
	variantMap["username"] = username();
	
	return variantMap;
}

QVariantMap RosterItem::resourcesVariantMap() const {
	
	QVariantList resourceList;
	foreach (Resource resource, m_resources) {
		if (resource.id.isEmpty()) {
			continue;
		}
		
		QVariantMap map;
		map["id"] = resource.id;
		map["show"] = resource.show;
		map["status"] = resource.status;
		map["priority"] = resource.priority;
		resourceList.append(map);
	}
	
	QVariantMap variantMap;
	variantMap["resources"] = resourceList;
	return variantMap;
}

void RosterItem::unlock() {
	
	if (!m_locked) {
		Logger::Logger::instance()->debug("Trying to unlock an already unlocked roster item.");
	}
	
	m_locked = false;
	m_changedProperties.clear();
}

void RosterItem::lock() {
	
	if (m_locked) {
		Logger::Logger::instance()->debug("Trying to lock an already locked roster item.");
	}
	
	m_locked = true;
	Roster::instance()->emitItemChanged(variantMap(), m_changedProperties);
	m_changedProperties.clear();
}

} // namespace Roster
