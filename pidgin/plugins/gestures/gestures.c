/*
 * Mouse gestures plugin for Purple
 *
 * Copyright (C) 2003 Christian Hammond.
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

#include <purple.h>

#include <pidgin.h>

#include "gstroke.h"

#define GESTURES_PLUGIN_ID "gtk-x11-gestures"

static void
stroke_close(GtkWidget *widget, void *data)
{
	PurpleConversation *conv;
	PidginConversation *gtkconv;

	conv = (PurpleConversation *)data;

	/* Double-check */
	if (!PIDGIN_IS_PIDGIN_CONVERSATION(conv))
		return;

	gtkconv = PIDGIN_CONVERSATION(conv);

	gstroke_cleanup(gtkconv->history);
	g_object_unref(conv);
}

static void
stroke_prev_tab(GtkWidget *widget, void *data)
{
	PurpleConversation *conv;
	PidginConversation *gtkconv;
	GtkWidget *win;

	conv = (PurpleConversation *)data;
	gtkconv = PIDGIN_CONVERSATION(conv);
	win = gtk_widget_get_toplevel(gtkconv->tab_cont);

	pidgin_display_window_select_previous(PIDGIN_DISPLAY_WINDOW(win));
}

static void
stroke_next_tab(GtkWidget *widget, void *data)
{
	PurpleConversation *conv;
	PidginConversation *gtkconv;
	GtkWidget *win;

	conv = (PurpleConversation *)data;
	gtkconv = PIDGIN_CONVERSATION(conv);
	win = gtk_widget_get_toplevel(gtkconv->tab_cont);

	pidgin_display_window_select_next(PIDGIN_DISPLAY_WINDOW(win));
}

static void
stroke_new_win(GtkWidget *widget, void *data)
{
	GtkWidget *new_win, *old_win;
	PurpleConversation *conv;

	conv    = (PurpleConversation *)data;
	old_win = gtk_widget_get_parent(PIDGIN_CONVERSATION(conv)->tab_cont);

	if(pidgin_display_window_get_count(PIDGIN_DISPLAY_WINDOW(old_win)) <= 1) {
		return;
	}

	new_win = pidgin_display_window_new();

	pidgin_display_window_remove(PIDGIN_DISPLAY_WINDOW(old_win),
	                                  conv);
	pidgin_display_window_add(PIDGIN_DISPLAY_WINDOW(new_win), conv);

	gtk_widget_show_all(new_win);
}

static void
attach_signals(PurpleConversation *conv)
{
	PidginConversation *gtkconv;

	gtkconv = PIDGIN_CONVERSATION(conv);

	gstroke_enable(gtkconv->history);
	gstroke_signal_connect(gtkconv->history, "14789",  stroke_close,    conv);
	gstroke_signal_connect(gtkconv->history, "1456",   stroke_close,    conv);
	gstroke_signal_connect(gtkconv->history, "1489",   stroke_close,    conv);
	gstroke_signal_connect(gtkconv->history, "74123",  stroke_next_tab, conv);
	gstroke_signal_connect(gtkconv->history, "7456",   stroke_next_tab, conv);
	gstroke_signal_connect(gtkconv->history, "96321",  stroke_prev_tab, conv);
	gstroke_signal_connect(gtkconv->history, "9654",   stroke_prev_tab, conv);
	gstroke_signal_connect(gtkconv->history, "25852",  stroke_new_win,  conv);
}

static void
new_conv_cb(PurpleConversation *conv)
{
	if (PIDGIN_IS_PIDGIN_CONVERSATION(conv))
		attach_signals(conv);
}

#if 0
static void
mouse_button_menu_cb(GtkComboBox *opt, gpointer data)
{
	int button = gtk_combo_box_get_active(opt);

	gstroke_set_mouse_button(button + 2);
}
#endif

static void
toggle_draw_cb(GtkToggleButton *toggle, gpointer data)
{
	purple_prefs_set_bool("/plugins/gtk/X11/gestures/visual",
		gtk_toggle_button_get_active(toggle));
}

static void
visual_pref_cb(const char *name, PurplePrefType type, gconstpointer value,
			   gpointer data)
{
	gstroke_set_draw_strokes((gboolean) GPOINTER_TO_INT(value) );
}

