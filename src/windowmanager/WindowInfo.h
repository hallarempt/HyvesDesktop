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

#ifndef WINDOWINFO_H
#define WINDOWINFO_H

#include <QRect>
#include <QString>

#include "hyveslib_export.h"

class QWidget;

namespace WindowManager {

class HYVESLIB_EXPORT WindowInfo {

	public:
		WindowInfo(int id, QWidget *window);
		WindowInfo(const WindowInfo &info);
		virtual ~WindowInfo();
		
		/**
		 * Returns the Window ID.
		 */
		int id() const;
		
		/**
		 * Returns whether the window is the active window.
		 */
		bool active() const;
		
		/**
		 * Returns the left position of the window.
		 */
		int left() const;
		
		/**
		 * Returns the top position of the window.
		 */
		int top() const;
		
		/**
		 * Returns the width of the window.
		 */
		int width() const;
		
		/**
		 * Returns the height of the window.
		 */
		int height() const;
		
		/**
		 * Returns the width of the contents of the window.
		 */
		int innerWidth() const;
		
		/**
		 * Returns the height of the contents of the window.
		 */
		int innerHeight() const;
		
		/**
		 * Returns whether the window is visible.
		 */
		bool visible() const;
		
		/**
		 * Returns the window state, either "normal" or "minimized".
		 */
		QString windowState() const;
		
		/**
		 * Returns the geometry of the screen of the window.
		 */
		QRect screenGeometry() const;
		
		/**
		 * Returns the geometry of the available space in the screen of the
		 * window.
		 */
		QRect availableGeometry() const;
		
		/**
		 * Returns the WindowInfo object as a JSON string.
		 */
		QString toJson() const;
		
		/**
		 * Assignment operator.
		 */
		WindowInfo &operator=(const WindowInfo &info);
		
	private:
		int m_id;
		QWidget *m_window;
};

} // namespace WindowManager

#endif // WINDOWINFO_H
