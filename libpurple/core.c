/* purple
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include <glib/gi18n-lib.h>
#define G_SETTINGS_ENABLE_BACKEND
#include <gio/gsettingsbackend.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */

#include "cmds.h"
#include "connection.h"
#include "conversations.h"
#include "core.h"
#include "debug.h"
#include "xfer.h"
#include "idle.h"
#include "image-store.h"
#include "network.h"
#include "notify.h"
#include "plugins.h"
#include "prefs.h"
#include "proxy.h"
#include "purpleconversation.h"
#include "purplecredentialmanager.h"
#include "purplehistorymanager.h"
#include "purplemessage.h"
#include "purplepath.h"
#include "purpleprivate.h"
#include "savedstatuses.h"
#include "signals.h"
#include "status.h"
#include "stun.h"
#include "util.h"
#ifdef _WIN32
#include "win32/win32dep.h"
#endif

struct PurpleCore
{
	PurpleUi *ui;

	void *reserved;
};

static PurpleCore      *_core = NULL;
static GSettingsBackend *settings_backend = NULL;

static void
purple_core_print_version(void)
{
	PurpleUi *ui = purple_core_get_ui();
	const gchar *ui_name = NULL;
	const gchar *ui_version = NULL;
	gchar *ui_full_name = NULL;

	if(PURPLE_IS_UI(ui)) {
		ui_name = purple_ui_get_name(ui);
		ui_version = purple_ui_get_version(ui);
	}

	if (ui_name) {
		if (ui_version) {
			ui_full_name = g_strdup_printf("%s %s", ui_name, ui_version);
		} else {
			ui_full_name = g_strdup(ui_name);
		}
	}

	purple_debug_info("main", "Launching %s%slibpurple %s",
		ui_full_name ? ui_full_name : "",
		ui_full_name ? " with " : "",
		purple_core_get_version());

	g_free(ui_full_name);
}

gboolean
purple_core_init(PurpleUi *ui, G_GNUC_UNUSED GError **error) {
	PurpleCore *core;
	const char *force_error_message = NULL;

	g_return_val_if_fail(PURPLE_IS_UI(ui), FALSE);
	g_return_val_if_fail(purple_get_core() == NULL, FALSE);

	bindtextdomain(PACKAGE, PURPLE_LOCALEDIR);

#ifdef _WIN32
	wpurple_init();
#endif

	_core = core = g_new0(PurpleCore, 1);
	core->ui = ui;
	core->reserved = NULL;

	/* This monster is to work around a bug that was fixed in glib 2.73.3. Once
	 * we require glib 2.74.0 this should be removed.
	 */
	if(TRUE) {
		char *path = g_build_filename(purple_config_dir(), "dummy.ini", NULL);
		g_object_unref(g_keyfile_settings_backend_new(path, "/dummy/", NULL));
		g_free(path);
	}

	/* The signals subsystem is important and should be first. */
	purple_signals_init();

	purple_util_init();

	purple_signal_register(core, "uri-handler",
		purple_marshal_BOOLEAN__POINTER_POINTER_POINTER,
		G_TYPE_BOOLEAN, 3,
		G_TYPE_STRING, /* Protocol */
		G_TYPE_STRING, /* Command */
		G_TYPE_POINTER); /* Parameters (GHashTable *) */

	purple_signal_register(core, "core-initialized", purple_marshal_VOID,
		G_TYPE_NONE, 0);

	purple_core_print_version();

	/* The prefs subsystem needs to be initialized before static protocols
	 * for protocol prefs to work. */
	purple_prefs_init();

	settings_backend = purple_ui_get_settings_backend(core->ui);

	purple_debug_init();

	purple_ui_prefs_init(core->ui);

	purple_notification_manager_startup();

	purple_cmds_init();
	purple_protocol_manager_startup();

	purple_credential_manager_startup(); /* before accounts */

	/* Since plugins get probed so early we should probably initialize their
	 * subsystem right away too.
	 */
	purple_plugins_init();

	/* The buddy icon code uses the image store, so init it early. */
	_purple_image_store_init();

	/* Accounts use status, buddy icons and connection signals, so
	 * initialize these before accounts
	 */
	purple_statuses_init();
	purple_buddy_icons_init();
	purple_connections_init();

	purple_account_manager_startup();
	purple_accounts_init();
	purple_contact_manager_startup();
	purple_savedstatuses_init();
	purple_notify_init();
	purple_conversations_init();
	purple_conversation_manager_startup();
	purple_whiteboard_manager_startup();
	purple_blist_init();
	purple_history_manager_startup();
	purple_network_init();
	purple_proxy_init();
	purple_stun_init();
	purple_xfers_init();
	purple_idle_init();

	/*
	 * Call this early on to try to auto-detect our IP address and
	 * hopefully save some time later.
	 */
	purple_network_discover_my_ip();

	/* Set this environment variable to anything to test the error reporting in
	 * the user interface.
	 */
	force_error_message = g_getenv("PURPLE3_CORE_ERROR_MESSAGE");
	if(force_error_message != NULL) {
		if(force_error_message[0] == '\0') {
			force_error_message = "This is a forced error for testing user "
			                      "interfaces.";
		}

		g_set_error_literal(error, g_quark_from_static_string("purple"), 0,
		                    force_error_message);

		return FALSE;
	}

	if(!purple_ui_start(core->ui, error)) {
		return FALSE;
	}

	/* Load the buddy list after UI init */
	purple_blist_boot();

	purple_signal_emit(purple_get_core(), "core-initialized");

	return TRUE;
}

