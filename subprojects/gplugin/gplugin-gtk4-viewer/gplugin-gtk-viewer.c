/*
 * Copyright (C) 2021 Elliott Sales de Andrade <quantum.analyst@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>
#define G_SETTINGS_ENABLE_BACKEND
#include <gio/gsettingsbackend.h>
#include <gtk/gtk.h>
#include <gplugin.h>
#include <gplugin-gtk.h>

#include "gplugin-gtk-viewer-window.h"

/******************************************************************************
 * Globals
 *****************************************************************************/
static gboolean show_internal = FALSE;
static char *show_plugin = NULL;

/******************************************************************************
 * Helpers
 *****************************************************************************/

static gint
handle_local_options_cb(
	G_GNUC_UNUSED GApplication *app,
	GVariantDict *options,
	G_GNUC_UNUSED gpointer data)
{
	gboolean option = FALSE;

	if(g_variant_dict_lookup(options, "version", "b", &option, NULL)) {
		/* Handle --version and exit with success. */
		printf("gplugin-gtk-viewer %s\n", GPLUGIN_VERSION);

		return 0;
	}

	return -1;
}

static void
startup_cb(G_GNUC_UNUSED GApplication *app, G_GNUC_UNUSED gpointer data)
{
	GPluginManager *manager = NULL;

	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);
	manager = gplugin_manager_get_default();

	gplugin_manager_prepend_paths_from_environment(
		manager,
		"GPLUGIN_PLUGIN_PATH");

	gplugin_manager_refresh(manager);
}

static void
shutdown_cb(G_GNUC_UNUSED GApplication *app, G_GNUC_UNUSED gpointer data)
{
	gplugin_uninit();
}

static void
activate_cb(GApplication *app)
{
	GtkWindow *window = NULL;
	GPluginPlugin *plugin = NULL;

	g_return_if_fail(GTK_IS_APPLICATION(app));

	if(show_plugin != NULL) {
		GPluginManager *manager = NULL;

		manager = gplugin_manager_get_default();
		plugin = gplugin_manager_find_plugin(manager, show_plugin);
		if(!GPLUGIN_IS_PLUGIN(plugin)) {
			g_error("Failed to find plugin with ID %s", show_plugin);
		}
	}

	window = gtk_application_get_active_window(GTK_APPLICATION(app));
	if(window == NULL) {
		GPluginGtkView *view = NULL;
		GSettingsBackend *backend = NULL;

		/* clang-format off */
		window = g_object_new(
			GPLUGIN_GTK_VIEWER_TYPE_WINDOW,
			"application", app,
			NULL);
		/* clang-format on */

		view = gplugin_gtk_viewer_window_get_view(
			GPLUGIN_GTK_VIEWER_WINDOW(window));
		gplugin_gtk_view_set_manager(view, gplugin_manager_get_default());
		gplugin_gtk_view_set_show_internal(view, show_internal);

		backend = g_memory_settings_backend_new();
		gplugin_gtk_view_set_settings_backend(view, backend);
		g_object_unref(backend);

		if(plugin != NULL) {
			gplugin_gtk_view_show_plugin(view, plugin);
		}
	}

	gtk_window_present(window);
}

/******************************************************************************
 * Main Stuff
 *****************************************************************************/
/* clang-format off */
static GOptionEntry entries[] = {
	{
		"internal", 'i', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE,
		&show_internal, "Show internal plugins", NULL,
	}, {
		"show-plugin", 'p', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING,
		&show_plugin, "Show page for a plugin on start", NULL,
	}, {
		"version", 'V', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE,
		NULL, "Display the version and exit", NULL,
	}, {
		NULL, 0, 0, 0, NULL, NULL, NULL,
	},
};
/* clang-format on */

gint
main(gint argc, gchar **argv)
{
	GtkApplication *app = NULL;
	gint ret;

	/* Set up gettext translations. */
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	/* Prepare application and options. */
#if GLIB_CHECK_VERSION(2, 74, 0)
	app = gtk_application_new(NULL, G_APPLICATION_DEFAULT_FLAGS);
#else
	app = gtk_application_new(NULL, G_APPLICATION_FLAGS_NONE);
#endif
	g_application_add_main_option_entries(G_APPLICATION(app), entries);
	g_application_add_option_group(
		G_APPLICATION(app),
		gplugin_get_option_group());

	g_signal_connect(app, "activate", G_CALLBACK(activate_cb), NULL);
	g_signal_connect(
		app,
		"handle-local-options",
		G_CALLBACK(handle_local_options_cb),
		NULL);
	g_signal_connect(app, "startup", G_CALLBACK(startup_cb), NULL);
	g_signal_connect(app, "shutdown", G_CALLBACK(shutdown_cb), NULL);

	/* Run and get exit code. */
	ret = g_application_run(G_APPLICATION(app), argc, argv);

	g_object_unref(app);

	return ret;
}
