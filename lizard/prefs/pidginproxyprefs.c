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

#include <glib/gi18n-lib.h>

#include <purple.h>

#include <adwaita.h>

#include "pidginproxyprefs.h"
#include "pidginprefsinternal.h"

struct _PidginProxyPrefs {
	AdwPreferencesPage parent;

	/* GNOME version */
	GtkWidget *gnome;
	GtkWidget *gnome_not_found;
	GtkWidget *gnome_program;
	gchar *gnome_program_path;

	/* Non-GNOME version */
	GtkWidget *nongnome;
	GtkWidget *type;
	GtkWidget *options;
	GtkWidget *host;
	GtkWidget *port;
	GtkWidget *username;
	GtkWidget *password;
};

G_DEFINE_TYPE(PidginProxyPrefs, pidgin_proxy_prefs, ADW_TYPE_PREFERENCES_PAGE)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static gchar *
proxy_type_expression_cb(GObject *self, G_GNUC_UNUSED gpointer data)
{
	const gchar *text = "";
	const gchar *value = NULL;

	value = gtk_string_object_get_string(GTK_STRING_OBJECT(self));
	if(purple_strequal(value, "none")) {
		text = _("No proxy");
	} else if(purple_strequal(value, "socks4")) {
		text = _("SOCKS 4");
	} else if(purple_strequal(value, "socks5")) {
		text = _("SOCKS 5");
	} else if(purple_strequal(value, "tor")) {
		text = _("Tor/Privacy (SOCKS 5)");
	} else if(purple_strequal(value, "http")) {
		text = _("HTTP");
	} else if(purple_strequal(value, "envvar")) {
		text = _("Use Environmental Settings");
	}

	return g_strdup(text);
}

static void
pidgin_proxy_prefs_type_changed_cb(GSettings *settings, char *key,
                                   gpointer data)
{
	PidginProxyPrefs *prefs = data;
	char *current = g_settings_get_string(settings, key);

	gtk_widget_set_visible(prefs->options,
	                       !purple_strequal(current, "No Proxy") &&
	                       !purple_strequal(current, "Use Environmental Settings"));

	g_free(current);
}

static void
proxy_row_activated_cb(G_GNUC_UNUSED AdwActionRow *row, gpointer data)
{
	PidginProxyPrefs *prefs = data;
	GError *err = NULL;

	if (g_spawn_command_line_async(prefs->gnome_program_path, &err)) {
		return;
	}

	purple_notify_error(NULL, NULL, _("Cannot start proxy configuration program."), err->message, NULL);
	g_error_free(err);
}

static gboolean
pidgin_proxy_prefs_get_type_mapping(GValue *value, GVariant *variant,
                                    G_GNUC_UNUSED gpointer data)
{
	const char *current = g_variant_get_string(variant, NULL);
	guint position = 0;

	/* The values for position are dependent on the order of the GtkStringList
	 * in prefs/proxy.ui.
	 */
	if(purple_strequal(current, "No Proxy")) {
		position = 0;
	} else if(purple_strequal(current, "SOCKS4")) {
		position = 1;
	} else if(purple_strequal(current, "SOCKS5")) {
		position = 2;
	} else if(purple_strequal(current, "TOR")) {
		position = 3;
	} else if(purple_strequal(current, "HTTP")) {
		position = 4;
	} else if(purple_strequal(current, "Use Environmental Settings")) {
		position = 5;
	} else {
		return FALSE;
	}

	g_value_set_uint(value, position);

	return TRUE;
}

static GVariant *
pidgin_proxy_prefs_set_type_mapping(const GValue *gvalue,
                                    G_GNUC_UNUSED const GVariantType *expected_type,
                                    G_GNUC_UNUSED gpointer data)
{
	guint position = g_value_get_uint(gvalue);

	/* The index of these items is dependent on the order of the GtkStringList
	 * in prefs/proxy.ui.
	 */
	const char *map[] = {
		"No Proxy", "SOCKS4", "SOCKS5", "TOR", "HTTP",
		"Use Environmental Settings",
	};

	if(position >= G_N_ELEMENTS(map)) {
		return NULL;
	}

	return g_variant_new_string(map[position]);
}

static gboolean
pidgin_proxy_prefs_get_port_mapping(GValue *value, GVariant *variant,
                                    G_GNUC_UNUSED gpointer data)
{
	g_value_take_string(value,
	                    g_strdup_printf("%d", g_variant_get_uint16(variant)));

	return TRUE;
}

static GVariant *
pidgin_proxy_prefs_set_port_mapping(const GValue *value,
                                    G_GNUC_UNUSED const GVariantType *expected_type,
                                    G_GNUC_UNUSED gpointer data)
{
	const char *current = g_value_get_string(value);

	if(current != NULL) {
		return g_variant_new_uint16(atoi(current) & 0xFFFF);
	}

	return NULL;
}

