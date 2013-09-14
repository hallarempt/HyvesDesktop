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

#include <QtGui>
#include <QSound>


#include "extender/Extender.h"
#include "logger/Logger.h"
#include "Player.h"

namespace Player {

Player *Player::s_instance = 0;

struct Player::Private {
	
	Logger::Logger *log;
	
	Private() : 
		log(0) {
		
	}
	
	~Private() {
	}
};

Player *Player::instance() {
	
	if (s_instance == 0) {
		s_instance = new Player();
	}
	
	return s_instance;
}

void Player::destroy() {
	
	delete s_instance;
	s_instance = 0;
}

void Player::playSound(const QString &soundResource) {
	
	QString sound = translateResource(soundResource);
	
	m_d->log->debug(QString("Playing sound: %1.").arg(sound));
	QSound::play(sound);
	
	emit soundPlayed(soundResource);
}

QString Player::translateResource(const QString &soundResource) {
	
	QString sound = soundResource;
	if (soundResource.startsWith(':')) {
		sound = sound.toLower().remove(0, 2);
		sound += ".wav";
	}
	
	return sound;
}

Player::Player() :
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("Player");
	
	Extender::Extender::instance()->registerObject("player", this);
}

Player::~Player() {
	
	Extender::Extender::instance()->unregisterObject("player");
	
	delete m_d;
}

} // namespace Player
