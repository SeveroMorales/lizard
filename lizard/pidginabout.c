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

#ifdef HAVE_MESON_CONFIG
#include "meson-config.h"
#endif

#include <glib/gi18n-lib.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <json-glib/json-glib.h>

#include <adwaita.h>
#include <talkatu.h>

#include "pidginabout.h"

#include "package_revision.h"
#include "gtkutils.h"
#include "pidgincore.h"
#include "pidginresources.h"

struct _PidginAboutDialog {
	GtkDialog parent;

	GtkWidget *application_name;

	GtkTextBuffer *main_buffer;

	AdwPreferencesPage *developers_page;

	AdwPreferencesPage *translators_page;

	AdwPreferencesGroup *build_info_group;
	AdwPreferencesGroup *runtime_info_group;
	AdwPreferencesGroup *gtk_settings_group;
	AdwPreferencesGroup *plugin_search_paths_group;
	AdwPreferencesGroup *conf_path_info_group;
	AdwPreferencesGroup *build_args_group;
};

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_about_dialog_load_application_name(PidginAboutDialog *about) {
	gchar *label = g_strdup_printf("%s %s", PIDGIN_NAME, VERSION);

	gtk_label_set_text(GTK_LABEL(about->application_name), label);

	g_free(label);
}

static void
pidgin_about_dialog_load_main_page(PidginAboutDialog *about) {
	GtkTextIter start;
	GInputStream *istream = NULL;
	GString *str = NULL;
	gchar buffer[8192];
	gssize read = 0, size = 0;

	/* now load the html */
	istream = g_resource_open_stream(pidgin_get_resource(),
	                                 "/im/pidgin/Pidgin3/About/about.md",
	                                 G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);

	str = g_string_new("");

	while((read = g_input_stream_read(istream, buffer, sizeof(buffer), NULL, NULL)) > 0) {
		g_string_append_len(str, (gchar *)buffer, read);
		size += read;
	}

	gtk_text_buffer_get_start_iter(about->main_buffer, &start);

	talkatu_markdown_insert(TALKATU_BUFFER(about->main_buffer), &start,
	                        str->str, size);

	g_string_free(str, TRUE);

	g_input_stream_close(istream, NULL, NULL);
}

static void
pidgin_about_dialog_group_add_row(AdwPreferencesGroup *group,
                                  const char *title, const char *value)
{
	GtkWidget *row = adw_action_row_new();

	adw_preferences_row_set_title(ADW_PREFERENCES_ROW(row), title);

	if(value != NULL) {
		GtkWidget *label = gtk_label_new(NULL);
		gtk_label_set_markup(GTK_LABEL(label), value);
		adw_action_row_add_suffix(ADW_ACTION_ROW(row), label);
	}

	adw_preferences_group_add(group, row);
}

static void
pidgin_about_dialog_load_json(AdwPreferencesPage *page,
                              const char *json_section)
{
	GInputStream *istream = NULL;
	GList *l = NULL, *sections = NULL;
	GError *error = NULL;
	JsonParser *parser = NULL;
	JsonNode *root_node = NULL;
	JsonObject *root_object = NULL;
	JsonArray *sections_array = NULL;

	/* get a stream to the credits resource */
	istream = g_resource_open_stream(pidgin_get_resource(),
	                                 "/im/pidgin/Pidgin3/About/credits.json",
	                                 G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);

	/* create our parser */
	parser = json_parser_new();

	if(!json_parser_load_from_stream(parser, istream, NULL, &error)) {
		g_critical("%s", error->message);
	}

	root_node = json_parser_get_root(parser);
	root_object = json_node_get_object(root_node);

	sections_array = json_object_get_array_member(root_object, json_section);
	sections = json_array_get_elements(sections_array);

	for(l = sections; l; l = l->next) {
		JsonObject *section = json_node_get_object(l->data);
		JsonArray *people = NULL;
		const gchar *title = NULL;
		AdwPreferencesGroup *group = NULL;
		guint n_people = 0;

		title = json_object_get_string_member(section, "title");
		group = ADW_PREFERENCES_GROUP(adw_preferences_group_new());
		adw_preferences_group_set_title(group, title);
		adw_preferences_page_add(page, group);

		people = json_object_get_array_member(section, "people");
		n_people = json_array_get_length(people);

		for(guint idx = 0; idx < n_people; idx++) {
			const char *name = json_array_get_string_element(people, idx);
			pidgin_about_dialog_group_add_row(group, name, NULL);
		}
	}

	g_list_free(sections);

	/* clean up */
	g_object_unref(G_OBJECT(parser));

	g_input_stream_close(istream, NULL, NULL);
}