static void
pidgin_proxy_prefs_init_gnome(PidginProxyPrefs *prefs) {
	gchar *path = NULL;

	gtk_widget_set_visible(prefs->gnome, TRUE);
	gtk_widget_set_visible(prefs->nongnome, FALSE);

	path = g_find_program_in_path("gnome-network-properties");
	if (path == NULL) {
		path = g_find_program_in_path("gnome-network-preferences");
	}
	if (path == NULL) {
		path = g_find_program_in_path("gnome-control-center");
		if (path != NULL) {
			char *tmp = g_strdup_printf("%s network", path);
			g_free(path);
			path = tmp;
		}
	}

	prefs->gnome_program_path = path;
	gtk_widget_set_visible(prefs->gnome_not_found, path == NULL);
	gtk_widget_set_visible(prefs->gnome_program, path != NULL);
}

static void
pidgin_proxy_prefs_init_non_gnome(PidginProxyPrefs *prefs) {
	GSettings *settings = NULL;
	gpointer settings_backend = NULL;

	settings_backend = purple_core_get_settings_backend();
	settings = g_settings_new_with_backend("im.pidgin.Purple.Proxy",
	                                       settings_backend);

	gtk_widget_set_visible(prefs->gnome, FALSE);
	gtk_widget_set_visible(prefs->nongnome, TRUE);

	g_settings_bind_with_mapping(settings, "type",
	                             prefs->type, "selected",
	                             G_SETTINGS_BIND_DEFAULT,
	                             pidgin_proxy_prefs_get_type_mapping,
	                             pidgin_proxy_prefs_set_type_mapping,
	                             NULL,
	                             NULL);

	g_settings_bind(settings, "host", prefs->host, "text",
	                G_SETTINGS_BIND_DEFAULT);
	g_settings_bind_with_mapping(settings, "port",
	                             prefs->port, "text",
	                             G_SETTINGS_BIND_DEFAULT,
	                             pidgin_proxy_prefs_get_port_mapping,
	                             pidgin_proxy_prefs_set_port_mapping,
	                             NULL,
	                             NULL);
	g_settings_bind(settings, "username", prefs->username, "text",
	                G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(settings, "password", prefs->password, "text",
	                G_SETTINGS_BIND_DEFAULT);

	g_signal_connect_object(settings, "changed::type",
	                        G_CALLBACK(pidgin_proxy_prefs_type_changed_cb),
	                        prefs, 0);

	/* Manually call the callback to set the initial visibility. */
	pidgin_proxy_prefs_type_changed_cb(settings, "type", prefs);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_proxy_prefs_finalize(GObject *obj)
{
	PidginProxyPrefs *prefs = PIDGIN_PROXY_PREFS(obj);

	purple_prefs_disconnect_by_handle(obj);

	g_free(prefs->gnome_program_path);

	G_OBJECT_CLASS(pidgin_proxy_prefs_parent_class)->finalize(obj);
}

static void
pidgin_proxy_prefs_class_init(PidginProxyPrefsClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->finalize = pidgin_proxy_prefs_finalize;

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/im/pidgin/Pidgin3/Prefs/proxy.ui"
	);

	gtk_widget_class_bind_template_child(
			widget_class, PidginProxyPrefs, gnome);
	gtk_widget_class_bind_template_child(
			widget_class, PidginProxyPrefs, gnome_not_found);
	gtk_widget_class_bind_template_child(
			widget_class, PidginProxyPrefs, gnome_program);

	gtk_widget_class_bind_template_child(
			widget_class, PidginProxyPrefs, nongnome);
	gtk_widget_class_bind_template_child(
			widget_class, PidginProxyPrefs, type);
	gtk_widget_class_bind_template_child(
			widget_class, PidginProxyPrefs, options);
	gtk_widget_class_bind_template_child(
			widget_class, PidginProxyPrefs, host);
	gtk_widget_class_bind_template_child(
			widget_class, PidginProxyPrefs, port);
	gtk_widget_class_bind_template_child(
			widget_class, PidginProxyPrefs, username);
	gtk_widget_class_bind_template_child(
			widget_class, PidginProxyPrefs, password);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        proxy_type_expression_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        proxy_row_activated_cb);
}

static void
pidgin_proxy_prefs_init(PidginProxyPrefs *prefs)
{

	gtk_widget_init_template(GTK_WIDGET(prefs));

	if(purple_running_gnome()) {
		pidgin_proxy_prefs_init_gnome(prefs);
	} else {
		pidgin_proxy_prefs_init_non_gnome(prefs);
	}
}

/******************************************************************************
 * API
 *****************************************************************************/
GtkWidget *
pidgin_proxy_prefs_new(void) {
	return g_object_new(PIDGIN_TYPE_PROXY_PREFS, NULL);
}
