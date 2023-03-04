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

#define PLUGIN_ID       "core-psychic"
#define PLUGIN_NAME     N_("Psychic Mode")
#define PLUGIN_CATEGORY N_("Utility")
#define PLUGIN_SUMMARY  N_("Psychic mode for incoming conversation")
#define PLUGIN_DESC     N_("Causes conversation windows to appear as other" \
			   " users begin to message you.  This works for XMPP")
#define PLUGIN_AUTHORS  { "Christopher O'Brien <siege@preoccupied.net>", NULL }


#define SETTINGS_SCHEMA_ID "im.pidgin.Purple.plugin.Psychic"
#define PREF_BUDDIES "buddies-only"
#define PREF_NOTICE "show-notice"
#define PREF_STATUS "activate-online"
#define PREF_RAISE "raise-conv"


static void
buddy_typing_cb(PurpleAccount *acct, const char *name,
                G_GNUC_UNUSED gpointer data)
{
	GSettings *settings = NULL;
	PurpleConversation *im;
	PurpleConversationManager *manager;

	settings = g_settings_new_with_backend(SETTINGS_SCHEMA_ID,
	                                       purple_core_get_settings_backend());

	if(g_settings_get_boolean(settings, PREF_STATUS) &&
	   !purple_status_is_available(purple_account_get_active_status(acct))) {
		purple_debug_info("psychic", "not available, doing nothing");
		g_object_unref(settings);
		return;
	}

	if(g_settings_get_boolean(settings, PREF_BUDDIES) &&
	   !purple_blist_find_buddy(acct, name)) {
		purple_debug_info("psychic", "not in blist, doing nothing");
		g_object_unref(settings);
		return;
	}

	manager = purple_conversation_manager_get_default();
	im = purple_conversation_manager_find_im(manager, acct, name);
	if(im == NULL) {
		purple_debug_info("psychic", "no previous conversation exists");
		im = purple_im_conversation_new(acct, name);

		if(g_settings_get_boolean(settings, PREF_RAISE)) {
			purple_conversation_present(im);
		}

		if(g_settings_get_boolean(settings, PREF_NOTICE)) {
			/* This is a quote from Star Wars.  You should probably not
			   translate it literally.  If you can't find a fitting cultural
			   reference in your language, consider translating something
			   like this instead: "You feel a new message coming." */
			purple_conversation_write_system_message(
			    im,
			    _("You feel a disturbance in the force..."),
			    PURPLE_MESSAGE_NO_LOG | PURPLE_MESSAGE_ACTIVE_ONLY);
		}

		/* Necessary because we may be creating a new conversation window. */
		purple_im_conversation_set_typing_state(PURPLE_IM_CONVERSATION(im),
		                                        PURPLE_IM_TYPING);
	}

	g_object_unref(settings);
}


static GPluginPluginInfo *
psychic_query(G_GNUC_UNUSED GError **error) {
  const gchar * const authors[] = PLUGIN_AUTHORS;

  return purple_plugin_info_new(
    "id",             PLUGIN_ID,
    "name",           PLUGIN_NAME,
    "version",        DISPLAY_VERSION,
    "category",       PLUGIN_CATEGORY,
    "summary",        PLUGIN_SUMMARY,
    "description",    PLUGIN_DESC,
    "authors",        authors,
    "website",        PURPLE_WEBSITE,
    "abi-version",    PURPLE_ABI_VERSION,
    "settings-schema", SETTINGS_SCHEMA_ID,
    NULL
  );
}


static gboolean
psychic_load(GPluginPlugin *plugin, G_GNUC_UNUSED GError **error) {

  void *convs_handle;

  convs_handle = purple_conversations_get_handle();

  purple_signal_connect(convs_handle, "buddy-typing", plugin,
		      G_CALLBACK(buddy_typing_cb), NULL);

  return TRUE;
}


static gboolean
psychic_unload(G_GNUC_UNUSED GPluginPlugin *plugin,
               G_GNUC_UNUSED gboolean shutdown,
               G_GNUC_UNUSED GError **error)
{
  return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(psychic)
