/*
 * purple
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

#include <config.h>

#include <glib/gi18n-lib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include <time.h>
#include <glib.h>
#include "debug.h"
#include "libc_internal.h"
#include "util.h"
#include <glib/gstdio.h>
#include "util.h"

#ifndef S_ISDIR
# define S_ISDIR(m) (((m)&S_IFDIR)==S_IFDIR)
#endif

/* fcntl.h */
/* This is not a full implementation of fcntl. Update as needed.. */
int wpurple_fcntl(int socket, int command, ...) {

	switch( command ) {
	case F_GETFL:
		return 0;

	case F_SETFL:
	{
		va_list args;
		int val;
		int ret=0;

		va_start(args, command);
		val = va_arg(args, int);
		va_end(args);

		switch( val ) {
		case O_NONBLOCK:
		{
			u_long imode=1;
			ret = ioctlsocket(socket, FIONBIO, &imode);
			break;
		}
		case 0:
		{
			u_long imode=0;
			ret = ioctlsocket(socket, FIONBIO, &imode);
			break;
		}
		default:
			errno = EINVAL;
			return -1;
		}/*end switch*/
		if( ret == SOCKET_ERROR ) {
			errno = WSAGetLastError();
			return -1;
		}
		return 0;
	}
	default:
		purple_debug_warning("wpurple", "wpurple_fcntl: Unsupported command");
		return -1;
	}/*end switch*/
}

/* netdb.h */
struct hostent* wpurple_gethostbyname(const char *name) {
	struct hostent *hp;

	if((hp = gethostbyname(name)) == NULL) {
		errno = WSAGetLastError();
		return NULL;
	}
	return hp;
}
