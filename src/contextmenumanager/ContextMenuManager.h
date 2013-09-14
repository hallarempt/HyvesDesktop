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

#ifndef CONTEXTMENUMANAGER_H
#define CONTEXTMENUMANAGER_H

#include <QObject>
#include <QVariantMap>

#include "hyveslib_export.h"

class QMenu;
class QPoint;
class QString;

namespace ContextMenuManager {

/**
 * The context menu manager handles context menu requests from windows. It
 * some default context menu items. If we have a development build, we also
 * add the webkit inspector items.
 *
 * Usage from javascript:
 *
 * <ul>
 * <li> get the ContextMenuManager object
 * <li> connect to the menuOptionClicked signal to a slot of your choice
 * <li> create the context menu
 * <li> add items to the context menu item
 * <li> in the oncontextmenu javascript event, call customContextMenu()
 * </ul>
 *
 * It is, of course, also possible to do the menu creation in the oncontextmenu.
 * event.
 *
 * NOTE: do not forget to return false from your oncontextmenu event, otherwise
 * the native menu of webkit will popup.
 *
 */
class HYVESLIB_EXPORT ContextMenuManager : public QObject {
	
	Q_OBJECT
	
	public:
		static ContextMenuManager *instance();
		static void destroy();
		
	signals:
		void contextMenuOptionClicked(const QString &key);
		
	public slots:
		/**
		 * Called whenever we want to show a temporary menu with the specified
		 * set of menuitems. The menu is discarded as soon as it disappears
		 *
		 * @param windowId the window that this menu is for
		 * @menuDefinition A nested variantmap that describes a menu of the form
		 *
		 * { menutext { itemtext, key } }
		 *
		 * if the menutext is separator, a separator will be inserted.
		 *
		 * @param pos the position of the menu in screen coordinates
		 * @return the key of the menuitem that was chosen
		 */
		QString showTemporaryMenu(int windowId, const QVariantMap &menuDefinition, const QPoint &pos);
		
		/**
		 * Same as above, but easier to call from javascript.
		 */
		QString showTemporaryMenuJSON(int windowId, const QString& menuDefinitionJSON, int x, int y);
		
	private:
		static ContextMenuManager *s_instance;
		
		struct Private;
		Private *const m_d;
		
		ContextMenuManager(QObject *parent = 0);
		virtual ~ContextMenuManager();
		
		void constructMenu(QMenu *menu, const QVariantMap &menuDefinition);
};

} // namespace ContextMenuManager

#endif // CONTEXTMENUMANAGER_H