void
purple_core_quit(void)
{
	PurpleCore *core = purple_get_core();
	PurpleCredentialManager *credential_manager = NULL;
	PurpleHistoryManager *history_manager = NULL;

	g_return_if_fail(core != NULL);

	/* Transmission ends */
	purple_connections_disconnect_all();

	/* Remove the active provider in the credential manager. */
	credential_manager = purple_credential_manager_get_default();
	purple_credential_manager_set_active(credential_manager, NULL, NULL);

	/* Remove the active history adapter */
	history_manager = purple_history_manager_get_default();
	purple_history_manager_set_active(history_manager, NULL, NULL);

	/* Save .xml files, remove signals, etc. */
	purple_idle_uninit();
	purple_whiteboard_manager_shutdown();
	purple_conversation_manager_shutdown();
	purple_conversations_uninit();
	purple_blist_uninit();
	purple_notify_uninit();
	purple_connections_uninit();
	purple_buddy_icons_uninit();
	purple_savedstatuses_uninit();
	purple_statuses_uninit();
	purple_accounts_uninit();
	purple_xfers_uninit();
	purple_proxy_uninit();
	_purple_image_store_uninit();
	purple_network_uninit();

	purple_ui_stop(core->ui);

	/* Everything after prefs_uninit must not try to read any prefs */
	g_clear_object(&settings_backend);
	purple_prefs_uninit();
	purple_plugins_uninit();

	/* after plugins */
	purple_contact_manager_shutdown();
	purple_account_manager_shutdown();
	purple_credential_manager_shutdown();
	purple_protocol_manager_shutdown();
	purple_cmds_uninit();

	purple_notification_manager_shutdown();
	purple_history_manager_shutdown();

	/* Everything after util_uninit cannot try to write things to the
	 * confdir.
	 */
	purple_util_uninit();

	purple_signals_uninit();

	g_clear_object(&core->ui);
	g_free(core);

#ifdef _WIN32
	wpurple_cleanup();
#endif

	_core = NULL;
}

const char *
purple_core_get_version(void)
{
	return VERSION;
}

PurpleCore *
purple_get_core(void)
{
	return _core;
}

gpointer
purple_core_get_settings_backend(void) {
	return settings_backend;
}

PurpleUi *
purple_core_get_ui(void) {
	return _core->ui;
}
