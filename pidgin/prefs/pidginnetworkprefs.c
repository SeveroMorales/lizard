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

#include "pidginnetworkprefs.h"
#include "pidginprefsinternal.h"

struct _PidginNetworkPrefs {
	AdwPreferencesPage parent;

	GtkWidget *stun_server;
	GtkWidget *auto_ip_row;
	GtkWidget *auto_ip;
	GtkWidget *public_ip;
	GtkWidget *map_ports;
	GtkWidget *ports_range_use;
	GtkWidget *ports_range_start;
	GtkWidget *ports_range_end;
};

G_DEFINE_TYPE(PidginNetworkPrefs, pidgin_network_prefs,
              ADW_TYPE_PREFERENCES_PAGE)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
network_ip_changed(GtkEditable *editable, G_GNUC_UNUSED gpointer data)
{
	const gchar *text = gtk_editable_get_text(editable);
	GtkWidget *widget = GTK_WIDGET(editable);

	if (text && *text) {
		if (g_hostname_is_ip_address(text)) {
			purple_network_set_public_ip(text);
			gtk_widget_remove_css_class(widget, "error");
			gtk_widget_add_css_class(widget, "success");
		} else {
			gtk_widget_remove_css_class(widget, "success");
			gtk_widget_add_css_class(widget, "error");
		}

	} else {
		purple_network_set_public_ip("");
		gtk_widget_remove_css_class(widget, "success");
		gtk_widget_remove_css_class(widget, "error");
	}
}

static void
network_stun_server_changed_cb(G_GNUC_UNUSED GtkEventControllerFocus *focus,
                               gpointer data)
{
	GtkEditable *editable = data;

	purple_prefs_set_string("/purple/network/stun_server",
	                        gtk_editable_get_text(editable));
	purple_network_set_stun_server(gtk_editable_get_text(editable));
}

static void
auto_ip_button_clicked_cb(G_GNUC_UNUSED GObject *obj,
                          G_GNUC_UNUSED GParamSpec *pspec, gpointer data)
{
	PidginNetworkPrefs *prefs = PIDGIN_NETWORK_PREFS(data);
	const char *ip;
	PurpleStunNatDiscovery *stun;
	char *auto_ip_text;
	GList *list = NULL;

	/* Make a lookup for the auto-detected IP ourselves. */
	if (purple_prefs_get_bool("/purple/network/auto_ip")) {
		/* Check if STUN discovery was already done */
		stun = purple_stun_discover(NULL);
		if ((stun != NULL) && (stun->status == PURPLE_STUN_STATUS_DISCOVERED)) {
			ip = stun->publicip;
		} else {
			ip = "0.0.0.0";
		}
	} else {
		ip = _("Disabled");
	}

	auto_ip_text = g_strdup_printf(_("Use _automatically detected IP address: %s"), ip);
	adw_preferences_row_set_title(ADW_PREFERENCES_ROW(prefs->auto_ip_row),
	                              auto_ip_text);
	g_free(auto_ip_text);
	g_list_free_full(list, g_free);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_network_prefs_class_init(PidginNetworkPrefsClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/im/pidgin/Pidgin3/Prefs/network.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginNetworkPrefs,
	                                     stun_server);
	gtk_widget_class_bind_template_child(widget_class, PidginNetworkPrefs,
	                                     auto_ip_row);
	gtk_widget_class_bind_template_child(widget_class, PidginNetworkPrefs,
	                                     auto_ip);
	gtk_widget_class_bind_template_child(widget_class, PidginNetworkPrefs,
	                                     public_ip);
	gtk_widget_class_bind_template_child(widget_class, PidginNetworkPrefs,
	                                     map_ports);
	gtk_widget_class_bind_template_child(widget_class, PidginNetworkPrefs,
	                                     ports_range_use);
	gtk_widget_class_bind_template_child(widget_class, PidginNetworkPrefs,
	                                     ports_range_start);
	gtk_widget_class_bind_template_child(widget_class, PidginNetworkPrefs,
	                                     ports_range_end);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        network_stun_server_changed_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        auto_ip_button_clicked_cb);
	gtk_widget_class_bind_template_callback(widget_class, network_ip_changed);
}

static void
pidgin_network_prefs_init(PidginNetworkPrefs *prefs)
{
	gtk_widget_init_template(GTK_WIDGET(prefs));

	gtk_editable_set_text(GTK_EDITABLE(prefs->stun_server),
	                      purple_prefs_get_string("/purple/network/stun_server"));

	pidgin_prefs_bind_switch("/purple/network/auto_ip", prefs->auto_ip);
	auto_ip_button_clicked_cb(NULL, NULL, prefs); /* Update label */

	gtk_editable_set_text(GTK_EDITABLE(prefs->public_ip),
	                      purple_network_get_public_ip());

	pidgin_prefs_bind_switch("/purple/network/map_ports", prefs->map_ports);

	pidgin_prefs_bind_expander_row("/purple/network/ports_range_use",
	                               prefs->ports_range_use);

	pidgin_prefs_bind_spin_button("/purple/network/ports_range_start",
			prefs->ports_range_start);
	pidgin_prefs_bind_spin_button("/purple/network/ports_range_end",
			prefs->ports_range_end);
}

/******************************************************************************
 * API
 *****************************************************************************/
GtkWidget *
pidgin_network_prefs_new(void) {
	return GTK_WIDGET(g_object_new(PIDGIN_TYPE_NETWORK_PREFS, NULL));
}
