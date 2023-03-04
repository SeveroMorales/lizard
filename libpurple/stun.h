/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_STUN_H
#define PURPLE_STUN_H

typedef struct _PurpleStunNatDiscovery PurpleStunNatDiscovery;

/**
 * PurpleStunStatus:
 * @PURPLE_STUN_STATUS_UNDISCOVERED: No request has been published
 * @PURPLE_STUN_STATUS_UNKNOWN: No STUN server reachable
 * @PURPLE_STUN_STATUS_DISCOVERING: The request has been sent to the server
 * @PURPLE_STUN_STATUS_DISCOVERED: The server has responded
 *
 * The status of a #PurpleStunNatDiscovery
 */
typedef enum {
	PURPLE_STUN_STATUS_UNDISCOVERED = -1,
	PURPLE_STUN_STATUS_UNKNOWN,
	PURPLE_STUN_STATUS_DISCOVERING,
	PURPLE_STUN_STATUS_DISCOVERED
} PurpleStunStatus;

/**
 * PurpleStunNatDiscovery:
 * @status: The #PurpleStunStatus
 * @publicip: The public ip
 * @servername: The name of the stun server
 * @lookup_time: The time when the lookup occurred
 *
 * A data type representing a STUN lookup.
 */
struct _PurpleStunNatDiscovery {
	PurpleStunStatus status;
	char publicip[16];
	char *servername;
	gint64 lookup_time;
};

typedef void (*PurpleStunCallback) (PurpleStunNatDiscovery *discovery);

G_BEGIN_DECLS

/**
 * purple_stun_discover:
 * @cb: (scope async): The callback to call when the STUN discovery is finished
 *      if the discovery would block. If the discovery is done, this is NOT
 *      called.
 *
 * Starts a NAT discovery. It returns a PurpleStunNatDiscovery if the discovery
 * is already done. Otherwise the callback is called when the discovery is over
 * and NULL is returned.
 *
 * Returns: (transfer none): a #PurpleStunNatDiscovery which includes the public IP and the type
 *          of NAT or NULL if discovery would block
 */
PurpleStunNatDiscovery *purple_stun_discover(PurpleStunCallback cb);

/**
 * purple_stun_init:
 *
 * Initializes the STUN API.  This is called by libpurple and you should not be
 * calling it yourself.
 */
void purple_stun_init(void);

G_END_DECLS

#endif /* PURPLE_STUN_H */
