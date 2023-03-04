/*
 * purple
 *
 * File: libc_internal.h
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
#ifndef PURPLE_WIN32_LIBC_INTERNAL
#define PURPLE_WIN32_LIBC_INTERNAL
#include <glib.h>

G_BEGIN_DECLS

/* netdb.h */
struct hostent* wpurple_gethostbyname(const char *name);

/* fcntl.h */
int wpurple_fcntl(int socket, int command, ...);
#define F_GETFL 3
#define F_SETFL 4
#define O_NONBLOCK 04000

G_END_DECLS

#endif /* PURPLE_WIN32_LIBC_INTERNAL */
