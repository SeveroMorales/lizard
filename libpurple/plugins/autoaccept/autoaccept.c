/*
 * Autoaccept - Auto-accept file transfers from selected users
 * Copyright (C) 2006
 *
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

#define PLUGIN_ID			"core-plugin_pack-autoaccept"
#define PLUGIN_NAME			N_("Autoaccept")
#define PLUGIN_CATEGORY		N_("Utility")
#define PLUGIN_STATIC_NAME	Autoaccept
#define PLUGIN_SUMMARY		N_("Auto-accept file transfer requests from selected users.")
#define PLUGIN_DESCRIPTION	N_("Auto-accept file transfer requests from selected users.")
#define PLUGIN_AUTHORS		{"Sadrul H Chowdhury <sadrul@users.sourceforge.net>", NULL}

/* System headers */
#include <glib.h>
#include <glib/gstdio.h>

#include <gplugin.h>
#include <gplugin-native.h>

#include <purple.h>

#define SETTINGS_SCHEMA_ID "im.pidgin.Purple.plugin.AutoAccept"
#define PREF_PATH "path"
#define PREF_STRANGER "stranger"
#define PREF_NOTIFY "notify"
#define PREF_NEWDIR "newdir"
#define PREF_ESCAPE "escape"

typedef enum
{
	FT_ASK,
	FT_ACCEPT,
	FT_REJECT
} AutoAcceptSetting;

static gboolean
ensure_path_exists(const char *dir)
{
	if (!g_file_test(dir, G_FILE_TEST_IS_DIR))
	{
		if (g_mkdir_with_parents(dir, S_IRUSR | S_IWUSR | S_IXUSR)) {
			return FALSE;
		}
	}

	return TRUE;
}

static void
auto_accept_complete_cb(PurpleXfer *xfer, G_GNUC_UNUSED GParamSpec *pspec,
                        G_GNUC_UNUSED gpointer data)
{
	GSettings *settings = NULL;
	PurpleConversationManager *manager = NULL;

	if (purple_xfer_get_status(xfer) != PURPLE_XFER_STATUS_DONE) {
		return;
	}

	settings = g_settings_new_with_backend(SETTINGS_SCHEMA_ID,
	                                       purple_core_get_settings_backend());
	manager = purple_conversation_manager_get_default();

	if(g_settings_get_boolean(settings, PREF_NOTIFY) &&
	   !purple_conversation_manager_find_im(manager,
	                                        purple_xfer_get_account(xfer),
	                                        purple_xfer_get_remote_user(xfer)))
	{
		char *message = g_strdup_printf(_("Autoaccepted file transfer of \"%s\" from \"%s\" completed."),
					purple_xfer_get_filename(xfer), purple_xfer_get_remote_user(xfer));
		purple_notify_info(NULL, _("Autoaccept complete"), message,
			NULL, purple_request_cpar_from_account(
				purple_xfer_get_account(xfer)));
		g_free(message);
	}

	g_object_unref(settings);
}

static void
file_recv_request_cb(PurpleXfer *xfer, G_GNUC_UNUSED gpointer data)
{
	GSettings *settings = NULL;
	PurpleAccount *account;
	PurpleBlistNode *node;
	gchar *pref;
	char *filename;
	char *dirname;

    int accept_setting;

	account = purple_xfer_get_account(xfer);
	node = PURPLE_BLIST_NODE(purple_blist_find_buddy(account, purple_xfer_get_remote_user(xfer)));

	settings = g_settings_new_with_backend(SETTINGS_SCHEMA_ID,
	                                       purple_core_get_settings_backend());

	/* If person is on buddy list, use the buddy setting; otherwise, use the
	   stranger setting. */
	if (node) {
		node = purple_blist_node_get_parent(node);
		g_return_if_fail(PURPLE_IS_META_CONTACT(node));
		accept_setting = purple_blist_node_get_int(node, "autoaccept");
	} else {
		accept_setting = g_settings_get_enum(settings, PREF_STRANGER);
	}

	switch (accept_setting) {
		case FT_ASK:
			break;
		case FT_ACCEPT:
			pref = g_settings_get_string(settings, PREF_PATH);
			if (ensure_path_exists(pref)) {
				int count = 1;
				const char *escape;
				gchar **name_and_ext;
				const gchar *name;
				gchar *ext;

				if (g_settings_get_boolean(settings, PREF_NEWDIR)) {
					const gchar *normalized = purple_normalize(account,
					                                           purple_xfer_get_remote_user(xfer));
					dirname = g_build_filename(pref, normalized, NULL);
				} else {
					dirname = g_build_filename(pref, NULL);
				}

				if (!ensure_path_exists(dirname)) {
					g_free(dirname);
					g_free(pref);
					break;
				}

				/* Escape filename (if escaping is turned on) */
				if (g_settings_get_boolean(settings, PREF_ESCAPE)) {
					escape = purple_escape_filename(purple_xfer_get_filename(xfer));
				} else {
					escape = purple_xfer_get_filename(xfer);
				}
				filename = g_build_filename(dirname, escape, NULL);

				/* Split at the first dot, to avoid uniquifying "foo.tar.gz" to "foo.tar-2.gz" */
				name_and_ext = g_strsplit(escape, ".", 2);
				name = name_and_ext[0];
				if (name == NULL) {
					g_strfreev(name_and_ext);
					g_free(pref);
					g_object_unref(settings);
					g_return_if_reached();
				}
				if (name_and_ext[1] != NULL) {
					/* g_strsplit does not include the separator in each chunk. */
					ext = g_strdup_printf(".%s", name_and_ext[1]);
				} else {
					ext = g_strdup("");
				}

				/* Make sure the file doesn't exist. Do we want some better checking than this? */
				/* FIXME: There is a race here: if the newly uniquified file name gets created between
				 *        this g_file_test and the transfer starting, the file created in the meantime
				 *        will be clobbered. But it's not at all straightforward to fix.
				 */
				while (g_file_test(filename, G_FILE_TEST_EXISTS)) {
					char *file = g_strdup_printf("%s-%d%s", name, count++, ext);
					g_free(filename);
					filename = g_build_filename(dirname, file, NULL);
					g_free(file);
				}

				purple_xfer_request_accepted(xfer, filename);

				g_strfreev(name_and_ext);
				g_free(ext);
				g_free(dirname);
				g_free(filename);
			}
			g_free(pref);

			g_signal_connect(xfer, "notify::status",
			                 G_CALLBACK(auto_accept_complete_cb), NULL);
			break;
		case FT_REJECT:
			purple_xfer_set_status(xfer, PURPLE_XFER_STATUS_CANCEL_LOCAL);
			break;
	}

	g_object_unref(settings);
}

