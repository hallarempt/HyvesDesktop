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

#include <windows.h>

#include <QWidget>

#include "SystemSpyWin.h"
#include "SimpleConnectionChecker.h"

namespace SystemSpy {

class SystemSpyWin::WindowsSpyWidget : public QWidget {
	
	public:
		WindowsSpyWidget(SystemSpyWin *spy) :
			QWidget(),
			m_spy(spy),
			m_connectionChecker(new SimpleConnectionChecker(this)) {
			
			create();
			connect(m_connectionChecker, SIGNAL(networkUp()), m_connectionChecker, SLOT(stop()));
			connect(m_connectionChecker, SIGNAL(networkUP()), m_spy, SIGNAL(justAwoken()));
		}
		
		bool winEvent(MSG *msg, long *result) {
			
			if (WM_POWERBROADCAST == msg->message) {
				switch (msg->wParam) {
					case PBT_APMSUSPEND:
						m_connectionChecker->stop();
						emit m_spy->willSleep();
						break;
						
					case PBT_APMRESUMESUSPEND:
						//emit m_spy->justAwoken();
						m_connectionChecker->start(); // wait until network is up again before emitting justAwoken()
						break;
						
					case PBT_APMRESUMECRITICAL:
						emit m_spy->willSleep();
						//emit m_spy->justAwoken();
						m_connectionChecker->start();
						break;
				}
			}
			
			return QWidget::winEvent(msg, result);
		}
		
	private:
		SystemSpyWin *m_spy;
		SimpleConnectionChecker *m_connectionChecker;
};

SystemSpyWin::SystemSpyWin() :
	SystemSpy() {
	
	m_spyWidget = new WindowsSpyWidget(this);
}

SystemSpyWin::~SystemSpyWin() {
	
	delete m_spyWidget;
}

} // namespace SystemSpy
