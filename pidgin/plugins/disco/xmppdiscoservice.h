/* pidgin
 *
 * Pidgin is the legal property of its developers, whose names are too numerous
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

#ifndef PIDGIN_XMPP_DISCO_SERVICE_H
#define PIDGIN_XMPP_DISCO_SERVICE_H

#include <glib-object.h>

#include <purple.h>

#define XMPP_DISCO_TYPE_SERVICE (xmpp_disco_service_get_type())
G_DECLARE_FINAL_TYPE(XmppDiscoService, xmpp_disco_service, XMPP_DISCO, SERVICE,
                     GObject)

#include "gtkdisco.h"

/**
 * XmppDiscoServiceType:
 * @XMPP_DISCO_SERVICE_TYPE_UNSET: An unset service type.
 * @XMPP_DISCO_SERVICE_TYPE_GATEWAY: A registerable gateway to another
 *                                   protocol. An example would be XMPP legacy
 *                                   transports.
 * @XMPP_DISCO_SERVICE_TYPE_DIRECTORY: A directory (e.g. allows the user to
 *                                     search for other users).
 * @XMPP_DISCO_SERVICE_TYPE_CHAT: A chat (multi-user conversation).
 * @XMPP_DISCO_SERVICE_TYPE_PUBSUB_COLLECTION: A pubsub collection (contains
 *                                             nodes).
 * @XMPP_DISCO_SERVICE_TYPE_PUBSUB_LEAF: A pubsub leaf (contains stuff, not
 *                                       nodes).
 * @XMPP_DISCO_SERVICE_TYPE_OTHER: Something else. Do we need more categories?
 *
 * The types of services.
 */
typedef enum {
	XMPP_DISCO_SERVICE_TYPE_UNSET,
	XMPP_DISCO_SERVICE_TYPE_GATEWAY,
	XMPP_DISCO_SERVICE_TYPE_DIRECTORY,
	XMPP_DISCO_SERVICE_TYPE_CHAT,
	XMPP_DISCO_SERVICE_TYPE_PUBSUB_COLLECTION,
	XMPP_DISCO_SERVICE_TYPE_PUBSUB_LEAF,
	XMPP_DISCO_SERVICE_TYPE_OTHER
} XmppDiscoServiceType;

/**
 * XmppDiscoServiceFlags:
 * @XMPP_DISCO_NONE: No flags.
 * @XMPP_DISCO_ADD: Supports an 'add' operation.
 * @XMPP_DISCO_BROWSE: Supports browsing.
 * @XMPP_DISCO_REGISTER: Supports a 'register' operation.
 *
 * The flags of services.
 */
typedef enum { /*< flags >*/
	XMPP_DISCO_NONE = 0x0000,
	XMPP_DISCO_ADD = 0x0001,
	XMPP_DISCO_BROWSE = 0x0002,
	XMPP_DISCO_REGISTER = 0x0004,
} XmppDiscoServiceFlags;

void xmpp_disco_service_register(PurplePlugin *plugin);

XmppDiscoService *xmpp_disco_service_new(PidginDiscoList *list);

PidginDiscoList *xmpp_disco_service_get_list(XmppDiscoService *service);

const char *xmpp_disco_service_get_name(XmppDiscoService *service);
void xmpp_disco_service_set_name(XmppDiscoService *service, const char *name);

const char *xmpp_disco_service_get_description(XmppDiscoService *service);
void xmpp_disco_service_set_description(XmppDiscoService *service, const char *description);

XmppDiscoServiceType xmpp_disco_service_get_service_type(XmppDiscoService *service);
void xmpp_disco_service_set_service_type(XmppDiscoService *service, XmppDiscoServiceType type);

const char *xmpp_disco_service_get_gateway_type(XmppDiscoService *service);
void xmpp_disco_service_set_gateway_type(XmppDiscoService *service, const char *gateway_type);

XmppDiscoServiceFlags xmpp_disco_service_get_flags(XmppDiscoService *service);
void xmpp_disco_service_set_flags(XmppDiscoService *service, XmppDiscoServiceFlags flags);
void xmpp_disco_service_add_flags(XmppDiscoService *service, XmppDiscoServiceFlags flags);
void xmpp_disco_service_remove_flags(XmppDiscoService *service, XmppDiscoServiceFlags flags);

const char *xmpp_disco_service_get_jid(XmppDiscoService *service);
void xmpp_disco_service_set_jid(XmppDiscoService *service, const char *jid);

const char *xmpp_disco_service_get_node(XmppDiscoService *service);
void xmpp_disco_service_set_node(XmppDiscoService *service, const char *node);

gboolean xmpp_disco_service_get_expanded(XmppDiscoService *service);
void xmpp_disco_service_set_expanded(XmppDiscoService *service, gboolean expanded);

char *xmpp_disco_service_get_icon_name(XmppDiscoService *service);

GListModel *xmpp_disco_service_get_child_model(XmppDiscoService *service);
void xmpp_disco_service_add_child(XmppDiscoService *service, XmppDiscoService *child);

#endif /* PIDGIN_XMPP_DISCO_SERVICE_H */
