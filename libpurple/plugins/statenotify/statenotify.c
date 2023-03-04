/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02111-1301, USA.
 */

#include <glib/gi18n-lib.h>

#include <gplugin.h>
#include <gplugin-native.h>

#include <purple.h>

#define STATENOTIFY_PLUGIN_ID "core-statenotify"
#define SETTINGS_SCHEMA_ID "im.pidgin.Purple.plugin.StateNotify"

static void
write_status(PurpleBuddy *buddy, const char *message)
{
	PurpleAccount *account = NULL;
	PurpleConversation *im;
	PurpleConversationManager *manager;
	const char *who;
	char buf[256];
	char *escaped;
	const gchar *buddy_name = NULL;

	account = purple_buddy_get_account(buddy);
	buddy_name = purple_buddy_get_name(buddy);

	manager = purple_conversation_manager_get_default();
	im = purple_conversation_manager_find_im(manager, account, buddy_name);

	if (im == NULL)
		return;

	/* Prevent duplicate notifications for buddies in multiple groups */
	if (buddy != purple_blist_find_buddy(account, buddy_name))
		return;

	who = purple_buddy_get_alias(buddy);
	escaped = g_markup_escape_text(who, -1);

	g_snprintf(buf, sizeof(buf), message, escaped);
	g_free(escaped);

	purple_conversation_write_system_message(im, buf,
		PURPLE_MESSAGE_ACTIVE_ONLY | PURPLE_MESSAGE_NO_LINKIFY);
}

static void
buddy_status_changed_cb(PurpleBuddy *buddy, PurpleStatus *old_status,
                        PurpleStatus *status, G_GNUC_UNUSED gpointer data)
{
	GSettings *settings = NULL;
	gboolean available, old_available;

	if (!purple_status_is_exclusive(status) ||
			!purple_status_is_exclusive(old_status))
	{
		return;
	}

	available = purple_status_is_available(status);
	old_available = purple_status_is_available(old_status);

	settings = g_settings_new_with_backend(SETTINGS_SCHEMA_ID,
	                                       purple_core_get_settings_backend());
	if(g_settings_get_boolean(settings, "notify-away")) {
		if (available && !old_available) {
			write_status(buddy, _("%s is no longer away."));
		} else if (!available && old_available) {
			write_status(buddy, _("%s has gone away."));
		}
	}
	g_object_unref(settings);
}

static void
buddy_idle_changed_cb(PurpleBuddy *buddy, gboolean old_idle, gboolean idle,
                      G_GNUC_UNUSED gpointer data)
{
	GSettings *settings = NULL;

	settings = g_settings_new_with_backend(SETTINGS_SCHEMA_ID,
	                                       purple_core_get_settings_backend());
	if(g_settings_get_boolean(settings, "notify-idle")) {
		if (idle && !old_idle) {
			write_status(buddy, _("%s has become idle."));
		} else if (!idle && old_idle) {
			write_status(buddy, _("%s is no longer idle."));
		}
	}
	g_object_unref(settings);
}

static void
buddy_signon_cb(PurpleBuddy *buddy, G_GNUC_UNUSED gpointer data)
{
	GSettings *settings = NULL;

	settings = g_settings_new_with_backend(SETTINGS_SCHEMA_ID,
	                                       purple_core_get_settings_backend());
	if(g_settings_get_boolean(settings, "notify-signon")) {
		write_status(buddy, _("%s has signed on."));
	}
	g_object_unref(settings);
}

static void
buddy_signoff_cb(PurpleBuddy *buddy, G_GNUC_UNUSED gpointer data)
{
	GSettings *settings = NULL;

	settings = g_settings_new_with_backend(SETTINGS_SCHEMA_ID,
	                                       purple_core_get_settings_backend());
	if(g_settings_get_boolean(settings, "notify-signon")) {
		write_status(buddy, _("%s has signed off."));
	}
	g_object_unref(settings);
}

static GPluginPluginInfo *
state_notify_query(G_GNUC_UNUSED GError **error)
{
	const gchar * const authors[] = {
		"Christian Hammond <chipx86@gnupdate.org>",
		NULL
	};

	return purple_plugin_info_new(
		"id",             STATENOTIFY_PLUGIN_ID,
		"name",           N_("Buddy State Notification"),
		"version",        DISPLAY_VERSION,
		"category",       N_("Notification"),
		"summary",        N_("Notifies in a conversation window when a "
		                     "buddy goes or returns from away or idle."),
		"description",    N_("Notifies in a conversation window when a "
		                     "buddy goes or returns from away or idle."),
		"authors",        authors,
		"website",        PURPLE_WEBSITE,
		"abi-version",    PURPLE_ABI_VERSION,
		"settings-schema", SETTINGS_SCHEMA_ID,
		NULL
	);
}

static gboolean
state_notify_load(GPluginPlugin *plugin, G_GNUC_UNUSED GError **error)
{
	void *blist_handle = purple_blist_get_handle();

	purple_signal_connect(blist_handle, "buddy-status-changed", plugin,
	                    G_CALLBACK(buddy_status_changed_cb), NULL);
	purple_signal_connect(blist_handle, "buddy-idle-changed", plugin,
	                    G_CALLBACK(buddy_idle_changed_cb), NULL);
	purple_signal_connect(blist_handle, "buddy-signed-on", plugin,
	                    G_CALLBACK(buddy_signon_cb), NULL);
	purple_signal_connect(blist_handle, "buddy-signed-off", plugin,
	                    G_CALLBACK(buddy_signoff_cb), NULL);

	return TRUE;
}

static gboolean
state_notify_unload(G_GNUC_UNUSED GPluginPlugin *plugin,
                    G_GNUC_UNUSED gboolean shutdown,
                    G_GNUC_UNUSED GError **error)
{
	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(state_notify)
