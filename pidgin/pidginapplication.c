/*
 * Pidgin - Internet Messenger
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <glib/gstdio.h>

#include <gplugin.h>
#include <purple.h>

#include <adwaita.h>

#include "pidginapplication.h"

#include "gtkblist.h"
#include "gtkdialogs.h"
#include "gtkroomlist.h"
#include "gtksavedstatuses.h"
#include "gtkxfer.h"
#include "pidginabout.h"
#include "pidginaccounteditor.h"
#include "pidginaccountmanager.h"
#include "pidginaccountsdisabledmenu.h"
#include "pidginaccountsenabledmenu.h"
#include "pidgincore.h"
#include "pidgindebug.h"
#include "pidgindisplaywindow.h"
#include "pidginmooddialog.h"
#include "pidginpluginsdialog.h"
#include "pidginpluginsmenu.h"
#include "pidginprefs.h"
#include "pidginstatuseditor.h"
#include "pidginstatusmanager.h"
#include "pidginui.h"

struct _PidginApplication {
	GtkApplication parent;

	GHashTable *action_groups;
};

/******************************************************************************
 * Globals
 *****************************************************************************/
static gchar *opt_config_dir_arg = NULL;
static gboolean opt_debug = FALSE;
static gboolean opt_nologin = FALSE;

static GOptionEntry option_entries[] = {
	{
		"config", 'c', 0, G_OPTION_ARG_FILENAME, &opt_config_dir_arg,
		N_("use DIR for config files"), N_("DIR")
	}, {
		"debug", 'd', 0, G_OPTION_ARG_NONE, &opt_debug,
		N_("print debugging messages to stdout"), NULL
	}, {
		"nologin", 'n', 0, G_OPTION_ARG_NONE, &opt_nologin,
		N_("don't automatically login"), NULL
	},
	{
		"version", 'v', 0, G_OPTION_ARG_NONE, NULL,
		N_("display the current version and exit"), NULL
	}, {
		NULL
	}
};

G_DEFINE_TYPE(PidginApplication, pidgin_application, GTK_TYPE_APPLICATION)

/******************************************************************************
 * Helpers
 *****************************************************************************/

/*
 * pidgin_application_present_transient_window:
 * @application: The application instance.
 * @window: The [class@Gtk.Window] to present.
 *
 * Presents a window and makes sure its transient parent is set to the currently
 * active window of @application.
 *
 * Since: 3.0.0
 */
static void
pidgin_application_present_transient_window(PidginApplication *application,
                                            GtkWindow *window)
{
	GtkWindow *active_window = NULL;

	g_return_if_fail(PIDGIN_IS_APPLICATION(application));
	g_return_if_fail(GTK_IS_WINDOW(window));

	active_window = pidgin_application_get_active_window(application);

	gtk_window_set_transient_for(window, active_window);

	gtk_window_present_with_time(window, GDK_CURRENT_TIME);
}

static void
pidgin_application_plugin_state_changed(G_GNUC_UNUSED GPluginManager *manager,
                                        G_GNUC_UNUSED GPluginPlugin *plugin,
                                        G_GNUC_UNUSED gpointer data)
{
	purple_plugins_save_loaded(PIDGIN_PREFS_ROOT "/plugins/loaded");
}

static void
pidgin_application_init_plugins(void) {
	GPluginManager *manager = gplugin_manager_get_default();

	gplugin_manager_append_paths_from_environment(manager,
	                                              "PIDGIN_PLUGIN_PATH");

	if(g_getenv("PURPLE_PLUGINS_SKIP")) {
		g_message("PURPLE_PLUGINS_SKIP environment variable set, skipping "
		          "normal Pidgin plugin paths");
	} else {
		gchar *path = g_build_filename(purple_data_dir(), "plugins", NULL);

		if(!g_file_test(path, G_FILE_TEST_IS_DIR)) {
			g_mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR);
		}

		gplugin_manager_append_path(manager, path);
		g_free(path);

		gplugin_manager_append_path(manager, PIDGIN_LIBDIR);
	}

	g_signal_connect(manager, "loaded-plugin",
	                 G_CALLBACK(pidgin_application_plugin_state_changed), NULL);
	g_signal_connect(manager, "load-plugin-failed",
	                 G_CALLBACK(pidgin_application_plugin_state_changed), NULL);
	g_signal_connect(manager, "unloaded-plugin",
	                 G_CALLBACK(pidgin_application_plugin_state_changed), NULL);
	g_signal_connect(manager, "unload-plugin-failed",
	                 G_CALLBACK(pidgin_application_plugin_state_changed), NULL);

	purple_plugins_refresh();
}

