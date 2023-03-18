/*
 * pidgin
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include "pidgin/pidginmooddialog.h"

#include <glib/gi18n-lib.h>

#include "config.h"

/*< private >
 * pidgin_mood_update_status:
 * @account: The #PurpleAccount instance.
 * @mood: The id of the new mood.
 * @text: The new status text.
 *
 * Updates the current status for @account with the given @mood and @text.
 */
static void
update_status_with_mood(PurpleAccount *account, const gchar *mood,
                        const gchar *text)
{
	if (mood && *mood) {
		if (text) {
			purple_account_set_status(account, "mood", TRUE,
			                          PURPLE_MOOD_NAME, mood,
				    				  PURPLE_MOOD_COMMENT, text,
			                          NULL);
		} else {
			purple_account_set_status(account, "mood", TRUE,
			                          PURPLE_MOOD_NAME, mood,
			                          NULL);
		}
	} else {
		purple_account_set_status(account, "mood", FALSE, NULL);
	}
}

/*< private
 * pidgin_mood_edit_cb:
 * @connection: The #PurpleConnection instance.
 * @fields: The #PurpleRequestFields
 *
 * This a callback function for when the request dialog has been accepted.
 */
static void
pidgin_mood_dialog_edit_cb(PurpleConnection *connection,
                           PurpleRequestFields *fields)
{
	PurpleRequestField *mood_field = NULL;
	GList *l = NULL;
	const gchar *mood = NULL;

	mood_field = purple_request_fields_get_field(fields, "mood");
	l = purple_request_field_list_get_selected(mood_field);

	if(l == NULL) {
		return;
	}

	mood = purple_request_field_list_get_data(mood_field, l->data);

	if(connection != NULL) {
		PurpleAccount *account = purple_connection_get_account(connection);
		PurpleConnectionFlags flags;
		const gchar *text = NULL;

		flags = purple_connection_get_flags(connection);
		if (flags & PURPLE_CONNECTION_FLAG_SUPPORT_MOOD_MESSAGES) {
			PurpleRequestField *text_field = NULL;

			text_field = purple_request_fields_get_field(fields, "text");
			text = purple_request_field_string_get_value(text_field);
		} else {
			text = NULL;
		}

		update_status_with_mood(account, mood, text);
	} else {
		GListModel *manager_model = NULL;
		guint n_items = 0;

		manager_model = purple_account_manager_get_default_as_model();
		n_items = g_list_model_get_n_items(manager_model);
		for(guint index = 0; index < n_items; index++) {
			PurpleAccount *account = NULL;

			account = g_list_model_get_item(manager_model, index);
			connection = purple_account_get_connection(account);
			if(PURPLE_IS_CONNECTION(connection)) {
				PurpleConnectionFlags flags;

				flags = purple_connection_get_flags(connection);
				if(flags & PURPLE_CONNECTION_FLAG_SUPPORT_MOODS) {
					update_status_with_mood(account, mood, NULL);
				}
			}

			g_object_unref(account);
		}
	}
}

/*< private >
 * pidgin_mood_get_global_moods:
 *
 * Returns an array of all global moods.
 *
 * This function should be in libpurple, and it needs a lot of cleanup.  It
 * should probably also be returning a GList of moods as that's easier to deal
 * with.
 *
 * Also, there is non-deterministic behavior here that the order of the
 * returned moods depends purely on the order that the accounts where connected
 * in.  This is probably okay, but we should look at fixing that somehow.
 *
 * Returns: (transfer full): A list of all global moods.
 */
static PurpleMood *
pidgin_mood_get_global_moods(void) {
	PurpleAccountManager *manager = NULL;
	GHashTable *global_moods = NULL;
	GHashTable *mood_counts = NULL;
	GList *accounts = NULL;
	PurpleMood *result = NULL;
	GList *out_moods = NULL;
	int i = 0;
	int num_accounts = 0;

	global_moods = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);
	mood_counts = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);

	manager = purple_account_manager_get_default();
	accounts = purple_account_manager_get_enabled(manager);
	for (; accounts ; accounts = g_list_delete_link(accounts, accounts)) {
		PurpleAccount *account = (PurpleAccount *) accounts->data;
		if (purple_account_is_connected(account)) {
			PurpleConnection *gc = purple_account_get_connection(account);

			if (purple_connection_get_flags(gc) & PURPLE_CONNECTION_FLAG_SUPPORT_MOODS) {
				PurpleProtocol *protocol = purple_connection_get_protocol(gc);
				PurpleMood *mood = NULL;

				for (mood = purple_protocol_client_get_moods(PURPLE_PROTOCOL_CLIENT(protocol), account) ;
				    mood->mood != NULL ; mood++) {
					int mood_count =
							GPOINTER_TO_INT(g_hash_table_lookup(mood_counts, mood->mood));

					if (!g_hash_table_contains(global_moods, mood->mood)) {
						g_hash_table_insert(global_moods, (gpointer)mood->mood, mood);
					}
					g_hash_table_insert(mood_counts, (gpointer)mood->mood,
					    GINT_TO_POINTER(mood_count + 1));
				}

				num_accounts++;
			}
		}
	}

	result = g_new0(PurpleMood, g_hash_table_size(global_moods) + 1);

	out_moods = g_hash_table_get_values(global_moods);
	while (out_moods) {
		PurpleMood *mood = (PurpleMood *) out_moods->data;
		int in_num_accounts =
			GPOINTER_TO_INT(g_hash_table_lookup(mood_counts, mood->mood));

		if (in_num_accounts == num_accounts) {
			/* mood is present in all accounts supporting moods */
			result[i].mood = mood->mood;
			result[i].description = mood->description;
			i++;
		}
		out_moods = g_list_delete_link(out_moods, out_moods);
	}

	g_hash_table_destroy(global_moods);
	g_hash_table_destroy(mood_counts);

	return result;
}

