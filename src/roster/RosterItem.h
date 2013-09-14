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

#ifndef ROSTERITEM_H
#define ROSTERITEM_H

#include <QStringList>
#include <QVariantMap>

#include "hyveslib_export.h"

namespace Roster {

struct HYVESLIB_EXPORT Resource {
	
	enum ClientResourceBits {
		siteClient = 0x01,
		webClient = 0x02,
		desktopClient = 0x04,
		otherClient = 0x08
	};
	
	QString id;
	QString show;
	QString status;
	int priority;
	
	Resource() :
		priority(0) {
	}
	
	Resource(const QString &id, const QString &show, const QString &status, int priority) :
		id(id),
		show(show),
		status(status),
		priority(priority) {
	}
	
	Resource &operator=(const Resource &resource) {
		
		id = resource.id;
		show = resource.show;
		status = resource.status;
		priority = resource.priority;
		
		return *this;
	}
	
	bool operator==(const Resource &resource) {
		
		return (id == resource.id);
	}
	
	const static Resource null;
};

class HYVESLIB_EXPORT RosterItem {
	
	public:
		RosterItem(const QString &jid);
		
		const QString &jid() const;
		
		const QString &show() const;
		const QString &status() const;
		
		const QString &userhome() const;
		const QString &username() const;
		
		bool hasAvailableResource() const;
		const Resource &resource() const;
		const Resource &resource(const QString &id);
		const QList<Resource> &resources() const;
		void addResource(const Resource &resource);
		void removeResource(const QString &id);
		int clientResourceBits() const;
				
		const QStringList &groups() const;
		void addGroup(const QString &group);
		void removeGroup(const QString &group);
		
		QString property(const QString &key) const;
		const QMap<QString, QString> &properties() const;
		void setProperty(const QString &key, const QString &value);
		void removeProperty(const QString &key);
		
		/**
		 * Returns the item with all its properties in a variant map.
		 */
		QVariantMap variantMap() const;
		
		/**
		 * Returns the item's resources in a variant map.
		 */
		QVariantMap resourcesVariantMap() const;
		
		/**
		 * Unlocks a roster item so that it may be modified.
		 *
		 * If a roster item has not been unlocked, all the setters in
		 * this class will refuse to work.
		 */
		void unlock();
		
		/**
		 * Locks a roster item after changes have been made.
		 *
		 * At this point, the itemChanged() (or itemAdded() if it's the
		 * first time the object is modified) signal is emitted by the
		 * Roster class.
		 */
		void lock();
		
	private:
		QString m_jid;
		QString m_username;
		QString m_userhome;
		
		QList<Resource> m_resources;
		QStringList m_groups;
		QMap<QString, QString> m_properties;
		
		bool m_locked;
		QVariantMap m_changedProperties;
};

} // namespace Roster

#endif // ROSTERITEM_H