static void
pidgin_application_populate_dynamic_menus(PidginApplication *application) {
	GMenu *target = NULL;
	GMenuModel *model = NULL;

	/* Link the AccountsDisabledMenu into its proper location. */
	model = pidgin_accounts_disabled_menu_new();
	target = gtk_application_get_menu_by_id(GTK_APPLICATION(application),
	                                        "disabled-accounts");
	g_menu_append_section(target, NULL, model);

	/* Link the AccountsEnabledMenu into its proper location. */
	model = pidgin_accounts_enabled_menu_new();
	target = gtk_application_get_menu_by_id(GTK_APPLICATION(application),
	                                        "enabled-accounts");
	g_menu_append_section(target, NULL, model);

	/* Link the PluginsMenu into its proper location. */
	model = pidgin_plugins_menu_new();
	target = gtk_application_get_menu_by_id(GTK_APPLICATION(application),
	                                        "plugins-menu");
	g_menu_append_section(target, NULL, model);
}

/******************************************************************************
 * Actions
 *****************************************************************************/
/*< private >
 * pidgin_application_online_actions:
 *
 * This list keeps track of which actions should only be enabled while online.
 */
static const gchar *pidgin_application_online_actions[] = {
	"add-buddy",
	"add-group",
	"get-user-info",
	"new-message",
	"set-mood",
};

/*< private >
 * pidgin_application_chat_actions:
 *
 * This list keeps track of which actions should only be enabled if a protocol
 * supporting groups chats is connected.
 */
static const gchar *pidgin_application_chat_actions[] = {
	"add-chat",
	"join-chat",
};

/*< private >
 * pidgin_application_room_list_actions:
 *
 * This list keeps track of which actions should only be enabled if an online
 * account supports room lists.
 */
static const gchar *pidgin_application_room_list_actions[] = {
	"room-list",
};

/*< private >
 * pidgin_action_group_actions_set_enable:
 * @group: The #PidginActionGroup instance.
 * @actions: The action names.
 * @n_actions: The number of @actions.
 * @enabled: Whether or not to enable the actions.
 *
 * Sets the enabled property of the named actions to @enabled.
 */
static void
pidgin_application_actions_set_enabled(PidginApplication *application,
                                       const gchar *const *actions,
                                       gint n_actions,
                                       gboolean enabled)
{
	gint i = 0;

	for(i = 0; i < n_actions; i++) {
		GAction *action = NULL;
		const gchar *name = actions[i];

		action = g_action_map_lookup_action(G_ACTION_MAP(application), name);

		if(action != NULL) {
			g_simple_action_set_enabled(G_SIMPLE_ACTION(action), enabled);
		} else {
			g_warning("Failed to find action named %s", name);
		}
	}
}

static void
pidgin_application_about(G_GNUC_UNUSED GSimpleAction *simple,
                         G_GNUC_UNUSED GVariant *parameter, gpointer data)
{
	PidginApplication *application = data;
	static GtkWidget *about = NULL;

	if(!GTK_IS_WIDGET(about)) {
		about = pidgin_about_dialog_new();
		g_object_add_weak_pointer(G_OBJECT(about), (gpointer)&about);
	}

	pidgin_application_present_transient_window(application, GTK_WINDOW(about));
}

static void
pidgin_application_accounts(G_GNUC_UNUSED GSimpleAction *simple,
                            G_GNUC_UNUSED GVariant *parameter, gpointer data)
{
	PidginApplication *application = data;
	static GtkWidget *manager = NULL;

	if(!GTK_IS_WIDGET(manager)) {
		manager = pidgin_account_manager_new();
		g_object_add_weak_pointer(G_OBJECT(manager), (gpointer)&manager);
	}

	pidgin_application_present_transient_window(application,
	                                            GTK_WINDOW(manager));
}

static void
pidgin_application_add_buddy(G_GNUC_UNUSED GSimpleAction *simple,
                             G_GNUC_UNUSED GVariant *parameter,
                             G_GNUC_UNUSED gpointer data)
{
	purple_blist_request_add_buddy(NULL, NULL, NULL, NULL);
}

