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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <mach/mach_init.h>
#include <mach/mach_interface.h>
#include <mach/mach_port.h>

#include <IOKit/pwr_mgt/IOPMLib.h>
#include <IOKit/IOMessage.h>

#include "SystemSpyOsx.h"

namespace SystemSpy {

io_connect_t root_port;

void sleepCallBack(void *systemSpy, io_service_t, natural_t messageType, void *messageArgument) {
	
	SystemSpyOSX *spy = static_cast<SystemSpyOSX *>(systemSpy);
	
	switch (messageType) {
		case kIOMessageSystemWillSleep:
			spy->emitSleep();
			IOAllowPowerChange(root_port, (long) messageArgument);
			break;
			
		case kIOMessageCanSystemSleep:
			spy->emitSleep();
			IOAllowPowerChange(root_port, (long) messageArgument);
			break;
			
		case kIOMessageSystemHasPoweredOn:
			spy->emitAwake();
			break;
	}
}

SystemSpyOSX::SystemSpyOSX() :
	SystemSpy() {
	
	// initialize sleep callback
	IONotificationPortRef notify;
	io_object_t anIterator;
	
	root_port = IORegisterForSystemPower(this, &notify, sleepCallBack, &anIterator);
	
	if (!root_port) {
		log("IORegisterForSystemPower failed.");
	} else {
		CFRunLoopAddSource(CFRunLoopGetCurrent(), IONotificationPortGetRunLoopSource(notify), kCFRunLoopCommonModes);
	}
}

SystemSpyOSX::~SystemSpyOSX() {
}

void SystemSpyOSX::emitSleep() {
	
	emit willSleep();
}

void SystemSpyOSX::emitAwake() {
	
	emit justAwoken();
}

} // namespace SystemSpy
