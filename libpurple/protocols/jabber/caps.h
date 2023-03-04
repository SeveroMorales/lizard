/*
 * purple - Jabber Protocol Plugin
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 */

#ifndef PURPLE_JABBER_CAPS_H
#define PURPLE_JABBER_CAPS_H

typedef struct _JabberCapsClientInfo JabberCapsClientInfo;

#include "jabber.h"

/* Implementation of XEP-0115 - Entity Capabilities */

typedef struct {
	const char *node;
	const char *ver;
	const char *hash;
} JabberCapsTuple;

struct _JabberCapsClientInfo {
	GList *identities; /* JabberIdentity */
	GList *features; /* char * */
	GList *forms; /* PurpleXmlNode * */

	const JabberCapsTuple tuple;
};

typedef void (*jabber_caps_get_info_cb)(JabberCapsClientInfo *info, gpointer user_data);

void jabber_caps_init(void);
void jabber_caps_uninit(void);

/**
 * Main entity capabilities function to get the capabilities of a contact.
 *
 * The callback will be called synchronously if we already have the
 * capabilities for the specified (node,ver,hash).
 */
void jabber_caps_get_info(JabberStream *js, const char *who, const char *node,
                          const char *ver, const char *hash,
                          jabber_caps_get_info_cb cb,
                          gpointer user_data);

/**
 *	Takes a JabberCapsClientInfo pointer and returns the caps hash according to
 *	XEP-0115 Version 1.5.
 *
 *	@param info A JabberCapsClientInfo pointer.
 *	@param hash_type GChecksumType to be used. Either sha-1 or md5.
 *	@return		The base64 encoded SHA-1 hash; must be freed by caller
 */
gchar *jabber_caps_calculate_hash(JabberCapsClientInfo *info,
                                  GChecksumType hash_type);

/**
 *  Calculate SHA1 hash for own featureset.
 */
void jabber_caps_calculate_own_hash(JabberStream *js);

/** Get the current caps hash.
 * 	@ret hash
**/
const gchar* jabber_caps_get_own_hash(JabberStream *js);

/**
 *  Broadcast a new calculated hash using a <presence> stanza.
 */
void jabber_caps_broadcast_change(void);

/**
 * Parse the <query/> element from an IQ stanza into a JabberCapsClientInfo
 * struct.
 *
 * Exposed for tests
 *
 * @param query The 'query' element from an IQ reply stanza.
 * @returns A JabberCapsClientInfo struct, or NULL on error
 */
JabberCapsClientInfo *jabber_caps_parse_client_info(PurpleXmlNode *query);

/**
 * Release memory of a JabberCapsClientInfo struct
 * returned by jabber_caps_parse_client_info.
 *
 * Exposed for tests
 *
 * @param info The info object to free.
 */
void jabber_caps_client_info_destroy(JabberCapsClientInfo *info);

#endif /* PURPLE_JABBER_CAPS_H */
