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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include <config.h>

#include <glib/gi18n-lib.h>

#include <gplugin.h>
#include <gplugin-native.h>

#include <purple.h>
#include "libpurple/glibcompat.h"

#include <canberra.h>

#define PURPLE_NOTIFICATION_SOUND_DOMAIN \
	g_quark_from_static_string("purple-notification-sound")

#define SETTINGS_SCHEMA_ID "im.pidgin.Purple.plugin.NotificationSound"
#define PREF_MUTED "muted"
#define PREF_MUTE_UNTIL "mute-until"

/******************************************************************************
 * Globals
 *****************************************************************************/
static gboolean muted = FALSE;
static GDateTime *mute_until = NULL;
static ca_context *context = NULL;

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_notification_sound_load_prefs(void) {
	GSettings *settings = NULL;
	gchar *str_mute_until = NULL;

	settings = g_settings_new_with_backend(SETTINGS_SCHEMA_ID,
	                                       purple_core_get_settings_backend());
	muted = g_settings_get_boolean(settings, PREF_MUTED);

	str_mute_until = g_settings_get_string(settings, PREF_MUTE_UNTIL);
	if(str_mute_until != NULL && *str_mute_until != '\0') {
		GTimeZone *tz = g_time_zone_new_utc();

		mute_until = g_date_time_new_from_iso8601(str_mute_until, tz);

		g_time_zone_unref(tz);
	}

	g_free(str_mute_until);
	g_object_unref(settings);
}

static void
purple_notification_sound_save_prefs(void) {
	GSettings *settings = NULL;

	settings = g_settings_new_with_backend(SETTINGS_SCHEMA_ID,
	                                       purple_core_get_settings_backend());

	g_settings_set_boolean(settings, PREF_MUTED, muted);
	if(mute_until != NULL) {
		gchar *str_mute_until = g_date_time_format_iso8601(mute_until);
		g_settings_set_string(settings, PREF_MUTE_UNTIL, str_mute_until);
		g_free(str_mute_until);
	} else {
		g_settings_set_string(settings, PREF_MUTE_UNTIL, "");
	}

	g_object_unref(settings);
}

