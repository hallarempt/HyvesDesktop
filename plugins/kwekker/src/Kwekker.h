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

#ifndef KWEKKER_H
#define KWEKKER_H

#include <QObject>

#include "plugger/CXXPluginInterface.h"

namespace Kwekker {

class Kwekker : public QObject, public Plugger::CXXPluginInterface {
	
	Q_OBJECT
	Q_INTERFACES(Plugger::CXXPluginInterface)
	
	public:
		Kwekker();
		virtual ~Kwekker();
		
		virtual bool run();
		virtual bool activate();
		virtual bool handleMessage(const QStringList &message);
		virtual bool acceptStopEvent();
		
	private:
		struct Private;
		Private *const m_d;
		
	private slots:
		void mainWindowDestroyed();
};

} // namespace Kwekker

#endif // KWEKKER_H