static void
save_cb(PurpleBlistNode *node, int choice)
{
	if (PURPLE_IS_BUDDY(node))
		node = purple_blist_node_get_parent(node);
	g_return_if_fail(PURPLE_IS_META_CONTACT(node));
	purple_blist_node_set_int(node, "autoaccept", choice);
}

static void
set_auto_accept_settings(PurpleBlistNode *node, gpointer plugin)
{
	char *message;

	if (PURPLE_IS_BUDDY(node))
		node = purple_blist_node_get_parent(node);
	g_return_if_fail(PURPLE_IS_META_CONTACT(node));

	message = g_strdup_printf(_("When a file-transfer request arrives from %s"),
					purple_meta_contact_get_alias(PURPLE_META_CONTACT(node)));
	purple_request_choice(plugin, _("Set Autoaccept Setting"), message,
						NULL, GINT_TO_POINTER(purple_blist_node_get_int(node, "autoaccept")),
						_("_Save"), G_CALLBACK(save_cb),
						_("_Cancel"), NULL,
						NULL, node,
						_("Ask"), GINT_TO_POINTER(FT_ASK),
						_("Auto Accept"), GINT_TO_POINTER(FT_ACCEPT),
						_("Auto Reject"), GINT_TO_POINTER(FT_REJECT),
						NULL);
	g_free(message);
}

static void
context_menu(PurpleBlistNode *node, GList **menu, gpointer plugin)
{
	PurpleActionMenu *action;

	if (!PURPLE_IS_BUDDY(node) && !PURPLE_IS_META_CONTACT(node) &&
		!purple_blist_node_is_transient(node))
		return;

	action = purple_action_menu_new(_("Autoaccept File Transfers..."),
					G_CALLBACK(set_auto_accept_settings), plugin, NULL);
	(*menu) = g_list_prepend(*menu, action);
}

static GPluginPluginInfo *
auto_accept_query(G_GNUC_UNUSED GError **error)
{
	const gchar * const authors[] = PLUGIN_AUTHORS;

	return purple_plugin_info_new(
		"id",             PLUGIN_ID,
		"name",           PLUGIN_NAME,
		"version",        DISPLAY_VERSION,
		"category",       PLUGIN_CATEGORY,
		"summary",        PLUGIN_SUMMARY,
		"description",    PLUGIN_DESCRIPTION,
		"authors",        authors,
		"website",        PURPLE_WEBSITE,
		"abi-version",    PURPLE_ABI_VERSION,
		"settings-schema", SETTINGS_SCHEMA_ID,
		NULL
	);
}

static gboolean
auto_accept_load(GPluginPlugin *plugin, G_GNUC_UNUSED GError **error)
{
	GSettings *settings = NULL;
	gchar *dirname;

	settings = g_settings_new_with_backend(SETTINGS_SCHEMA_ID,
	                                       purple_core_get_settings_backend());

	/* Set a default download directory path. */
	dirname = g_settings_get_string(settings, PREF_PATH);
	if(*dirname == '\0') {
		const gchar *download_dir = NULL;

		download_dir = g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD);
		if(download_dir == NULL) {
			download_dir = g_get_home_dir();
		}

		g_free(dirname);
		dirname = g_build_filename(download_dir, "autoaccept", NULL);
		g_settings_set_string(settings, PREF_PATH, dirname);
	}

	g_free(dirname);
	g_object_unref(settings);

	purple_signal_connect(purple_xfers_get_handle(), "file-recv-request", plugin,
						G_CALLBACK(file_recv_request_cb), plugin);
	purple_signal_connect(purple_blist_get_handle(), "blist-node-extended-menu", plugin,
						G_CALLBACK(context_menu), plugin);
	return TRUE;
}

static gboolean
auto_accept_unload(G_GNUC_UNUSED GPluginPlugin *plugin,
                   G_GNUC_UNUSED gboolean shutdown,
                   G_GNUC_UNUSED GError **error)
{
	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(auto_accept)