static void
pidgin_application_add_chat(G_GNUC_UNUSED GSimpleAction *simple,
                            G_GNUC_UNUSED GVariant *parameter,
                            G_GNUC_UNUSED gpointer data)
{
	purple_blist_request_add_chat(NULL, NULL, NULL, NULL);
}

static void
pidgin_application_add_group(G_GNUC_UNUSED GSimpleAction *simple,
                             G_GNUC_UNUSED GVariant *parameter,
                             G_GNUC_UNUSED gpointer data)
{
	purple_blist_request_add_group();
}

static void
pidgin_application_connect_account(G_GNUC_UNUSED GSimpleAction *simple,
                                   GVariant *parameter,
                                   G_GNUC_UNUSED gpointer data)
{
	PurpleAccount *account = NULL;
	PurpleAccountManager *manager = NULL;
	const gchar *id = NULL;

	id = g_variant_get_string(parameter, NULL);

	manager = purple_account_manager_get_default();

	account = purple_account_manager_find_by_id(manager, id);
	if(PURPLE_IS_ACCOUNT(account)) {
		purple_account_connect(account);
	}
}

static void
pidgin_application_debug(G_GNUC_UNUSED GSimpleAction *simple,
                         G_GNUC_UNUSED GVariant *parameter,
                         G_GNUC_UNUSED gpointer data)
{
	gboolean old = purple_prefs_get_bool(PIDGIN_PREFS_ROOT "/debug/enabled");
	purple_prefs_set_bool(PIDGIN_PREFS_ROOT "/debug/enabled", !old);
}


static void
pidgin_application_disable_account(G_GNUC_UNUSED GSimpleAction *simple,
                                   GVariant *parameter,
                                   G_GNUC_UNUSED gpointer data)
{
	PurpleAccount *account = NULL;
	PurpleAccountManager *manager = NULL;
	const gchar *id = NULL;

	id = g_variant_get_string(parameter, NULL);

	manager = purple_account_manager_get_default();

	account = purple_account_manager_find_by_id(manager, id);
	if(PURPLE_IS_ACCOUNT(account)) {
		if(purple_account_get_enabled(account)) {
			purple_account_set_enabled(account, FALSE);
		}
	}
}

static void
pidgin_application_donate(G_GNUC_UNUSED GSimpleAction *simple,
                          G_GNUC_UNUSED GVariant *parameter,
                          G_GNUC_UNUSED gpointer data)
{
	purple_notify_uri(NULL, "https://www.imfreedom.org/donate/");
}

static void
pidgin_application_edit_account(G_GNUC_UNUSED GSimpleAction *simple,
                                GVariant *parameter, gpointer data)
{
	PidginApplication *application = data;
	PurpleAccount *account = NULL;
	PurpleAccountManager *manager = NULL;
	const gchar *id = NULL;

	id = g_variant_get_string(parameter, NULL);

	manager = purple_account_manager_get_default();

	account = purple_account_manager_find_by_id(manager, id);
	if(PURPLE_IS_ACCOUNT(account)) {
		GtkWidget *editor = pidgin_account_editor_new(account);

		pidgin_application_present_transient_window(application,
		                                            GTK_WINDOW(editor));
	}
}

static void
pidgin_application_enable_account(G_GNUC_UNUSED GSimpleAction *simple,
                                  GVariant *parameter,
                                  G_GNUC_UNUSED gpointer data)
{
	PurpleAccount *account = NULL;
	PurpleAccountManager *manager = NULL;
	const gchar *id = NULL;

	id = g_variant_get_string(parameter, NULL);

	manager = purple_account_manager_get_default();

	account = purple_account_manager_find_by_id(manager, id);
	if(PURPLE_IS_ACCOUNT(account)) {
		if(!purple_account_get_enabled(account)) {
			purple_account_set_enabled(account, TRUE);
		}
	}
}

static void
pidgin_application_file_transfers(G_GNUC_UNUSED GSimpleAction *simple,
                                  G_GNUC_UNUSED GVariant *parameter,
                                  G_GNUC_UNUSED gpointer data)
{
	pidgin_xfer_dialog_show(NULL);
}

static void
pidgin_application_get_user_info(G_GNUC_UNUSED GSimpleAction *simple,
                                 G_GNUC_UNUSED GVariant *parameter,
                                 G_GNUC_UNUSED gpointer data)
{
	pidgin_dialogs_info();
}

