/*
 * BuddyNote - Store notes on particular buddies
 * Copyright (C) 2004 Stu Tomlinson <stu@nosnilmot.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1301, USA.
 */
#include <glib/gi18n-lib.h>

#include <gplugin.h>
#include <gplugin-native.h>

#include <purple.h>

static void
dont_do_it_cb(G_GNUC_UNUSED PurpleBlistNode *node,
              G_GNUC_UNUSED const char *note)
{
}

static void
do_it_cb(PurpleBlistNode *node, const char *note)
{
	purple_blist_node_set_string(node, "notes", note);
}

static void
buddynote_edit_cb(PurpleBlistNode *node, G_GNUC_UNUSED gpointer data)
{
	const char *note;

	note = purple_blist_node_get_string(node, "notes");

	purple_request_input(node, _("Notes"),
					   _("Enter your notes below..."),
					   NULL,
					   note, TRUE, FALSE, "html",
					   _("Save"), G_CALLBACK(do_it_cb),
					   _("Cancel"), G_CALLBACK(dont_do_it_cb),
					   NULL, node);
}

static void
buddynote_extended_menu_cb(PurpleBlistNode *node, GList **m)
{
	PurpleActionMenu *bna = NULL;

	if (purple_blist_node_is_transient(node))
		return;

	*m = g_list_append(*m, bna);
	bna = purple_action_menu_new(_("Edit Notes..."), G_CALLBACK(buddynote_edit_cb), NULL, NULL);
	*m = g_list_append(*m, bna);
}

static GPluginPluginInfo *
buddy_note_query(G_GNUC_UNUSED GError **error)
{
	const gchar * const authors[] = {
		"Stu Tomlinson <stu@nosnilmot.com>",
		NULL
	};

	return purple_plugin_info_new(
		"id",           "core-plugin_pack-buddynote",
		"name",         N_("Buddy Notes"),
		"version",      DISPLAY_VERSION,
		"category",     N_("Utility"),
		"summary",      N_("Store notes on particular buddies."),
		"description",  N_("Adds the option to store notes for buddies on your "
		                   "buddy list."),
		"authors",      authors,
		"website",      PURPLE_WEBSITE,
		"abi-version",  PURPLE_ABI_VERSION,
		NULL
	);
}

static gboolean
buddy_note_load(GPluginPlugin *plugin, G_GNUC_UNUSED GError **error)
{

	purple_signal_connect(purple_blist_get_handle(), "blist-node-extended-menu",
						plugin, G_CALLBACK(buddynote_extended_menu_cb), NULL);

	return TRUE;
}

static gboolean
buddy_note_unload(G_GNUC_UNUSED GPluginPlugin *plugin,
                  G_GNUC_UNUSED gboolean shutdown,
                  G_GNUC_UNUSED GError **error)
{
	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(buddy_note)
