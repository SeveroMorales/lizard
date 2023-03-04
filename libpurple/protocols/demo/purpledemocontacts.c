/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <json-glib/json-glib.h>

#include "purpledemocontacts.h"

#include "purpledemoresource.h"

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_demo_protocol_load_status(PurpleAccount *account,
                                 G_GNUC_UNUSED PurpleGroup *group,
                                 G_GNUC_UNUSED PurpleMetaContact *contact,
                                 PurpleBuddy *buddy, JsonObject *buddy_object)
{
	JsonObject *status_object = NULL;
	const gchar *id = NULL;

	if(!json_object_has_member(buddy_object, "status")) {
		return;
	}

	status_object = json_object_get_object_member(buddy_object, "status");

	if(json_object_has_member(status_object, "id")) {
		id = json_object_get_string_member(status_object, "id");
	}

	if(id != NULL) {
		if(json_object_has_member(status_object, "message")) {
			const gchar *message = NULL;

			message = json_object_get_string_member(status_object, "message");

			purple_protocol_got_user_status(account,
			                                purple_buddy_get_name(buddy),
			                                id,
			                                "message", message,
			                                NULL);
		} else {
			purple_protocol_got_user_status(account,
			                                purple_buddy_get_name(buddy),
			                                id,
			                                NULL);
		}

		if(json_object_has_member(status_object, "idle")) {
			PurplePresence *presence = NULL;
			GDateTime *now = NULL;
			GDateTime *idle_since = NULL;
			gint idle_minutes = 0;

			idle_minutes = json_object_get_int_member(status_object, "idle");
			now = g_date_time_new_now_local();
			idle_since = g_date_time_add_minutes(now, -1 * idle_minutes);

			presence = purple_buddy_get_presence(buddy);
			purple_presence_set_idle(presence, TRUE, idle_since);

			g_date_time_unref(idle_since);
			g_date_time_unref(now);
		}
	}
}

static void
purple_demo_protocol_load_icon(PurpleAccount *account, const gchar *name)
{
	gchar *path = NULL;
	GBytes *icon = NULL;
	gpointer icon_data = NULL;
	gsize icon_len = 0;

	path = g_strdup_printf("/im/pidgin/purple/demo/buddy_icons/%s.png", name);
	icon = g_resource_lookup_data(purple_demo_get_resource(), path, G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
	g_free(path);

	if(icon == NULL) {
		/* No stored icon. */
		return;
	}

	icon_data = g_bytes_unref_to_data(icon, &icon_len);
	purple_buddy_icons_set_for_user(account, name, icon_data, icon_len, NULL);
}

static void
purple_demo_protocol_load_buddies(PurpleAccount *account, PurpleGroup *group,
                                  PurpleMetaContact *contact, GList *buddies)
{
	while(buddies != NULL) {
		JsonNode *buddy_node = NULL;
		JsonObject *buddy_object = NULL;
		const gchar *name = NULL, *alias = NULL;

		buddy_node = (JsonNode *)buddies->data;
		buddy_object = json_node_get_object(buddy_node);

		if(json_object_has_member(buddy_object, "name")) {
			name = json_object_get_string_member(buddy_object, "name");
		}

		if(json_object_has_member(buddy_object, "alias")) {
			alias = json_object_get_string_member(buddy_object, "alias");
		}

		if(name != NULL) {
			PurpleBuddy *buddy = NULL;

			buddy = purple_blist_find_buddy(account, name);
			if(buddy == NULL) {
				buddy = purple_buddy_new(account, name, alias);
				purple_blist_add_buddy(buddy, contact, group, NULL);
			}

			purple_demo_protocol_load_icon(account, name);
			purple_demo_protocol_load_status(account, group, contact, buddy,
			                                 buddy_object);
			if (purple_strequal(name, "Echo")) {
				purple_protocol_got_media_caps(account, name);
			}
		}

		buddies = g_list_delete_link(buddies, buddies);
	}
}

static void
purple_demo_protocol_load_contacts(PurpleAccount *account, PurpleGroup *group,
                                   GList *contacts)
{
	while(contacts != NULL) {
		PurpleMetaContact *contact = NULL;
		JsonNode *contact_node = NULL;
		JsonObject *contact_object = NULL;

		contact_node = (JsonNode *)contacts->data;
		contact_object = json_node_get_object(contact_node);

		contact = purple_meta_contact_new();

		/* Set the contact's alias if one was specified. */
		if(json_object_has_member(contact_object, "alias")) {
			const gchar *alias = NULL;

			alias = json_object_get_string_member(contact_object, "alias");

			purple_meta_contact_set_alias(contact, alias);
		}

		/* Add the contact to the group. */
		purple_blist_add_contact(contact, group, NULL);

		/* Finally add the buddies */
		if(json_object_has_member(contact_object, "buddies")) {
			JsonArray *buddies_array = NULL;
			GList *buddies = NULL;

			buddies_array = json_object_get_array_member(contact_object,
			                                             "buddies");
			buddies = json_array_get_elements(buddies_array);

			purple_demo_protocol_load_buddies(account, group, contact,
			                                  buddies);
		}

		contacts = g_list_delete_link(contacts, contacts);
	}
}

static void
purple_demo_protocol_load_groups(PurpleAccount *account,
                                 JsonObject *root_object)
{
	PurpleGroup *last = NULL;
	GList *groups = NULL;

	/* Get the members of the root object, this is our list of group names. */
	groups = json_object_get_members(root_object);

	while(groups != NULL) {
		PurpleGroup *group = NULL;
		JsonArray *group_array = NULL;
		GList *contacts = NULL;
		const gchar *group_name = (const gchar *)groups->data;

		/* Add each group according to the json file. */
		group = purple_group_new(group_name);
		purple_blist_add_group(group, PURPLE_BLIST_NODE(last));

		/* Now get the contacts and add them. */
		group_array = json_object_get_array_member(root_object, group_name);
		contacts = json_array_get_elements(group_array);

		purple_demo_protocol_load_contacts(account, group, contacts);

		groups = g_list_delete_link(groups, groups);

		last = group;
	}
}

/******************************************************************************
 * Local Exports
 *****************************************************************************/
void
purple_demo_contacts_load(PurpleAccount *account) {
	GInputStream *istream = NULL;
	GError *error = NULL;
	JsonParser *parser = NULL;
	JsonNode *root_node = NULL;
	JsonObject *root_object = NULL;

	/* get a stream to the contacts.json resource */
	istream = g_resource_open_stream(purple_demo_get_resource(),
	                                 "/im/pidgin/purple/demo/contacts.json",
	                                 G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);

	/* create our parser */
	parser = json_parser_new();

	if(!json_parser_load_from_stream(parser, istream, NULL, &error)) {
		g_critical("%s", error->message);
	}

	/* Load our data */
	root_node = json_parser_get_root(parser);
	root_object = json_node_get_object(root_node);

	/* Load the groups! */
	purple_demo_protocol_load_groups(account, root_object);

	/* Clean up everything else... */
	g_clear_object(&parser);

	g_input_stream_close(istream, NULL, NULL);
	g_object_unref(istream);
}