static void
pidgin_about_dialog_load_developers(PidginAboutDialog *about) {
	pidgin_about_dialog_load_json(about->developers_page, "developers");
}

static void
pidgin_about_dialog_load_translators(PidginAboutDialog *about) {
	pidgin_about_dialog_load_json(about->translators_page, "languages");
}

static void
pidgin_about_dialog_build_info_add_version(AdwPreferencesGroup *group,
                                           const gchar *title,
                                           guint major,
                                           guint minor,
                                           guint micro)
{
	gchar *version = g_strdup_printf("%u.%u.%u", major, minor, micro);

	pidgin_about_dialog_group_add_row(group, title, version);

	g_free(version);
}

static void
pidgin_about_dialog_load_build_info(PidginAboutDialog *about) {

	/* add the commit hash */
	pidgin_about_dialog_group_add_row(about->build_info_group, "Commit Hash",
	                                  REVISION);

	/* add the purple version */
	pidgin_about_dialog_build_info_add_version(about->build_info_group,
	                                           _("Purple Version"),
	                                           PURPLE_MAJOR_VERSION,
	                                           PURPLE_MINOR_VERSION,
	                                           PURPLE_MICRO_VERSION);

	/* add the glib version */
	pidgin_about_dialog_build_info_add_version(about->build_info_group,
	                                           _("GLib Version"),
	                                           GLIB_MAJOR_VERSION,
	                                           GLIB_MINOR_VERSION,
	                                           GLIB_MICRO_VERSION);

	/* add the gtk version */
	pidgin_about_dialog_build_info_add_version(about->build_info_group,
	                                           _("GTK Version"),
	                                           GTK_MAJOR_VERSION,
	                                           GTK_MINOR_VERSION,
	                                           GTK_MICRO_VERSION);
}

static char *
pidgin_about_dialog_copy_build_info(void) {
	char *info = NULL;

	info = g_strdup_printf(
		"Build Information\n"
		"=================\n"
		"Commit Hash: %s\n"
		"Purple Version: %u.%u.%u\n"
		"GLib Version: %u.%u.%u\n"
		"GTK Version: %u.%u.%u\n",
		REVISION,
		PURPLE_MAJOR_VERSION, PURPLE_MINOR_VERSION, PURPLE_MICRO_VERSION,
		GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, GLIB_MICRO_VERSION,
		GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION);

	return info;
}

static void
pidgin_about_dialog_load_runtime_info(PidginAboutDialog *about) {
	/* add the purple version */
	pidgin_about_dialog_build_info_add_version(about->runtime_info_group,
	                                           _("Purple Version"),
	                                           purple_major_version,
	                                           purple_minor_version,
	                                           purple_micro_version);

	/* add the glib version */
	pidgin_about_dialog_build_info_add_version(about->runtime_info_group,
	                                           _("GLib Version"),
	                                           glib_major_version,
	                                           glib_minor_version,
	                                           glib_micro_version);

	/* add the gtk version */
	pidgin_about_dialog_build_info_add_version(about->runtime_info_group,
	                                           _("GTK Version"),
	                                           gtk_get_major_version(),
	                                           gtk_get_minor_version(),
	                                           gtk_get_micro_version());
}

static char *
pidgin_about_dialog_copy_runtime_info(void) {
	char *info = NULL;

	info = g_strdup_printf(
		"Runtime Information\n"
		"===================\n"
		"Purple Version: %u.%u.%u\n"
		"GLib Version: %u.%u.%u\n"
		"GTK Version: %u.%u.%u\n",
		purple_major_version, purple_minor_version, purple_micro_version,
		glib_major_version, glib_minor_version, glib_micro_version,
		gtk_get_major_version(), gtk_get_minor_version(), gtk_get_micro_version());

	return info;
}