/*< private >
 * pidgin_mood_get_global_status:
 *
 * Get the currently selected mood name for all mood support accounts.  If no
 * mood is set, or accounts have different moods then %NULL is returned.
 *
 * Returns: The currently selected mood name or %NULL if a mood is not set, or
 *          accounts are using different moods.
 */
static const gchar *
pidgin_mood_get_global_status(void) {
	PurpleAccountManager *manager = NULL;
	GList *accounts = NULL;
	const gchar *found_mood = NULL;

	manager = purple_account_manager_get_default();
	accounts = purple_account_manager_get_enabled(manager);
	for (; accounts ; accounts = g_list_delete_link(accounts, accounts)) {
		PurpleAccount *account = (PurpleAccount *) accounts->data;

		if (purple_account_is_connected(account) &&
		    (purple_connection_get_flags(purple_account_get_connection(account)) &
		     PURPLE_CONNECTION_FLAG_SUPPORT_MOODS)) {
			PurplePresence *presence = purple_account_get_presence(account);
			PurpleStatus *status = purple_presence_get_status(presence, "mood");
			const gchar *curr_mood = purple_status_get_attr_string(status, PURPLE_MOOD_NAME);

			if (found_mood != NULL && !purple_strequal(curr_mood, found_mood)) {
				/* found a different mood */
				found_mood = NULL;
				break;
			} else {
				found_mood = curr_mood;
			}
		}
	}

	return found_mood;
}

/******************************************************************************
 * Public API
 *****************************************************************************/
void
pidgin_mood_dialog_show(PurpleAccount *account) {
	const gchar *current_mood;
	PurpleRequestFields *fields;
	PurpleRequestFieldGroup *g;
	PurpleRequestField *f;
	PurpleConnection *gc = NULL;
	PurpleProtocol *protocol = NULL;
	PurpleMood *mood = NULL;
	PurpleMood *global_moods = NULL;

	if (account) {
		PurplePresence *presence = purple_account_get_presence(account);
		PurpleStatus *status = purple_presence_get_status(presence, "mood");
		gc = purple_account_get_connection(account);
		g_return_if_fail(purple_connection_get_protocol(gc) != NULL);
		protocol = purple_connection_get_protocol(gc);
		current_mood = purple_status_get_attr_string(status, PURPLE_MOOD_NAME);
	} else {
		current_mood = pidgin_mood_get_global_status();
	}

	fields = purple_request_fields_new();
	g = purple_request_field_group_new(NULL);
	f = purple_request_field_list_new("mood", _("Please select your mood from the list"));

	purple_request_field_list_add_icon(f, _("None"), NULL, "");
	if (current_mood == NULL)
		purple_request_field_list_add_selected(f, _("None"));

	/* TODO: rlaager wants this sorted. */
	/* TODO: darkrain wants it sorted post-translation */
	if (account && PURPLE_IS_PROTOCOL_CLIENT(protocol)) {
		mood = purple_protocol_client_get_moods(PURPLE_PROTOCOL_CLIENT(protocol), account);
	}

	if(mood == NULL) {
		mood = global_moods = pidgin_mood_get_global_moods();
	}

	for ( ; mood != NULL && mood->mood != NULL ; mood++) {
		char *path;

		if (mood->description == NULL) {
			continue;
		}

		path = pidgin_mood_get_icon_path(mood->mood);
		purple_request_field_list_add_icon(f, _(mood->description),
				path, (gpointer)mood->mood);
		g_free(path);

		if (current_mood && purple_strequal(current_mood, mood->mood))
			purple_request_field_list_add_selected(f, _(mood->description));
	}
	purple_request_field_group_add_field(g, f);

	purple_request_fields_add_group(fields, g);

	/* if the connection allows setting a mood message */
	if (gc && (purple_connection_get_flags(gc) & PURPLE_CONNECTION_FLAG_SUPPORT_MOOD_MESSAGES)) {
		g = purple_request_field_group_new(NULL);
		f = purple_request_field_string_new("text",
		    _("Message (optional)"), NULL, FALSE);
		purple_request_field_group_add_field(g, f);
		purple_request_fields_add_group(fields, g);
	}

	purple_request_fields(gc, _("Edit User Mood"), _("Edit User Mood"),
	                      NULL, fields,
	                      _("OK"), G_CALLBACK(pidgin_mood_dialog_edit_cb),
	                      _("Cancel"), NULL,
	                      purple_request_cpar_from_connection(gc), gc);

	g_free(global_moods);
}

gchar *
pidgin_mood_get_icon_path(const gchar *mood) {
	gchar *path;

	if(purple_strequal(mood, "busy")) {
		path = g_build_filename(PURPLE_DATADIR, "pidgin", "icons",
			"hicolor", "16x16", "status", "user-busy.png", NULL);
	} else if(purple_strequal(mood, "hiptop")) {
		path = g_build_filename(PURPLE_DATADIR, "pidgin", "icons",
			"hicolor", "16x16", "emblems", "emblem-hiptop.png",
			NULL);
	} else {
		gchar *filename = g_strdup_printf("%s.png", mood);
		path = g_build_filename(PURPLE_DATADIR, "pixmaps", "pidgin",
			"emotes", "small", filename, NULL);
		g_free(filename);
	}

	return path;
}

