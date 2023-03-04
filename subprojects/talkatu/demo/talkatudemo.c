/*
 * Talkatu - GTK widgets for chat applications
 * Copyright (C) 2017-2020 Gary Kramlich <grim@reaperworld.com>
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
 * along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <locale.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>

#include <talkatu/talkatu.h>

#include "talkatudemowindow.h"

static GOptionEntry entries[] = {
	{
		"version", 'V', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE,
		NULL, N_("Display the version and exit"),
		NULL,
	}, {
		NULL, 0, 0, 0, NULL, NULL, NULL,
	}
};

static gint
talkatu_demo_handle_local_options(G_GNUC_UNUSED GApplication *application,
                                  GVariantDict *options,
                                  G_GNUC_UNUSED gpointer data)
{
	if (g_variant_dict_contains(options, "version")) {
		printf("talkatu-demo %s\n", TALKATU_VERSION);

		return 0;
	}

	return -1;
}

static void
talkatu_demo_startup(G_GNUC_UNUSED GApplication *app,
                     G_GNUC_UNUSED gpointer data)
{
	talkatu_init();
}

static void
talkatu_demo_shutdown(G_GNUC_UNUSED GApplication *app,
                      G_GNUC_UNUSED gpointer data)
{
	talkatu_uninit();
}

static void
talkatu_demo_activate(GApplication *app, G_GNUC_UNUSED gpointer data) {
	GtkWidget *win = NULL;

	win = talkatu_demo_window_new();
	gtk_application_add_window(GTK_APPLICATION(app), GTK_WINDOW(win));
	gtk_widget_show(win);
}

gint
main(gint argc, gchar **argv) {
	GtkApplication *app = NULL;
	gint status;

	setlocale(LC_ALL, "");
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	app = gtk_application_new("org.imfreedom.keep.talkatu.TalkatuDemo",
#if GLIB_CHECK_VERSION(2,74,0)
	                          G_APPLICATION_DEFAULT_FLAGS);
#else
	                          G_APPLICATION_FLAGS_NONE);
#endif
	g_application_add_main_option_entries(G_APPLICATION(app), entries);

	g_signal_connect(app, "handle-local-options",
	                 G_CALLBACK(talkatu_demo_handle_local_options), NULL);
	g_signal_connect(app, "activate", G_CALLBACK(talkatu_demo_activate), NULL);
	g_signal_connect(app, "startup", G_CALLBACK(talkatu_demo_startup), NULL);
	g_signal_connect(app, "shutdown", G_CALLBACK(talkatu_demo_shutdown), NULL);

	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	return status;
}
