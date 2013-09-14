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

#include <QStringList>

#include "Version.h"

Version::Version() :
	major(0),
	minor(0),
	patch(0) {
}

Version::Version(const Version &version) :
	major(version.major),
	minor(version.minor),
	patch(version.patch) {
}

Version::Version(const QString &version) {
	
	QStringList parts = version.split('.');
	major = parts.size() > 0 ? parts[0].toInt() : 0;
	minor = parts.size() > 1 ? parts[1].toInt() : 0;
	patch = parts.size() > 2 ? parts[2].toInt() : 0;
}

QString Version::toString() const {
	
	return QString::number(major) + "." +
	       QString::number(minor) + "." +
	       QString::number(patch);
}

Version &Version::operator=(const Version &version) {
	
	major = version.major;
	minor = version.minor;
	patch = version.patch;
	return *this;
}

Version &Version::operator=(const QString &version) {
	
	return operator=(Version(version));
}

bool Version::operator==(const Version &version) const {
	
	return (major == version.major && minor == version.minor && patch == version.patch);
}

bool Version::operator!=(const Version &version) const {
	
	return (major != version.major || minor != version.minor || patch != version.patch);
}

bool Version::operator>=(const Version &version) const {
	
	return (major >  version.major) ||
	       (major == version.major && minor >  version.minor) ||
	       (major == version.major && minor == version.minor && patch >= version.patch);
}

bool Version::operator>(const Version &version) const {
	
	return (major >  version.major) ||
	       (major == version.major && minor >  version.minor) ||
	       (major == version.major && minor == version.minor && patch > version.patch);
}