static void
pidgin_application_join_chat(G_GNUC_UNUSED GSimpleAction *simple,
                             G_GNUC_UNUSED GVariant *parameter,
                             G_GNUC_UNUSED gpointer data)
{
	pidgin_blist_joinchat_show();
}

static void
pidgin_application_new_message(G_GNUC_UNUSED GSimpleAction *simple,
                               G_GNUC_UNUSED GVariant *parameter,
                               G_GNUC_UNUSED gpointer data)
{
	pidgin_dialogs_im();
}

static void
pidgin_application_new_status(G_GNUC_UNUSED GSimpleAction *simple,
                              G_GNUC_UNUSED GVariant *parameter,
                              G_GNUC_UNUSED gpointer data)
{
	GtkWidget *editor = pidgin_status_editor_new(NULL);
	gtk_window_present_with_time(GTK_WINDOW(editor), GDK_CURRENT_TIME);
}

static void
pidgin_application_online_help(G_GNUC_UNUSED GSimpleAction *simple,
                               G_GNUC_UNUSED GVariant *parameter,
                               G_GNUC_UNUSED gpointer data)
{
	purple_notify_uri(NULL, PURPLE_WEBSITE "help");
}

static void
pidgin_application_plugins(G_GNUC_UNUSED GSimpleAction *simple,
                           G_GNUC_UNUSED GVariant *parameter, gpointer data)
{
	PidginApplication *application = data;
	static GtkWidget *dialog = NULL;

	if(!GTK_IS_WIDGET(dialog)) {
		dialog = pidgin_plugins_dialog_new();
		g_object_add_weak_pointer(G_OBJECT(dialog), (gpointer)&dialog);
	}

	pidgin_application_present_transient_window(application,
	                                            GTK_WINDOW(dialog));
}

static void
pidgin_application_preferences(G_GNUC_UNUSED GSimpleAction *simple,
                               G_GNUC_UNUSED GVariant *parameter,
                               gpointer data)
{
	PidginApplication *application = data;
	static GtkWidget *preferences = NULL;

	if(!GTK_IS_WIDGET(preferences)) {
		preferences = g_object_new(PIDGIN_TYPE_PREFS_WINDOW, NULL);
		g_object_add_weak_pointer(G_OBJECT(preferences), (gpointer)&preferences);
	}

	pidgin_application_present_transient_window(application,
	                                            GTK_WINDOW(preferences));

}

static void
pidgin_application_quit(G_GNUC_UNUSED GSimpleAction *simple,
                        G_GNUC_UNUSED GVariant *parameter,
                        G_GNUC_UNUSED gpointer data)
{
	GPluginManager *manager = NULL;

	/* Remove the signal handlers for plugin state changing so we don't try to
	 * update preferences.
	 */
	manager = gplugin_manager_get_default();
	g_signal_handlers_disconnect_by_func(manager,
	                                     pidgin_application_plugin_state_changed,
	                                     NULL);

	purple_core_quit();
}

static void
pidgin_application_room_list(G_GNUC_UNUSED GSimpleAction *simple,
                             G_GNUC_UNUSED GVariant *parameter,
                             G_GNUC_UNUSED gpointer data)
{
	pidgin_roomlist_dialog_show();
}

static void
pidgin_application_set_mood(G_GNUC_UNUSED GSimpleAction *simple,
                            G_GNUC_UNUSED GVariant *parameter,
                            G_GNUC_UNUSED gpointer data)
{
	pidgin_mood_dialog_show(NULL);
}

static void
pidgin_application_show_status_manager(G_GNUC_UNUSED GSimpleAction *simple,
                                       G_GNUC_UNUSED GVariant *parameter,
                                       gpointer data)
{
	PidginApplication *application = data;
	static GtkWidget *manager = NULL;

	if(!GTK_IS_WIDGET(manager)) {
		manager = pidgin_status_manager_new();
		g_object_add_weak_pointer(G_OBJECT(manager), (gpointer)&manager);
	}

	pidgin_application_present_transient_window(application,
	                                            GTK_WINDOW(manager));
}