static GtkWidget *
get_config_frame(PurplePlugin *plugin)
{
	GtkWidget *ret;
	GtkWidget *vbox;
	GtkWidget *toggle;
#if 0
	GtkWidget *opt;
#endif

	/* Outside container */
	ret = gtk_box_new(GTK_ORIENTATION_VERTICAL, 18);
	gtk_container_set_border_width(GTK_CONTAINER(ret), 12);

	/* Configuration frame */
	vbox = pidgin_make_frame(ret, _("Mouse Gestures Configuration"));

#if 0
	/* Mouse button drop-down menu */
	opt = gtk_combo_box_new_text();

	gtk_combo_box_append_text(_("Middle mouse button"));
	gtk_combo_box_append_text(_("Right mouse button"));
	g_signal_connect(G_OBJECT(opt), "changed",
	                 G_CALLBACK(mouse_button_menu_cb), NULL);

	gtk_box_pack_start(GTK_BOX(vbox), opt, FALSE, FALSE, 0);
	gtk_combo_box_set_active(GTK_COMBO_BOX(opt),
							gstroke_get_mouse_button() - 2);
#endif

	/* "Visual gesture display" checkbox */
	toggle = gtk_check_button_new_with_mnemonic(_("_Visual gesture display"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle),
			purple_prefs_get_bool("/plugins/gtk/X11/gestures/visual"));
	g_signal_connect(G_OBJECT(toggle), "toggled",
					 G_CALLBACK(toggle_draw_cb), NULL);

	gtk_widget_show_all(ret);

	return ret;
}

static GPluginPluginInfo *
gestures_query(GError **error)
{
	const gchar * const authors[] = {
		"Christian Hammond <chipx86@gnupdate.org>",
		NULL
	};

	return pidgin_plugin_info_new(
		"id",                   GESTURES_PLUGIN_ID,
		"name",                 N_("Mouse Gestures"),
		"version",              DISPLAY_VERSION,
		"category",             N_("User interface"),
		"summary",              N_("Provides support for mouse gestures"),
		"description",          N_("Allows support for mouse gestures in "
		                           "conversation windows. Drag the middle "
		                           "mouse button to perform certain "
		                           "actions:\n"
		                           " • Drag down and then to the right to "
		                           "close a conversation.\n"
		                           " • Drag up and then to the left to "
		                           "switch to the previous conversation.\n"
		                           " • Drag up and then to the right to "
		                           "switch to the next conversation."),
		"authors",              authors,
		"website",              PURPLE_WEBSITE,
		"abi-version",          PURPLE_ABI_VERSION,
		"gtk-config-frame-cb",  get_config_frame,
		NULL
	);
}

static gboolean
gestures_load(GPluginPlugin *plugin, GError **error)
{
	PurpleConversation *conv;
	PurpleConversationManager *manager;
	GList *list;

	purple_prefs_add_none("/plugins/gtk");
	purple_prefs_add_none("/plugins/gtk/X11");
	purple_prefs_add_none("/plugins/gtk/X11/gestures");
	purple_prefs_add_bool("/plugins/gtk/X11/gestures/visual", FALSE);

	purple_prefs_connect_callback(plugin,
		"/plugins/gtk/X11/gestures/visual", visual_pref_cb, NULL);
	gstroke_set_draw_strokes(purple_prefs_get_bool(
		"/plugins/gtk/X11/gestures/visual"));

	manager = purple_conversation_manager_get_default();
	list = purple_conversation_manager_get_all(manager);
	while(list != NULL) {
		conv = PURPLE_CONVERSATION(list->data);

		if(!PIDGIN_IS_PIDGIN_CONVERSATION(conv)) {
			list = g_list_delete_link(list, list);

			continue;
		}

		attach_signals(conv);

		list = g_list_delete_link(list, list);
	}

	purple_signal_connect(purple_conversations_get_handle(),
						"conversation-created",
						plugin, G_CALLBACK(new_conv_cb), NULL);

	return TRUE;
}

static gboolean
gestures_unload(GPluginPlugin *plugin, gboolean shutdown, GError **error)
{
	PurpleConversation *conv;
	PurpleConversationManager *manager;
	PidginConversation *gtkconv;
	GList *list;

	manager = purple_conversation_manager_get_default();
	list = purple_conversation_manager_get_all(manager);

	while(list != NULL) {
		conv = PURPLE_CONVERSATION(list->data);

		if (!PIDGIN_IS_PIDGIN_CONVERSATION(conv)) {
			list = g_list_delete_link(list, list);

			continue;
		}

		gtkconv = PIDGIN_CONVERSATION(conv);

		gstroke_cleanup(gtkconv->history);
		gstroke_disable(gtkconv->history);

		list = g_list_delete_link(list, list);
	}

	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(gestures)
