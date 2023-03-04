/*
 * purple
 *
 * File: libc_interface.h
 *
 * Copyright (C) 2002-2003, Herman Bloggs <hermanator12002@yahoo.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 */
#ifndef PURPLE_WIN32_LIBC_INTERFACE_H
#define PURPLE_WIN32_LIBC_INTERFACE_H

#include <config.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>
#include <errno.h>
#include "libc_internal.h"
#include <glib.h>
#include "glibcompat.h"

G_BEGIN_DECLS

#ifdef _MSC_VER
#define S_IRUSR S_IREAD
#define S_IWUSR S_IWRITE
#define S_IXUSR S_IEXEC

#define S_ISDIR(m)	 (((m)&S_IFDIR)==S_IFDIR)

#define F_OK 0
#endif

/* fcntl.h */
#define fcntl( fd, command, ... ) \
wpurple_fcntl( fd, command, ##__VA_ARGS__ )

/* netdb.h */
#define gethostbyname( name ) \
wpurple_gethostbyname( name )

/* stdio.h */
#if !defined(__MINGW64_VERSION_MAJOR) || __MINGW64_VERSION_MAJOR < 3 || \
	!defined(IS_WIN32_CROSS_COMPILED)
#  undef vsnprintf
#  define vsnprintf _vsnprintf
#endif

G_END_DECLS

#endif /* PURPLE_WIN32_LIBC_INTERFACE_H */