static GActionEntry app_entries[] = {
	{
		.name = "about",
		.activate = pidgin_application_about,
	}, {
		.name = "add-buddy",
		.activate = pidgin_application_add_buddy,
	}, {
		.name = "add-chat",
		.activate = pidgin_application_add_chat,
	}, {
		.name = "add-group",
		.activate = pidgin_application_add_group,
	}, {
		.name = "connect-account",
		.activate = pidgin_application_connect_account,
		.parameter_type = "s",
	}, {
		.name = "debug",
		.activate = pidgin_application_debug,
	}, {
		.name = "disable-account",
		.activate = pidgin_application_disable_account,
		.parameter_type = "s",
	}, {
		.name = "donate",
		.activate = pidgin_application_donate,
	}, {
		.name = "edit-account",
		.activate = pidgin_application_edit_account,
		.parameter_type = "s",
	}, {
		.name = "enable-account",
		.activate = pidgin_application_enable_account,
		.parameter_type = "s",
	}, {
		.name = "file-transfers",
		.activate = pidgin_application_file_transfers,
	}, {
		.name = "get-user-info",
		.activate = pidgin_application_get_user_info,
	}, {
		.name = "join-chat",
		.activate = pidgin_application_join_chat,
	}, {
		.name = "manage-accounts",
		.activate = pidgin_application_accounts,
	}, {
		.name = "manage-plugins",
		.activate = pidgin_application_plugins,
	}, {
		.name = "new-message",
		.activate = pidgin_application_new_message,
	}, {
		.name = "new-status",
		.activate = pidgin_application_new_status,
	}, {
		.name = "online-help",
		.activate = pidgin_application_online_help,
	}, {
		.name = "preferences",
		.activate = pidgin_application_preferences,
	}, {
		.name = "quit",
		.activate = pidgin_application_quit,
	}, {
		.name = "room-list",
		.activate = pidgin_application_room_list,
	}, {
		.name = "set-mood",
		.activate = pidgin_application_set_mood,
	}, {
		.name = "status-manager",
		.activate = pidgin_application_show_status_manager,
	}
};

/******************************************************************************
 * Purple Signal Callbacks
 *****************************************************************************/
static void
pidgin_application_online_cb(gpointer data) {
	gint n_actions = G_N_ELEMENTS(pidgin_application_online_actions);

	pidgin_application_actions_set_enabled(PIDGIN_APPLICATION(data),
	                                       pidgin_application_online_actions,
	                                       n_actions,
	                                       TRUE);
}

static void
pidgin_application_offline_cb(gpointer data) {
	gint n_actions = G_N_ELEMENTS(pidgin_application_online_actions);

	pidgin_application_actions_set_enabled(PIDGIN_APPLICATION(data),
	                                       pidgin_application_online_actions,
	                                       n_actions,
	                                       FALSE);
}

static void
pidgin_application_signed_on_cb(PurpleAccount *account, gpointer data) {
	PidginApplication *application = PIDGIN_APPLICATION(data);
	PurpleProtocol *protocol = NULL;
	gboolean should_enable_chat = FALSE, should_enable_room_list = FALSE;
	gint n_actions = 0;

	protocol = purple_account_get_protocol(account);

	/* We assume that the current state is correct, so we don't bother changing
	 * state unless the newly connected account implements the chat interface,
	 * which would cause a state change.
	 */
	should_enable_chat = PURPLE_PROTOCOL_IMPLEMENTS(protocol, CHAT, info);
	if(should_enable_chat) {
		n_actions = G_N_ELEMENTS(pidgin_application_chat_actions);
		pidgin_application_actions_set_enabled(application,
		                                       pidgin_application_chat_actions,
		                                       n_actions,
		                                       TRUE);
	}

	/* likewise, for the room list, we only care about enabling in this
	 * handler.
	 */
	should_enable_room_list = PURPLE_PROTOCOL_IMPLEMENTS(protocol, ROOMLIST,
	                                                     get_list);
	if(should_enable_room_list) {
		n_actions = G_N_ELEMENTS(pidgin_application_room_list_actions);
		pidgin_application_actions_set_enabled(application,
		                                       pidgin_application_room_list_actions,
		                                       n_actions,
		                                       TRUE);
	}
}

