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

#ifndef HYVESLIB_EXPORT_H
#define HYVESLIB_EXPORT_H

#include "hyves_macros.h"


/* We use _WIN32/_WIN64 instead of Q_OS_WIN so that this header can be used from C files too */
#if defined(_WIN32) || defined(_WIN64)
# ifndef HYVESLIB_STATIC
#  ifdef HYVESLIB_MAKEDLL
#   define HYVESLIB_EXPORT HYVES_EXPORT
#  else
#   define HYVESLIB_EXPORT HYVES_IMPORT
#  endif
# endif
#endif
#ifndef HYVESLIB_EXPORT
# define HYVESLIB_EXPORT HYVES_EXPORT
#endif

#endif
