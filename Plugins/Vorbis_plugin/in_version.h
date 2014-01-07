/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2003 Albert L. Faber
**
** http://cdexos.sourceforge.net/
** http://sourceforge.net/projects/cdexos 
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


# ifndef IN_VERSION_H_INCLUDED
# define IN_VERSION_H_INCLUDED

# define VPI_VERSION_MAJOR	1
# define VPI_VERSION_MINOR	0
# define VPI_VERSION_PATCH	0
# define VPI_VERSION_EXTRA	""

# define VPI_VERSION_STRINGIZE(str)	#str
# define VPI_VERSION_STRING(num)	VPI_VERSION_STRINGIZE(num)

# define VPI_VERSION		VPI_VERSION_STRING(VPI_VERSION_MAJOR) "."  \
				VPI_VERSION_STRING(VPI_VERSION_MINOR) "."  \
				VPI_VERSION_STRING(VPI_VERSION_PATCH)  \
				VPI_VERSION_EXTRA

# define VPI_PUBLISHYEAR	"2003"
# define VPI_AUTHOR			"Albert Faber"
# define VPI_EMAIL			" "

extern char const VPI_version[];
extern char const VPI_copyright[];
extern char const VPI_author[];
extern char const VPI_build[];

# endif
