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

#include "pidginawayprefs.h"
#include "gtksavedstatuses.h"
#include "gtkutils.h"
#include "pidginprefsinternal.h"

struct _PidginAwayPrefs {
	AdwPreferencesPage parent;

	GtkWidget *idle_reporting;
	GtkWidget *mins_before_away;
	GtkWidget *idle_row;
	GtkWidget *away_when_idle;
	GtkWidget *startup_current_status;
	GtkWidget *startup_row;
};

G_DEFINE_TYPE(PidginAwayPrefs, pidgin_away_prefs, ADW_TYPE_PREFERENCES_PAGE)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static gchar *
idle_reporting_expression_cb(GObject *self, G_GNUC_UNUSED gpointer data)
{
	const gchar *text = "";
	const gchar *value = NULL;

	value = gtk_string_object_get_string(GTK_STRING_OBJECT(self));
	if(purple_strequal(value, "none")) {
		text = _("Never");
	} else if(purple_strequal(value, "purple")) {
		text = _("From last sent message");
	} else if(purple_strequal(value, "system")) {
		text = _("Based on keyboard or mouse use");
	}

	return g_strdup(text);
}

static void
set_idle_away(PurpleSavedStatus *status)
{
	purple_prefs_set_int("/purple/savedstatus/idleaway",
	                     purple_savedstatus_get_creation_time(status));
}

static void
set_startupstatus(PurpleSavedStatus *status)
{
	purple_prefs_set_int("/purple/savedstatus/startup",
	                     purple_savedstatus_get_creation_time(status));
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_away_prefs_class_init(PidginAwayPrefsClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(
	    widget_class,
	    "/im/pidgin/Pidgin3/Prefs/away.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginAwayPrefs,
	                                     idle_reporting);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        idle_reporting_expression_cb);
	gtk_widget_class_bind_template_child(widget_class, PidginAwayPrefs,
	                                     mins_before_away);
	gtk_widget_class_bind_template_child(widget_class, PidginAwayPrefs,
	                                     away_when_idle);
	gtk_widget_class_bind_template_child(widget_class, PidginAwayPrefs,
	                                     idle_row);
	gtk_widget_class_bind_template_child(widget_class, PidginAwayPrefs,
	                                     startup_current_status);
	gtk_widget_class_bind_template_child(widget_class, PidginAwayPrefs,
	                                     startup_row);
}

static void
pidgin_away_prefs_init(PidginAwayPrefs *prefs)
{
	GtkWidget *menu;

	gtk_widget_init_template(GTK_WIDGET(prefs));

	pidgin_prefs_bind_combo_row("/purple/away/idle_reporting",
	                            prefs->idle_reporting);

	pidgin_prefs_bind_spin_button("/purple/away/mins_before_away",
			prefs->mins_before_away);

	pidgin_prefs_bind_switch("/purple/away/away_when_idle",
	                         prefs->away_when_idle);

	/* TODO: Show something useful if we don't have any saved statuses. */
	menu = pidgin_status_menu(purple_savedstatus_get_idleaway(),
	                          G_CALLBACK(set_idle_away));
	gtk_widget_set_valign(menu, GTK_ALIGN_CENTER);
	adw_action_row_add_suffix(ADW_ACTION_ROW(prefs->idle_row), menu);
	adw_action_row_set_activatable_widget(ADW_ACTION_ROW(prefs->idle_row),
	                                      menu);

	g_object_bind_property(prefs->away_when_idle, "active",
			menu, "sensitive",
			G_BINDING_SYNC_CREATE);

	/* Signon status stuff */
	pidgin_prefs_bind_switch("/purple/savedstatus/startup_current_status",
	                         prefs->startup_current_status);

	/* TODO: Show something useful if we don't have any saved statuses. */
	menu = pidgin_status_menu(purple_savedstatus_get_startup(),
	                          G_CALLBACK(set_startupstatus));
	gtk_widget_set_valign(menu, GTK_ALIGN_CENTER);
	adw_action_row_add_suffix(ADW_ACTION_ROW(prefs->startup_row), menu);
	adw_action_row_set_activatable_widget(ADW_ACTION_ROW(prefs->startup_row),
	                                      menu);
}

/******************************************************************************
 * API
 *****************************************************************************/
GtkWidget *
pidgin_away_prefs_new(void) {
	return GTK_WIDGET(g_object_new(PIDGIN_TYPE_AWAY_PREFS, NULL));
}