static void
pidgin_application_signed_off_cb(G_GNUC_UNUSED PurpleAccount *account,
                                 gpointer data)
{
	PidginApplication *application = PIDGIN_APPLICATION(data);
	gboolean should_disable_chat = TRUE, should_disable_room_list = TRUE;
	GList *connections = NULL, *l = NULL;
	gint n_actions = 0;

	/* walk through all the connections, looking for online ones that implement
	 * the chat interface.  We don't bother checking the account that this
	 * signal was emitted for, because it's already offline and will be
	 * filtered out by the online check.
	 */
	connections = purple_connections_get_all();
	for(l = connections; l != NULL; l = l->next) {
		PurpleConnection *connection = PURPLE_CONNECTION(l->data);
		PurpleProtocol *protocol = NULL;

		/* if the connection isn't online, we don't care about it */
		if(!PURPLE_CONNECTION_IS_CONNECTED(connection)) {
			continue;
		}

		protocol = purple_connection_get_protocol(connection);

		/* check if the protocol implements the chat interface */
		if(PURPLE_PROTOCOL_IMPLEMENTS(protocol, CHAT, info)) {
			should_disable_chat = FALSE;
		}

		/* check if the protocol implements the room list interface */
		if(PURPLE_PROTOCOL_IMPLEMENTS(protocol, ROOMLIST, get_list)) {
			should_disable_room_list = FALSE;
		}

		/* if we can't disable both, we can bail out of the loop */
		if(!should_disable_chat && !should_disable_room_list) {
			break;
		}
	}

	if(should_disable_chat) {
		n_actions = G_N_ELEMENTS(pidgin_application_chat_actions);
		pidgin_application_actions_set_enabled(application,
		                                       pidgin_application_chat_actions,
		                                       n_actions,
		                                       FALSE);
	}

	if(should_disable_room_list) {
		n_actions = G_N_ELEMENTS(pidgin_application_room_list_actions);
		pidgin_application_actions_set_enabled(application,
		                                       pidgin_application_room_list_actions,
		                                       n_actions,
		                                       FALSE);
	}
}

static void
pidgin_application_error_reponse_cb(G_GNUC_UNUSED AdwMessageDialog *self,
                                    G_GNUC_UNUSED char *response,
                                    gpointer data)
{
	g_application_quit(data);
}

/******************************************************************************
 * GtkApplication Implementation
 *****************************************************************************/
static void
pidgin_application_window_added(GtkApplication *application,
                                GtkWindow *window)
{
	PidginApplication *pidgin_application = PIDGIN_APPLICATION(application);
	GHashTableIter iter;
	gpointer key, value;

	GTK_APPLICATION_CLASS(pidgin_application_parent_class)->window_added(application,
	                                                                     window);

	if(strstr(VERSION, "-devel")) {
		gtk_widget_add_css_class(GTK_WIDGET(window), "devel");
	}

	g_hash_table_iter_init(&iter, pidgin_application->action_groups);
	while(g_hash_table_iter_next(&iter, &key, &value)) {
		GActionGroup *action_group = value;
		gchar *prefix = key;

		gtk_widget_insert_action_group(GTK_WIDGET(window), prefix,
		                               action_group);
	}
}

/******************************************************************************
 * GApplication Implementation
 *****************************************************************************/
