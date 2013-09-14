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

#ifndef MAINWINDOWBASE_H
#define MAINWINDOWBASE_H

#include <QMainWindow>
#include <QMenuBar>

#include "hyveslib_export.h"
#include "WindowBase.h"

class QString;

namespace WindowManager {

/**
 * Base class for plugin mainwindows. The base mainwindow sets up
 * basic shortcuts and creates a number of default menus.
 *
 * <ul>
 *    <li>File menu (you can retrieve this to change the title)
 *    <li>Edit menu
 *    <li>Window menu
 *    <li>Help menu
 * </ul> 
 *
 * By default, MainWindowBase windows have Qt::WA_DeleteOnClose set to false.
 */
class HYVESLIB_EXPORT MainWindowBase : public QMainWindow, public WindowBase {
	
	Q_OBJECT
	
	public:
		MainWindowBase(const QString &pluginTitle, QWidget *parent = 0, Qt::WindowFlags flags = 0);
		virtual ~MainWindowBase();
		
		/**
		 * Creates a new (sub) menu in the main menu and returns it.
		 *
		 * @param title Title of the new menu.
		 */
		QMenu *menu(const QString &title);
		
		virtual QWebFrame *mainFrame() const { return 0; }
		
	public slots:
		/**
		 * Override this method with the code that needs to happen when
		 * your plugin closes.
		 */
		virtual void signOut() = 0;
		
	private slots:
		void introduction();
		void onlineHelp();
		void privacy();
		void giveFeedback();
		void userConditions();
		void about();
		void showLandingPage();
		void updateWindowMenu();
		
	private:
		void populateMenuBar();
		
	private:
		struct Private;
		Private *const m_d;
};

} // namespace WindowManager

#endif // MAINWINDOWBASE_H