static void
purple_notification_sound_play(const gchar *event_id) {
	/* If we have a mute_util time check to see if it has passed. */
	if(mute_until != NULL) {
		GDateTime *now = g_date_time_new_now_utc();

		if(g_date_time_compare(mute_until, now) <= 0) {
			muted = FALSE;
			g_clear_pointer(&mute_until, g_date_time_unref);

			purple_notification_sound_save_prefs();
		}

		g_date_time_unref(now);
	}

	/* If we're not muted at this point play the sound. */
	if(!muted) {
		gint r = 0;

		r = ca_context_play(context, 0,
		                    CA_PROP_EVENT_ID, event_id,
		                    CA_PROP_EVENT_DESCRIPTION, "New libpurple message",
		                    NULL);

		if(r != 0) {
			purple_debug_warning("notification-sound",
			                     "failed to play sound %s\n",
			                     ca_strerror(r));
		}
	}
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
purple_notification_sound_im_message_received(G_GNUC_UNUSED PurpleAccount *account,
                                              G_GNUC_UNUSED const gchar *sender,
                                              G_GNUC_UNUSED const gchar *message,
                                              G_GNUC_UNUSED PurpleConversation *conv,
                                              G_GNUC_UNUSED PurpleMessageFlags flags,
                                              G_GNUC_UNUSED gpointer data)
{
	purple_notification_sound_play("message-new-instant");
}

static void
purple_notification_sound_chat_message_received(G_GNUC_UNUSED PurpleAccount *account,
                                                G_GNUC_UNUSED gchar *sender,
                                                G_GNUC_UNUSED gchar *message,
                                                G_GNUC_UNUSED PurpleConversation *conv,
                                                G_GNUC_UNUSED PurpleMessageFlags flags,
                                                G_GNUC_UNUSED gpointer data)
{
	purple_notification_sound_play("message-new-instant");
}

/******************************************************************************
 * Actions
 *****************************************************************************/
static void
purple_notification_sound_mute(G_GNUC_UNUSED GSimpleAction *action,
                               GVariant *parameter,
                               G_GNUC_UNUSED gpointer data)
{
	if(!g_variant_is_of_type(parameter, G_VARIANT_TYPE_INT32)) {
		g_critical("notification-sound mute action parameter is of incorrect type %s",
		           g_variant_get_type_string(parameter));

		return;
	}

	if(!muted) {
		gint delay = g_variant_get_int32(parameter);

		if(delay == 0) {
			g_clear_pointer(&mute_until, g_date_time_unref);
		} else {
			GDateTime *now = NULL;

			now = g_date_time_new_now_utc();
			mute_until = g_date_time_add_minutes(now, delay);
			g_date_time_unref(now);
		}

		muted = TRUE;

		purple_notification_sound_save_prefs();
	}
}

static void
purple_notification_sound_unmute(G_GNUC_UNUSED GSimpleAction *action,
                                 G_GNUC_UNUSED GVariant *parameter,
                                 G_GNUC_UNUSED gpointer data)
{
	if(muted) {
		muted = FALSE;
		g_clear_pointer(&mute_until, g_date_time_unref);

		purple_notification_sound_save_prefs();
	}
}

/******************************************************************************
 * Plugin Exports
 *****************************************************************************/
static GPluginPluginInfo *
notification_sound_query(G_GNUC_UNUSED GError **error) {
	GMenu *menu = NULL, *section = NULL;
	GSimpleActionGroup *group = NULL;
	GActionEntry entries[] = {
		{
			.name = "mute",
			.activate = purple_notification_sound_mute,
			.parameter_type = "i",
		}, {
			.name = "unmute",
			.activate = purple_notification_sound_unmute,
		}
	};
	const gchar * const authors[] = {
		"Pidgin Developers <devel@pidgin.im>",
		NULL
	};

	menu = g_menu_new();

	section = g_menu_new();
	g_menu_append(section, _("Mute"), "mute(0)");
	g_menu_append(section, _("Unmute"), "unmute");

	g_menu_append_section(menu, NULL, G_MENU_MODEL(section));

	section = g_menu_new();
	g_menu_append(section, _("Mute for 30 minutes"), "mute(30)");
	g_menu_append(section, _("Mute for 1 hour"), "mute(60)");
	g_menu_append(section, _("Mute for 2 hours"), "mute(120)");
	g_menu_append(section, _("Mute for 4 hours"), "mute(240)");

	g_menu_append_section(menu, NULL, G_MENU_MODEL(section));

	/* Create our action group. */
	group = g_simple_action_group_new();
	g_action_map_add_action_entries(G_ACTION_MAP(group), entries,
	                                G_N_ELEMENTS(entries), NULL);

	return purple_plugin_info_new(
		"id", "purple-notification-sound",
		"abi-version", PURPLE_ABI_VERSION,
		"name", N_("Notification Sounds"),
		"version", VERSION,
		"summary", N_("Play sounds for notifications"),
		"authors", authors,
		"action-group", group,
		"action-menu", menu,
		"settings-schema", SETTINGS_SCHEMA_ID,
		NULL
	);
}

static gboolean
notification_sound_load(GPluginPlugin *plugin, GError **error) {
	gpointer conv_handle = NULL;
	gint error_code = 0;

	error_code = ca_context_create(&context);
	if(error_code != 0) {
		g_set_error(error, PURPLE_NOTIFICATION_SOUND_DOMAIN, error_code,
		            "failed to create canberra context: %s",
		            ca_strerror(error_code));

		return FALSE;
	}

	purple_notification_sound_load_prefs();

	conv_handle = purple_conversations_get_handle();

	purple_signal_connect(conv_handle,
	                      "received-im-msg",
	                      plugin,
	                      G_CALLBACK(purple_notification_sound_im_message_received),
	                      NULL
	);

	purple_signal_connect(conv_handle,
	                      "received-chat-msg",
	                      plugin,
	                      G_CALLBACK(purple_notification_sound_chat_message_received),
	                      NULL
	);

	return TRUE;
}

static gboolean
notification_sound_unload(GPluginPlugin *plugin,
                          G_GNUC_UNUSED gboolean shutdown,
                          G_GNUC_UNUSED GError **error)
{
	purple_signals_disconnect_by_handle(plugin);

	purple_notification_sound_save_prefs();

	g_clear_pointer(&context, ca_context_destroy);
	g_clear_pointer(&mute_until, g_date_time_unref);

	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(notification_sound)