static void
pidgin_application_startup(GApplication *application) {
	PurpleAccountManager *manager = NULL;
	GError *error = NULL;
	GList *active_accounts = NULL;
	gpointer handle = NULL;

	G_APPLICATION_CLASS(pidgin_application_parent_class)->startup(application);

	adw_init();

	/* set a user-specified config directory */
	if (opt_config_dir_arg != NULL) {
		if (g_path_is_absolute(opt_config_dir_arg)) {
			purple_util_set_user_dir(opt_config_dir_arg);
		} else {
			/* Make an absolute (if not canonical) path */
			gchar *cwd = g_get_current_dir();
			gchar *path = g_build_filename(cwd, opt_config_dir_arg, NULL);

			purple_util_set_user_dir(path);

			g_free(cwd);
			g_free(path);
		}
	}

	pidgin_debug_init_handler();
#ifdef DEBUG
	pidgin_debug_set_print_enabled(TRUE);
#else
	pidgin_debug_set_print_enabled(opt_debug);
#endif

#ifdef _WIN32
	winpidgin_init();
#endif

	if(!purple_core_init(pidgin_ui_new(), &error)) {
		GtkWidget *message = NULL;
		const char *error_message = "unknown error";

		if(error != NULL) {
			error_message = error->message;
		}

		message = adw_message_dialog_new(NULL,
		                                 _("Pidgin 3 failed to initialize"),
		                                 error_message);
		g_clear_error(&error);

		adw_message_dialog_add_responses(ADW_MESSAGE_DIALOG(message),
		                                 "close", _("Close"), NULL);
		adw_message_dialog_set_close_response(ADW_MESSAGE_DIALOG(message),
		                                      "close");

		g_signal_connect(message, "response",
		                 G_CALLBACK(pidgin_application_error_reponse_cb),
		                 application);

		gtk_window_present_with_time(GTK_WINDOW(message), GDK_CURRENT_TIME);

		return;
	}

	pidgin_application_init_plugins();

	/* load plugins we had when we quit */
	purple_plugins_load_saved(PIDGIN_PREFS_ROOT "/plugins/loaded");

	gtk_window_set_default_icon_name("pidgin");

	g_free(opt_config_dir_arg);
	opt_config_dir_arg = NULL;

	/*
	 * We want to show the blist early in the init process so the
	 * user feels warm and fuzzy.
	 */
	purple_blist_show();

	if(purple_prefs_get_bool(PIDGIN_PREFS_ROOT "/debug/enabled")) {
		pidgin_debug_window_show();
	}

	if(opt_nologin) {
		/* Set all accounts to "offline" */
		PurpleSavedStatus *saved_status;

		/* If we've used this type+message before, lookup the transient status */
		saved_status = purple_savedstatus_find_transient_by_type_and_message(
							PURPLE_STATUS_OFFLINE, NULL);

		/* If this type+message is unique then create a new transient saved status */
		if(saved_status == NULL) {
			saved_status = purple_savedstatus_new(NULL, PURPLE_STATUS_OFFLINE);
		}

		/* Set the status for each account */
		purple_savedstatus_activate(saved_status);
	} else {
		/* Everything is good to go--sign on already */
		if (!purple_prefs_get_bool("/purple/savedstatus/startup_current_status")) {
			purple_savedstatus_activate(purple_savedstatus_get_startup());
		}

		purple_accounts_restore_current_statuses();
	}

	manager = purple_account_manager_get_default();
	active_accounts = purple_account_manager_get_enabled(manager);
	if(active_accounts == NULL) {
		g_action_group_activate_action(G_ACTION_GROUP(application),
		                               "manage-accounts", NULL);
	} else {
		g_list_free(active_accounts);
	}

	/* Populate our dynamic menus. */
	pidgin_application_populate_dynamic_menus(PIDGIN_APPLICATION(application));

	/* TODO: Use GtkApplicationWindow or add a window instead */
	g_application_hold(application);

	/* connect to the online and offline signals in purple connections.  This
	 * is used to toggle states of actions that require being online.
	 */
	handle = purple_connections_get_handle();
	purple_signal_connect(handle, "online", application,
	                      G_CALLBACK(pidgin_application_online_cb),
	                      application);
	purple_signal_connect(handle, "offline", application,
	                      G_CALLBACK(pidgin_application_offline_cb),
	                      application);

	/* connect to account-signed-on and account-signed-off to toggle actions
	 * that depend on specific interfaces in accounts.
	 */
	handle = purple_accounts_get_handle();
	purple_signal_connect(handle, "account-signed-on", application,
	                      G_CALLBACK(pidgin_application_signed_on_cb),
	                      application);
	purple_signal_connect(handle, "account-signed-off", application,
	                      G_CALLBACK(pidgin_application_signed_off_cb),
	                      application);

}

static void
pidgin_application_activate(G_GNUC_UNUSED GApplication *application) {
	GtkWidget *convwin = pidgin_display_window_get_default();

	if(GTK_IS_WINDOW(convwin)) {
		gtk_window_present(GTK_WINDOW(convwin));
	}
}

static gint
pidgin_application_command_line(GApplication *application,
                                GApplicationCommandLine *cmdline)
{
	gchar **argv = NULL;
	gint argc = 0, i = 0;

	argv = g_application_command_line_get_arguments(cmdline, &argc);

	if(argc == 1) {
		/* No arguments, just activate */
		g_application_activate(application);
	}

	/* Start at 1 to skip the executable name */
	for (i = 1; i < argc; i++) {
		purple_got_protocol_handler_uri(argv[i]);
	}

	g_strfreev(argv);

	return 0;
}

