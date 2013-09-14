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

#include <QApplication>

#include "logger/Logger.h"

#include "Kwekker.h"
#include "MainWindow.h"

namespace Kwekker {

struct Kwekker::Private {
	Logger::Logger *log;
	
	MainWindow *mainWindow;
	
	Private() :
		log(0),
		mainWindow(0) {
	}
	
	~Private() {
		
		delete mainWindow;
		delete log;
	}
};

Kwekker::Kwekker() :
	QObject(),
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("Kwekker");
}

Kwekker::~Kwekker() {
	
	delete m_d;
}

bool Kwekker::run() {
	
	m_d->mainWindow = new MainWindow();
	connect(m_d->mainWindow, SIGNAL(destroyed()), SLOT(mainWindowDestroyed()));
	
	m_d->mainWindow->show();
	
	return true;
}

bool Kwekker::activate() {
	
	if (m_d->mainWindow) {
		m_d->mainWindow->show();
		m_d->mainWindow->showNormal();
		m_d->mainWindow->raise();
		m_d->mainWindow->activateWindow();
		QApplication::setActiveWindow(m_d->mainWindow);
		return true;
	}
	
	return false;
}

bool Kwekker::handleMessage(const QStringList &args) {

	Q_UNUSED(args);
	return false;
}

bool Kwekker::acceptStopEvent() {
	
	return true;
}

void Kwekker::mainWindowDestroyed() {
	
	m_d->mainWindow = 0;
	
	deleteLater();
}

} // namespace Kwekker

Q_EXPORT_PLUGIN2(Kwekker, Kwekker::Kwekker)
