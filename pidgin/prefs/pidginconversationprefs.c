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

#include "pidginconversationprefs.h"
#include "pidgincore.h"
#include "pidginprefsinternal.h"

struct _PidginConversationPrefs {
	AdwPreferencesPage parent;

	GtkWidget *show_incoming_formatting;
	struct {
		GtkWidget *send_typing;
	} im;
	struct {
		GtkWidget *blink_im_row;
		GtkWidget *blink_im;
	} win32;
	GtkWidget *minimum_entry_lines;
};

G_DEFINE_TYPE(PidginConversationPrefs, pidgin_conversation_prefs,
              ADW_TYPE_PREFERENCES_PAGE)

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_conversation_prefs_class_init(PidginConversationPrefsClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/im/pidgin/Pidgin3/Prefs/conversation.ui"
	);

	gtk_widget_class_bind_template_child(
			widget_class, PidginConversationPrefs,
			show_incoming_formatting);
	gtk_widget_class_bind_template_child(
			widget_class, PidginConversationPrefs,
			im.send_typing);
	gtk_widget_class_bind_template_child(
			widget_class, PidginConversationPrefs,
			win32.blink_im_row);
	gtk_widget_class_bind_template_child(
			widget_class, PidginConversationPrefs,
			win32.blink_im);
	gtk_widget_class_bind_template_child(
			widget_class, PidginConversationPrefs,
			minimum_entry_lines);
}

static void
pidgin_conversation_prefs_init(PidginConversationPrefs *prefs)
{
	gtk_widget_init_template(GTK_WIDGET(prefs));

	pidgin_prefs_bind_switch(PIDGIN_PREFS_ROOT "/conversations/show_incoming_formatting",
	                         prefs->show_incoming_formatting);

	pidgin_prefs_bind_switch("/purple/conversations/im/send_typing",
	                         prefs->im.send_typing);

#ifdef _WIN32
	pidgin_prefs_bind_switch(PIDGIN_PREFS_ROOT "/win32/blink_im",
	                         prefs->win32.blink_im);
#else
	gtk_widget_hide(prefs->win32.blink_im_row);
#endif

	pidgin_prefs_bind_spin_button(
		PIDGIN_PREFS_ROOT "/conversations/minimum_entry_lines",
		prefs->minimum_entry_lines);
}

/******************************************************************************
 * API
 *****************************************************************************/
GtkWidget *
pidgin_conversation_prefs_new(void) {
	return GTK_WIDGET(g_object_new(PIDGIN_TYPE_CONVERSATION_PREFS, NULL));
}
