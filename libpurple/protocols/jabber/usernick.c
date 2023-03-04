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

#include <glib/gi18n-lib.h>

#include <purple.h>

#include "usernick.h"
#include "pep.h"
#include <string.h>

static void jabber_nick_cb(JabberStream *js, const char *from, PurpleXmlNode *items) {
	/* it doesn't make sense to have more than one item here, so let's just pick the first one */
	PurpleXmlNode *item = purple_xmlnode_get_child(items, "item");
	JabberBuddy *buddy = jabber_buddy_find(js, from, FALSE);
	PurpleXmlNode *nick;
	char *nickname = NULL;

	/* ignore the nick of people not on our buddy list */
	if (!buddy || !item)
		return;

	nick = purple_xmlnode_get_child_with_namespace(item, "nick", "http://jabber.org/protocol/nick");
	if (!nick)
		return;
	nickname = purple_xmlnode_get_data(nick);
	purple_serv_got_alias(js->gc, from, nickname);
	g_free(nickname);
}

static void do_nick_set(JabberStream *js, const char *nick) {
	PurpleXmlNode *publish, *nicknode;

	publish = purple_xmlnode_new("publish");
	purple_xmlnode_set_attrib(publish,"node","http://jabber.org/protocol/nick");
	nicknode = purple_xmlnode_new_child(purple_xmlnode_new_child(publish, "item"), "nick");
	purple_xmlnode_set_namespace(nicknode, "http://jabber.org/protocol/nick");

	if(nick && nick[0] != '\0')
		purple_xmlnode_insert_data(nicknode, nick, -1);

	jabber_pep_publish(js, publish);
	/* publish is freed by jabber_pep_publish -> jabber_iq_send -> jabber_iq_free
		(yay for well-defined memory management rules) */
}

static void
do_nick_got_own_nick_cb(JabberStream *js, G_GNUC_UNUSED const char *from,
                        PurpleXmlNode *items)
{
	char *oldnickname = NULL;
	PurpleXmlNode *item = NULL;

	if (items)
		item = purple_xmlnode_get_child(items,"item");

	if(item) {
		PurpleXmlNode *nick = purple_xmlnode_get_child_with_namespace(item,"nick","http://jabber.org/protocol/nick");
		if(nick)
			oldnickname = purple_xmlnode_get_data(nick);
	}

	purple_request_input(js->gc, _("Set User Nickname"), _("Please specify a new nickname for you."),
		_("This information is visible to all contacts on your contact list, so choose something appropriate."),
		oldnickname, FALSE, FALSE, NULL, _("Set"), G_CALLBACK(do_nick_set), _("Cancel"), NULL,
		purple_request_cpar_from_connection(js->gc), js);
	g_free(oldnickname);
}

static void
do_nick_set_nick(G_GNUC_UNUSED GSimpleAction *action, GVariant *parameter,
                 G_GNUC_UNUSED gpointer data)
{
	const char *account_id = NULL;
	PurpleAccountManager *manager = NULL;
	PurpleAccount *account = NULL;
	PurpleConnection *connection = NULL;
	JabberStream *js = NULL;

	if(!g_variant_is_of_type(parameter, G_VARIANT_TYPE_STRING)) {
		g_critical("XMPP Set Nickname action parameter is of incorrect type %s",
		           g_variant_get_type_string(parameter));
	}

	account_id = g_variant_get_string(parameter, NULL);
	manager = purple_account_manager_get_default();
	account = purple_account_manager_find_by_id(manager, account_id);
	connection = purple_account_get_connection(account);
	js = purple_connection_get_protocol_data(connection);

	/* since the nickname might have been changed by another resource of this account, we always have to request the old one
		from the server to present as the default for the new one */
	jabber_pep_request_item(js, NULL, "http://jabber.org/protocol/nick", NULL, do_nick_got_own_nick_cb);
}

void jabber_nick_init(void) {
	jabber_add_feature("http://jabber.org/protocol/nick", jabber_pep_namespace_only_when_pep_enabled_cb);
	jabber_pep_register_handler("http://jabber.org/protocol/nick", jabber_nick_cb);
}

void
jabber_nick_add_action_entries(GSimpleActionGroup *group) {
	GActionEntry entries[] = {
		{
			.name = "set-nickname",
			.activate = do_nick_set_nick,
			.parameter_type = "s",
		},
	};
	gsize nentries = G_N_ELEMENTS(entries);

	g_action_map_add_action_entries(G_ACTION_MAP(group), entries, nentries,
	                                NULL);
}

void
jabber_nick_append_menu(GMenu *menu) {
	GMenuItem *item = NULL;

	item = g_menu_item_new(_("Set Nickname..."), "prpl-xmpp.set-nickname");
	g_menu_item_set_attribute(item, PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET, "s",
	                          "account");
	g_menu_append_item(menu, item);
	g_object_unref(item);
}
