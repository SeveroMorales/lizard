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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301 USA
 */

#ifndef PIDGIN_XMPP_DISCO_H
#define PIDGIN_XMPP_DISCO_H

#include "xmppdiscoservice.h"
#include "gtkdisco.h"

#define XMPP_PROTOCOL_ID    "prpl-jabber"
#define NS_DISCO_INFO       "http://jabber.org/protocol/disco#info"
#define NS_DISCO_ITEMS      "http://jabber.org/protocol/disco#items"
#define NS_MUC              "http://jabber.org/protocol/muc"
#define NS_REGISTER         "jabber:iq:register"

#define PLUGIN_ID      "gtk-xmppdisco"
#define PLUGIN_DOMAIN  (g_quark_from_static_string(PLUGIN_ID))

#include <purple.h>
extern PurplePlugin *my_plugin;

void xmpp_disco_start(PidginDiscoList *list);

void xmpp_disco_service_expand(XmppDiscoService *service);
void xmpp_disco_register_service(XmppDiscoService *service);

#endif /* PIDGIN_XMPP_DISCO_H */