static gint
pidgin_application_handle_local_options(G_GNUC_UNUSED GApplication *application,
                                        GVariantDict *options)
{
	if (g_variant_dict_contains(options, "version")) {
		printf("%s %s (libpurple %s)\n", PIDGIN_NAME, DISPLAY_VERSION,
		       purple_core_get_version());

		return 0;
	}

	return -1;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_application_dispose(GObject *obj) {
	PidginApplication *application = PIDGIN_APPLICATION(obj);

	g_clear_pointer(&application->action_groups, g_hash_table_destroy);

	G_OBJECT_CLASS(pidgin_application_parent_class)->dispose(obj);
}

static void
pidgin_application_init(PidginApplication *application) {
	GApplication *gapp = G_APPLICATION(application);
	gboolean online = FALSE;
	gint n_actions = 0;

	application->action_groups = g_hash_table_new_full(g_str_hash, g_str_equal,
	                                                   g_free, g_object_unref);

	g_application_add_main_option_entries(gapp, option_entries);
	g_application_add_option_group(gapp, purple_get_option_group());
	g_application_add_option_group(gapp, gplugin_get_option_group());

	g_action_map_add_action_entries(G_ACTION_MAP(application), app_entries,
	                                G_N_ELEMENTS(app_entries), application);

	/* Set the default state for our actions to match our online state. */
	online = purple_connections_is_online();

	n_actions = G_N_ELEMENTS(pidgin_application_online_actions);
	pidgin_application_actions_set_enabled(application,
	                                       pidgin_application_online_actions,
	                                       n_actions,
	                                       online);

	n_actions = G_N_ELEMENTS(pidgin_application_chat_actions);
	pidgin_application_actions_set_enabled(application,
	                                       pidgin_application_chat_actions,
	                                       n_actions,
	                                       online);

	n_actions = G_N_ELEMENTS(pidgin_application_room_list_actions);
	pidgin_application_actions_set_enabled(application,
	                                       pidgin_application_room_list_actions,
	                                       n_actions,
	                                       online);
}

static void
pidgin_application_class_init(PidginApplicationClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GApplicationClass *app_class = G_APPLICATION_CLASS(klass);
	GtkApplicationClass *gtk_app_class = GTK_APPLICATION_CLASS(klass);

	obj_class->dispose = pidgin_application_dispose;

	app_class->startup = pidgin_application_startup;
	app_class->activate = pidgin_application_activate;
	app_class->command_line = pidgin_application_command_line;
	app_class->handle_local_options = pidgin_application_handle_local_options;

	gtk_app_class->window_added = pidgin_application_window_added;
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GApplication *
pidgin_application_new(void) {
	return g_object_new(
		PIDGIN_TYPE_APPLICATION,
		"application-id", "im.pidgin.Pidgin3",
		"flags", G_APPLICATION_CAN_OVERRIDE_APP_ID |
		         G_APPLICATION_HANDLES_COMMAND_LINE,
		"register-session", TRUE,
		NULL);
}

void
pidgin_application_add_action_group(PidginApplication *application,
                                    const gchar *prefix,
                                    GActionGroup *action_group)
{
	GList *windows = NULL;

	g_return_if_fail(prefix != NULL);

	if(G_IS_ACTION_GROUP(action_group)) {
		/* If action_group is valid, we need to create new references to the
		 * prefix and action_group when inserting them into the hash table.
		 */
		g_hash_table_insert(application->action_groups, g_strdup(prefix),
		                    g_object_ref(action_group));
	} else {
		g_hash_table_remove(application->action_groups, prefix);
	}

	/* Now walk through the windows and add/remove the action group. */
	windows = gtk_application_get_windows(GTK_APPLICATION(application));
	while(windows != NULL) {
		GtkWidget *window = GTK_WIDGET(windows->data);

		gtk_widget_insert_action_group(window, prefix, action_group);

		windows = windows->next;
	}
}

GtkWindow *
pidgin_application_get_active_window(PidginApplication *application) {
	GtkApplication *gtk_application = NULL;
	GtkWindow *window = NULL;

	g_return_val_if_fail(PIDGIN_IS_APPLICATION(application), NULL);

	gtk_application = GTK_APPLICATION(application);

	window = gtk_application_get_active_window(gtk_application);
	if(!GTK_IS_WINDOW(window)) {
		GList *windows = NULL;

		windows = gtk_application_get_windows(gtk_application);
		if(windows != NULL) {
			window = windows->data;
		}
	}

	return window;
}
