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

#ifndef PLAYER_H
#define PLAYER_H

#include <QtCore>

#include "hyveslib_export.h"

namespace Player {

/**
 * The Player is a singleton that makes it easy to play a sound.
 */
class HYVESLIB_EXPORT Player : public QObject {
	
	Q_OBJECT
	
	public:
		static Player *instance();
		static void destroy();
		
	public slots:
		/**
		 * Play the specified sound. This works whether this sound
		 * is a local file a resource file (filename starts with a colon)
		 * or an url.
		 */
		void playSound(const QString &soundResource);
		
	public:
		QString translateResource(const QString &soundResource);
		
	signals:
		/**
		 * Emitted when a sound is played.
		 *
		 * This signal is only used for unit testing.
		 */
		void soundPlayed(const QString &soundResource);
		
	private:
		static Player *s_instance;
		
		struct Private;
		Private *const m_d;
		
		Player();
		virtual ~Player();
};

} // namespace Player

#endif // PLAYER_H