static void
pidgin_about_dialog_load_gtk_settings(PidginAboutDialog *about) {
	gchar *cursor_theme_name = NULL, *theme_name = NULL;
	gchar *icon_theme_name = NULL;
	gchar *im_module = NULL;
	gchar *sound_theme_name = NULL;
	gboolean enable_animations = FALSE;
	gboolean shell_shows_app_menu = FALSE, shell_shows_menubar = FALSE;

	/* get the settings we're interested in */
	g_object_get(
		gtk_settings_get_default(),
		"gtk-cursor-theme-name", &cursor_theme_name,
		"gtk-enable-animations", &enable_animations,
		"gtk-icon-theme-name", &icon_theme_name,
		"gtk-im-module", &im_module,
		"gtk-shell-shows-app-menu", &shell_shows_app_menu,
		"gtk-shell-shows-menubar", &shell_shows_menubar,
		"gtk-sound-theme-name", &sound_theme_name,
		"gtk-theme-name", &theme_name,
		NULL);

	pidgin_about_dialog_group_add_row(
		about->gtk_settings_group,
		"gtk-cursor-theme-name",
		(cursor_theme_name != NULL) ? cursor_theme_name : _("(not set)"));

	pidgin_about_dialog_group_add_row(
		about->gtk_settings_group,
		"gtk-enable-animations",
		enable_animations ? _("yes") : _("no"));

	pidgin_about_dialog_group_add_row(
		about->gtk_settings_group,
		"gtk-icon-theme-name",
		(icon_theme_name != NULL) ? icon_theme_name : _("(not set)"));

	pidgin_about_dialog_group_add_row(
		about->gtk_settings_group,
		"gtk-im-module",
		(im_module != NULL) ? im_module : _("(not set)"));

	pidgin_about_dialog_group_add_row(
		about->gtk_settings_group,
		"gtk-shell-shows-app-menu",
		shell_shows_app_menu ? _("yes") : _("no"));

	pidgin_about_dialog_group_add_row(
		about->gtk_settings_group,
		"gtk-shell-shows-menubar",
		shell_shows_menubar ? _("yes") : _("no"));

	pidgin_about_dialog_group_add_row(
		about->gtk_settings_group,
		"gtk-sound-theme-name",
		(sound_theme_name != NULL) ? sound_theme_name : _("(not set)"));

	pidgin_about_dialog_group_add_row(
		about->gtk_settings_group,
		"gtk-theme-name",
		(theme_name != NULL) ? theme_name : _("(not set)"));

	g_free(cursor_theme_name);
	g_free(icon_theme_name);
	g_free(im_module);
	g_free(sound_theme_name);
	g_free(theme_name);
}

static char *
pidgin_about_dialog_copy_gtk_settings(void) {
	char *cursor_theme_name = NULL, *theme_name = NULL;
	char *icon_theme_name = NULL;
	char *im_module = NULL;
	char *sound_theme_name = NULL;
	gboolean enable_animations = FALSE;
	gboolean shell_shows_app_menu = FALSE, shell_shows_menubar = FALSE;
	GString *info = NULL;

	/* get the settings we're interested in */
	g_object_get(
		gtk_settings_get_default(),
		"gtk-cursor-theme-name", &cursor_theme_name,
		"gtk-enable-animations", &enable_animations,
		"gtk-icon-theme-name", &icon_theme_name,
		"gtk-im-module", &im_module,
		"gtk-shell-shows-app-menu", &shell_shows_app_menu,
		"gtk-shell-shows-menubar", &shell_shows_menubar,
		"gtk-sound-theme-name", &sound_theme_name,
		"gtk-theme-name", &theme_name,
		NULL);

	info = g_string_new(
		"GTK Settings\n"
		"============\n");

	g_string_append_printf(
		info, "gtk-cursor-theme-name: %s\n",
		(cursor_theme_name != NULL) ? cursor_theme_name : _("(not set)"));

	g_string_append_printf(info, "gtk-enable-animations: %s\n",
	                       enable_animations ? _("yes") : _("no"));

	g_string_append_printf(
		info, "gtk-icon-theme-name: %s\n",
		(icon_theme_name != NULL) ? icon_theme_name : _("(not set)"));

	g_string_append_printf(info, "gtk-im-module: %s\n",
	                       (im_module != NULL) ? im_module : _("(not set)"));

	g_string_append_printf(info, "gtk-shell-shows-app-menu: %s\n",
	                       shell_shows_app_menu ? _("yes") : _("no"));

	g_string_append_printf(info, "gtk-shell-shows-menubar: %s\n",
	                       shell_shows_menubar ? _("yes") : _("no"));

	g_string_append_printf(
		info, "gtk-sound-theme-name: %s\n",
		(sound_theme_name != NULL) ? sound_theme_name : _("(not set)"));

	g_string_append_printf(info, "gtk-theme-name: %s\n",
	                       (theme_name != NULL) ? theme_name : _("(not set)"));

	g_free(cursor_theme_name);
	g_free(icon_theme_name);
	g_free(im_module);
	g_free(sound_theme_name);
	g_free(theme_name);

	return g_string_free(info, FALSE);
}

