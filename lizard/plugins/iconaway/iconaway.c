/* purple
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
#include <glib/gi18n-lib.h>

#include <purple.h>

#include <pidgin.h>

#define ICONAWAY_PLUGIN_ID "gtk-iconaway"

static void
iconify_windows(G_GNUC_UNUSED PurpleAccount *account,
                G_GNUC_UNUSED PurpleStatus *old,
                PurpleStatus *newstatus)
{
	GApplication *application = NULL;
	PurplePresence *presence;
	GList *windows;

	presence = purple_status_get_presence(newstatus);

	if(purple_presence_is_available(presence)) {
		return;
	}

	purple_blist_set_visible(FALSE);

	application = g_application_get_default();
	windows = gtk_application_get_windows(GTK_APPLICATION(application));
	g_list_foreach(windows, (GFunc)gtk_window_minimize, NULL);
}

/*
 *  EXPORTED FUNCTIONS
 */

static GPluginPluginInfo *
icon_away_query(G_GNUC_UNUSED GError **error)
{
	const gchar * const authors[] = {
		"Eric Warmenhoven <eric@warmenhoven.org>",
		NULL
	};

	return pidgin_plugin_info_new(
		"id",           ICONAWAY_PLUGIN_ID,
		"name",         N_("Minimize on Away"),
		"version",      DISPLAY_VERSION,
		"category",     N_("User interface"),
		"summary",      N_("Minimizes the buddy list and your conversations "
		                   "when you go away."),
		"description",  N_("Minimizes the buddy list and your conversations "
		                   "when you go away."),
		"authors",      authors,
		"website",      PURPLE_WEBSITE,
		"abi-version",  PURPLE_ABI_VERSION,
		NULL
	);
}

static gboolean
icon_away_load(GPluginPlugin *plugin, G_GNUC_UNUSED GError **error)
{
	purple_signal_connect(purple_accounts_get_handle(), "account-status-changed",
						plugin, G_CALLBACK(iconify_windows), NULL);

	return TRUE;
}

static gboolean
icon_away_unload(G_GNUC_UNUSED GPluginPlugin *plugin,
                 G_GNUC_UNUSED gboolean shutdown,
                 G_GNUC_UNUSED GError **error)
{
	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(icon_away)
