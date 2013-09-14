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

#ifndef ROSTER_H
#define ROSTER_H

#include <QObject>
#include <QVariantMap>

#include "hyveslib_export.h"

namespace Roster {

class RosterItem;

class HYVESLIB_EXPORT Roster : public QObject {
	
	Q_OBJECT
	
	public:
		static Roster *instance();
		static void destroy();
		
		/**
		 * Sets the roster list to be initialized.
		 */
		void setInitialized();
		
		/**
		 * Sets the full JID of the current user.
		 *
		 * By setting the own JID, an item for it will be created if it
		 * doesn't exist yet, and the specified resource will also be
		 * created in the item if it doesn't exist yet.
		 *
		 * @param jid Full JID.
		 */
		void setOwnJid(const QString &jid);
		
		/**
		 * Returns a list with all the JIDs in the roster.
		 *
		 * @return A list of all JIDs.
		 */
		QStringList itemJids() const;
		
		/**
		 * Returns a specific roster item.
		 *
		 * If the item that's requested does not exist, it is created
		 * automatically.
		 *
		 * @param jid JID of the roster item to return.
		 * @return The specified roster item.
		 */
		RosterItem *item(const QString &jid);
		
		/**
		 * Removes a specific roster item.
		 *
		 * @param jid JID of the roster item to remove.
		 */
		void removeItem(const QString &jid);
		
		/**
		 * Clears the entire roster list, removing all items and
		 * resetting the owner JID.
		 */
		void clear();
		
		/**
		 * Emits the itemChanged() signal.
		 *
		 * @param item The item that has been changed.
		 * @param changedProperties Map of all the properties that have
		 *                          been changed, containing their
		 *                          previous values.
		 */
		void emitItemChanged(const QVariantMap &item, const QVariantMap &changedProperties);
		
	public slots:
		/**
		 * Returns whether the roster list has been fully initialized.
		 */
		bool isInitialized() const;
		
		/**
		 * Returns the bare JID of the current user.
		 */
		QString ownBareJid() const;
		
		/**
		 * Returns the resource of the current user.
		 */
		QString ownResource() const;
		
		/**
		 * Returns the full JID of the current user.
		 */
		QString ownFullJid() const;
		
		/**
		 * Queries whether a specific roster item exists.
		 *
		 * @param jid JID of the roster item to query for.
		 * @return True if the item exists, or false otherwise.
		 */
		bool hasItem(const QString &jid) const;
		
		/**
		 * Returns a specific roster item, encoded in JSON.
		 *
		 * Unlike item(), this method will not create an item if it does
		 * not exist yet.
		 *
		 * @param jid JID of the roster item to return.
		 * @return The specified roster item, encoded in JSON.
		 */
		QString itemJSON(const QString &jid);
		
		/**
		 * Returns all roster items, encoded in JSON.
		 */
		QString itemsJSON();
		
		/**
		 * Returns the roster item of the current user, encoded in JSON.
		 */
		QString ownItemJSON();
		
		/**
		 * Makes sure an item exists, and creates it if it doesn't.
		 *
		 * @param jid JID of the item to check, and optionally create.
		 * @return true if the item had to created, false otherwise.
		 */
		bool ensureItem(const QString &jid);
		
		/**
		 * Returns the resources of a roster item, encoded in JSON.
		 *
		 * @param jid JID of the roster item to return the resources of.
		 * @return The item's resources, encoded in JSON.
		 */
		QString itemResourcesJSON(const QString &jid);
		
		/**
		 * Returns the resources of the roster item of the current user,
		 * encoded in JSON.
		 */
		QString ownItemResourcesJSON();
		
		/**
		 * Sets a list of properties on an item.
		 *
		 * @param jid JID of the item to modify.
		 * @param propertiesJSON The properties with their new values,
		 *                       encoded in JSON.
		 */
		void setItemProperties(const QString &jid, const QString &propertiesJSON);
		
		/**
		 * Returns whether a given status is recognized.
		 */
		bool statusExists(const QString &status) const;
		
		/**
		 * Returns the show property used for a given status.
		 */
		QString statusShow(const QString &status) const;
		
		/**
		 * Returns the text for a given status.
		 */
		QString statusText(const QString &status) const;
		
		/**
		 * Sets the text for a given status.
		 */
		void setStatusText(const QString &status, const QString &text);
		
		/**
		 * Sets the status for the current user.
		 */
		void setOwnStatus(const QString &status);
		
	signals:
		/**
		 * Emitted when the roster list is fully loaded.
		 */
		void initialized();
		
		/**
		 * Emitted when a roster item has been changed (or a new one is
		 * added after the roster list has initialized).
		 *
		 * @param item The roster item as a variant map.
		 * @param changedProperties Map of all the properties that have
		 *                          been changed, containing their
		 *                          previous values.
		 */
		void itemChanged(const QVariantMap &item, const QVariantMap &changedProperties);
		
	private:
		static Roster *s_instance;
		
		struct Private;
		Private *const m_d;
		
		Roster();
		virtual ~Roster();
};

} // namespace Roster

#endif // ROSTER_H