static void
pidgin_about_dialog_load_plugin_search_paths(PidginAboutDialog *about) {
	GList *paths = NULL;
	GPluginManager *manager = gplugin_manager_get_default();

	/* add the search paths */
	paths = gplugin_manager_get_paths(manager);
	while(paths != NULL) {
		pidgin_about_dialog_group_add_row(about->plugin_search_paths_group,
		                                  paths->data, NULL);

		paths = paths->next;
	}
}

static char *
pidgin_about_dialog_copy_plugin_search_paths(void) {
	GList *paths = NULL;
	GPluginManager *manager = gplugin_manager_get_default();
	GString *info = NULL;

	info = g_string_new(
		"Plugin Search Paths\n"
		"===================\n");

	/* add the search paths */
	paths = gplugin_manager_get_paths(manager);
	while(paths != NULL) {
		g_string_append_printf(info, "- %s\n", (char *)paths->data);

		paths = paths->next;
	}

	return g_string_free(info, FALSE);
}

static void
pidgin_about_dialog_load_conf_path_info(PidginAboutDialog *about) {
	/* add the cache directory path */
	pidgin_about_dialog_group_add_row(about->conf_path_info_group, _("Cache"),
	                                  purple_cache_dir());

	/* add the config directory path */
	pidgin_about_dialog_group_add_row(about->conf_path_info_group,
	                                  _("Configuration"), purple_config_dir());

	/* add the data directory path */
	pidgin_about_dialog_group_add_row(about->conf_path_info_group, _("Data"),
	                                  purple_data_dir());
}

static char *
pidgin_about_dialog_copy_conf_path_info(void) {
	char *info = NULL;

	info = g_strdup_printf(
		"Runtime Directories\n"
		"===================\n"
		"Cache: %s\n"
		"Configuration: %s\n"
		"Data: %s\n",
		purple_cache_dir(),
		purple_config_dir(),
		purple_data_dir());

	return info;
}

static void
pidgin_about_dialog_add_build_args(PidginAboutDialog *about,
                                   const char *build_args)
{
	gchar **splits = NULL;

	/* Walk through the arguments and add them */
	splits = g_strsplit(build_args, " ", -1);
	for(gint idx = 0; splits[idx]; idx++) {
		gchar **value_split = g_strsplit(splits[idx], "=", 2);

		if(value_split[0] == NULL || value_split[0][0] == '\0') {
			continue;
		}

		pidgin_about_dialog_group_add_row(about->build_args_group,
		                                  value_split[0], value_split[1]);

		g_strfreev(value_split);
	}

	g_strfreev(splits);
}

static char *
pidgin_about_dialog_copy_build_args(const char *build_args) {
	char **splits = NULL;
	GString *info = NULL;

	info = g_string_new(
		"Meson Arguments\n"
		"===============\n");

	/* Walk through the arguments and add them */
	splits = g_strsplit(build_args, " ", -1);
	for(gint idx = 0; splits[idx]; idx++) {
		char **value_split = g_strsplit(splits[idx], "=", 2);
		char *value = NULL;

		if(value_split[0] == NULL || value_split[0][0] == '\0') {
			continue;
		}

		if(value_split[1] != NULL) {
			value = purple_unescape_text(value_split[1]);
		} else {
			value = NULL;
		}

		g_string_append_printf(info, "%s: %s\n", value_split[0], value);

		g_free(value);
		g_strfreev(value_split);
	}

	g_strfreev(splits);

	return g_string_free(info, FALSE);
}

