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

#ifndef VERSION_H
#define VERSION_H

#include <QString>
#include "hyveslib_export.h"

// fix compile bug in old glibc
#ifdef major
#undef major
#undef minor
#endif

/**
 * Data type for version information.
 */
class  HYVESLIB_EXPORT Version {

	public:
		/**
		 * Major version number.
		 */
		int major;

		/**
		 * Minor version number.
		 */
		int minor;

		/**
		 * Patch version number.
		 */
		int patch;

		/**
		 * Default constructor.
		 */
		Version();

		/**
		 * Copy constructor.
		 */
		Version(const Version &version);

		/**
		 * Implicit constructor from QString.
		 *
		 * @param version. Version can consist of up to three parts delimited by dots. If a part cannot
		 * be converted to an integer, the part is replaced by 0.
		 */
		Version(const QString &version);

		/**
		 * Returns the version as a string.
		 *
		 * @return String representation in the form "major.minor.patch".
		 */
		QString toString() const;

		/**
		 * Assignment operator.
		 */
		Version &operator=(const Version &version);

		/**
		 * Assignment operator from QString.
		 */
		Version &operator=(const QString &version);

		/**
		 * Equality operator.
		 */
		bool operator==(const Version &version) const;

		/**
		 * Inequality operator.
		 */
		bool operator!=(const Version &version) const;

		/**
		 * Greater-than or equals operator.
		 */
		bool operator>=(const Version &version) const;

		/**
		 * Greater-than operator
		 */
		bool operator>(const Version &version) const;
};

#endif // VERSION_H