static void
pidgin_about_dialog_load_build_configuration(PidginAboutDialog *about) {
	pidgin_about_dialog_load_build_info(about);
	pidgin_about_dialog_load_runtime_info(about);
	pidgin_about_dialog_load_gtk_settings(about);
	pidgin_about_dialog_load_plugin_search_paths(about);
	pidgin_about_dialog_load_conf_path_info(about);

#ifdef MESON_ARGS
	pidgin_about_dialog_add_build_args(about, MESON_ARGS);
	gtk_widget_set_visible(GTK_WIDGET(about->build_args_group), TRUE);
#endif /* MESON_ARGS */
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_about_dialog_response_cb(GtkDialog *dialog, gint response_id,
                                G_GNUC_UNUSED gpointer data)
{
	switch(response_id) {
		case GTK_RESPONSE_CLOSE:
		case GTK_RESPONSE_DELETE_EVENT:
			gtk_window_destroy(GTK_WINDOW(dialog));
			break;
	}
}

static void
pidgin_about_dialog_open_url_cb(G_GNUC_UNUSED TalkatuView *view,
                                const char *url, gpointer data)
{
	gtk_show_uri(GTK_WINDOW(data), url, GDK_CURRENT_TIME);
}

static void
pidgin_about_dialog_copy_button_cb(GtkButton *button,
                                   gpointer data)
{
	PidginAboutDialog *about = NULL;
	GdkClipboard *clipboard = NULL;
	char *info = NULL;

	about = PIDGIN_ABOUT_DIALOG(gtk_widget_get_root(GTK_WIDGET(button)));

	if(data == about->build_info_group) {
		info = pidgin_about_dialog_copy_build_info();
	} else if(data == about->runtime_info_group) {
		info = pidgin_about_dialog_copy_runtime_info();
	} else if(data == about->gtk_settings_group) {
		info = pidgin_about_dialog_copy_gtk_settings();
	} else if(data == about->plugin_search_paths_group) {
		info = pidgin_about_dialog_copy_plugin_search_paths();
	} else if(data == about->conf_path_info_group) {
		info = pidgin_about_dialog_copy_conf_path_info();
#ifdef MESON_ARGS
	} else if(data == about->build_args_group) {
		info = pidgin_about_dialog_copy_build_args(MESON_ARGS);
#endif
	} else {
		GString *everything = g_string_new(NULL);

		info = pidgin_about_dialog_copy_build_info();
		g_string_append(everything, info);
		g_string_append_c(everything, '\n');
		g_free(info);

		info = pidgin_about_dialog_copy_runtime_info();
		g_string_append(everything, info);
		g_string_append_c(everything, '\n');
		g_free(info);

		info = pidgin_about_dialog_copy_conf_path_info();
		g_string_append(everything, info);
		g_string_append_c(everything, '\n');
		g_free(info);

		info = pidgin_about_dialog_copy_gtk_settings();
		g_string_append(everything, info);
		g_string_append_c(everything, '\n');
		g_free(info);

		info = pidgin_about_dialog_copy_plugin_search_paths();
		g_string_append(everything, info);
		g_free(info);

#ifdef MESON_ARGS
		g_string_append_c(everything, '\n');
		info = pidgin_about_dialog_copy_build_args(MESON_ARGS);
		g_string_append(everything, info);
		g_free(info);
#endif

		info = g_string_free(everything, FALSE);
	}

	clipboard = gtk_widget_get_clipboard(GTK_WIDGET(about));
	gdk_clipboard_set_text(clipboard, info);

	g_free(info);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(PidginAboutDialog, pidgin_about_dialog, GTK_TYPE_DIALOG);

static void
pidgin_about_dialog_class_init(PidginAboutDialogClass *klass) {
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/im/pidgin/Pidgin3/About/about.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginAboutDialog,
	                                     application_name);

	gtk_widget_class_bind_template_child(widget_class, PidginAboutDialog,
	                                     main_buffer);

	gtk_widget_class_bind_template_child(widget_class, PidginAboutDialog,
	                                     developers_page);

	gtk_widget_class_bind_template_child(widget_class, PidginAboutDialog,
	                                     translators_page);

	gtk_widget_class_bind_template_child(widget_class, PidginAboutDialog,
	                                     build_info_group);
	gtk_widget_class_bind_template_child(widget_class, PidginAboutDialog,
	                                     runtime_info_group);
	gtk_widget_class_bind_template_child(widget_class, PidginAboutDialog,
	                                     gtk_settings_group);
	gtk_widget_class_bind_template_child(widget_class, PidginAboutDialog,
	                                     plugin_search_paths_group);
	gtk_widget_class_bind_template_child(widget_class, PidginAboutDialog,
	                                     conf_path_info_group);
	gtk_widget_class_bind_template_child(widget_class, PidginAboutDialog,
	                                     build_args_group);

	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_about_dialog_response_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_about_dialog_open_url_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_about_dialog_copy_button_cb);
}

static void
pidgin_about_dialog_init(PidginAboutDialog *about) {
	gtk_widget_init_template(GTK_WIDGET(about));

	/* setup the application name label */
	pidgin_about_dialog_load_application_name(about);

	/* setup the main page */
	pidgin_about_dialog_load_main_page(about);

	/* setup the developers stuff */
	pidgin_about_dialog_load_developers(about);

	/* setup the translators stuff */
	pidgin_about_dialog_load_translators(about);

	/* setup the build info page */
	pidgin_about_dialog_load_build_configuration(about);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
pidgin_about_dialog_new(void) {
	return g_object_new(
		PIDGIN_TYPE_ABOUT_DIALOG,
		"title", "About Pidgin",
		NULL
	);
}
