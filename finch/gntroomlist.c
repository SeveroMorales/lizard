/*
 * finch
 *
 * Finch is the legal property of its developers, whose names are too numerous
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

#include <purple.h>

#include <gnt.h>

#include "gntrequest.h"
#include "gntroomlist.h"

#define PREF_ROOT "/finch/roomlist"


/* Yes, just one roomlist at a time. Let's not get greedy. Aight? */
struct _FinchRoomlist
{
	GntWidget *window;

	GntWidget *accounts;
	GntWidget *tree;
	GntWidget *details;

	GntWidget *getlist;
	GntWidget *add;
	GntWidget *join;
	GntWidget *stop;
	GntWidget *close;

	PurpleAccount *account;
	PurpleRoomlist *roomlist;
} froomlist;

typedef struct _FinchRoomlist FinchRoomlist;

static void
unset_roomlist(G_GNUC_UNUSED gpointer data)
{
	froomlist.window = NULL;
	g_clear_object(&froomlist.roomlist);
	froomlist.account = NULL;
	froomlist.tree = NULL;
}

static void
fl_stop(G_GNUC_UNUSED GntWidget *button, G_GNUC_UNUSED gpointer data) {
	if (froomlist.roomlist &&
			purple_roomlist_get_in_progress(froomlist.roomlist))
		purple_roomlist_cancel_get_list(froomlist.roomlist);
}

static void
fl_get_list(G_GNUC_UNUSED GntWidget *button, G_GNUC_UNUSED gpointer data) {
	PurpleAccount *account = gnt_combo_box_get_selected_data(GNT_COMBO_BOX(froomlist.accounts));
	PurpleConnection *gc = purple_account_get_connection(account);

	if (!gc)
		return;

	g_clear_object(&froomlist.roomlist);
	froomlist.roomlist = purple_roomlist_get_list(gc);
	gnt_box_give_focus_to_child(GNT_BOX(froomlist.window), froomlist.tree);
}

static void
fl_add_chat(G_GNUC_UNUSED GntWidget *button, G_GNUC_UNUSED gpointer data) {
	char *name;
	PurpleRoomlistRoom *room = gnt_tree_get_selection_data(GNT_TREE(froomlist.tree));
	PurpleConnection *gc = purple_account_get_connection(froomlist.account);
	PurpleProtocol *protocol = NULL;

	if (gc == NULL || room == NULL)
		return;

	protocol = purple_connection_get_protocol(gc);

	if (PURPLE_PROTOCOL_IMPLEMENTS(protocol, ROOMLIST, room_serialize)) {
		name = purple_protocol_roomlist_room_serialize(PURPLE_PROTOCOL_ROOMLIST(protocol), room);
	} else {
		name = g_strdup(purple_roomlist_room_get_name(room));
	}

	purple_blist_request_add_chat(froomlist.account, NULL, NULL, name);

	g_free(name);
}

static void
fl_close(G_GNUC_UNUSED GntWidget *button, G_GNUC_UNUSED gpointer data)
{
	gnt_widget_destroy(froomlist.window);
}

static void
roomlist_activated(GntWidget *widget)
{
	PurpleRoomlistRoom *room = gnt_tree_get_selection_data(GNT_TREE(widget));
	if (!room)
		return;

	purple_roomlist_join_room(froomlist.roomlist, room);
}

static void
roomlist_selection_changed(G_GNUC_UNUSED GntWidget *widget,
                           G_GNUC_UNUSED gpointer old, gpointer current,
                           G_GNUC_UNUSED gpointer data)
{
	PurpleRoomlistRoom *room = current;
	GntTextView *tv = GNT_TEXT_VIEW(froomlist.details);

	gnt_text_view_clear(tv);

	if (!room)
		return;

	gnt_text_view_append_text_with_flags(tv,
	                                     purple_roomlist_room_get_name(room),
	                                     GNT_TEXT_FLAG_BOLD);
}

static void
roomlist_account_changed(G_GNUC_UNUSED GntWidget *widget,
                         G_GNUC_UNUSED gpointer old, gpointer current,
                         G_GNUC_UNUSED gpointer data)
{
	if (froomlist.account == current) {
		return;
	}

	froomlist.account = current;
	if (froomlist.roomlist) {
		if (purple_roomlist_get_in_progress(froomlist.roomlist))
			purple_roomlist_cancel_get_list(froomlist.roomlist);
		g_clear_object(&froomlist.roomlist);
	}

	gnt_tree_remove_all(GNT_TREE(froomlist.tree));
	gnt_widget_draw(froomlist.tree);
}

static void
reset_account_list(G_GNUC_UNUSED PurpleAccount *account)
{
	GList *list;
	GntComboBox *accounts = GNT_COMBO_BOX(froomlist.accounts);
	gnt_combo_box_remove_all(accounts);
	for (list = purple_connections_get_all(); list; list = list->next) {
		PurpleProtocol *protocol = NULL;
		PurpleConnection *gc = list->data;

		protocol = purple_connection_get_protocol(gc);
		if(PURPLE_CONNECTION_IS_CONNECTED(gc) &&
		   PURPLE_PROTOCOL_IMPLEMENTS(protocol, ROOMLIST, get_list))
		{
			PurpleAccount *account = purple_connection_get_account(gc);
			PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);
			char *text = NULL;

			text = g_strdup_printf("%s (%s)",
			                       purple_contact_info_get_username(info),
			                       purple_account_get_protocol_name(account));
			gnt_combo_box_add_data(accounts, account, text);
			g_free(text);
		}
	}
}

static void
size_changed_cb(GntWidget *widget, G_GNUC_UNUSED int oldw,
                G_GNUC_UNUSED int oldh)
{
	int w, h;
	gnt_widget_get_size(widget, &w, &h);
	purple_prefs_set_int(PREF_ROOT "/size/width", w);
	purple_prefs_set_int(PREF_ROOT "/size/height", h);
}

static void
setup_roomlist(PurpleAccount *account)
{
	GntWidget *window, *tree, *hbox, *accounts;
	int iter;
	struct {
		const char *label;
		GCallback callback;
		GntWidget **widget;
	} buttons[] = {
		{_("Stop"), G_CALLBACK(fl_stop), &froomlist.stop},
		{_("Get"), G_CALLBACK(fl_get_list), &froomlist.getlist},
		{_("Add"), G_CALLBACK(fl_add_chat), &froomlist.add},
		{_("Close"), G_CALLBACK(fl_close), &froomlist.close},
		{NULL, NULL, NULL}
	};

	if (froomlist.window)
		return;

	froomlist.window = window = gnt_window_new();
	g_object_set(G_OBJECT(window), "vertical", TRUE, NULL);
	gnt_box_set_pad(GNT_BOX(window), 0);
	gnt_box_set_title(GNT_BOX(window), _("Room List"));
	gnt_box_set_alignment(GNT_BOX(window), GNT_ALIGN_MID);

	froomlist.accounts = accounts = gnt_combo_box_new();
	reset_account_list(account);
	gnt_box_add_widget(GNT_BOX(window), accounts);
	g_signal_connect(G_OBJECT(accounts), "selection-changed",
			G_CALLBACK(roomlist_account_changed), NULL);
	froomlist.account = gnt_combo_box_get_selected_data(GNT_COMBO_BOX(accounts));

	froomlist.tree = tree = gnt_tree_new_with_columns(2);
	gnt_tree_set_show_title(GNT_TREE(tree), TRUE);
	g_signal_connect(G_OBJECT(tree), "activate", G_CALLBACK(roomlist_activated), NULL);
	gnt_tree_set_column_titles(GNT_TREE(tree), _("Name"), "");
	gnt_tree_set_show_separator(GNT_TREE(tree), FALSE);
	gnt_tree_set_col_width(GNT_TREE(tree), 1, 1);
	gnt_tree_set_column_resizable(GNT_TREE(tree), 1, FALSE);
	gnt_tree_set_search_column(GNT_TREE(tree), 0);

	gnt_box_add_widget(GNT_BOX(window), tree);

	froomlist.details = gnt_text_view_new();
	gnt_text_view_set_flag(GNT_TEXT_VIEW(froomlist.details), GNT_TEXT_VIEW_TOP_ALIGN);
	gnt_box_add_widget(GNT_BOX(window), froomlist.details);
	gnt_widget_set_size(froomlist.details, -1, 8);

	hbox = gnt_hbox_new(FALSE);
	gnt_box_add_widget(GNT_BOX(window), hbox);

	for (iter = 0; buttons[iter].label; iter++) {
		GntWidget *button = gnt_button_new(buttons[iter].label);
		gnt_box_add_widget(GNT_BOX(hbox), button);
		g_signal_connect(G_OBJECT(button), "activate", buttons[iter].callback, NULL);
		*buttons[iter].widget = button;
		gnt_text_view_attach_scroll_widget(GNT_TEXT_VIEW(froomlist.details), button);
	}

	g_signal_connect(G_OBJECT(tree), "selection-changed", G_CALLBACK(roomlist_selection_changed), NULL);

	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(unset_roomlist), NULL);
}

static void
fl_show_with_account(PurpleAccount *account)
{
	setup_roomlist(account);
	g_signal_handlers_disconnect_matched(G_OBJECT(froomlist.window), G_SIGNAL_MATCH_FUNC,
			0, 0, NULL, G_CALLBACK(size_changed_cb), NULL);
	gnt_widget_show(froomlist.window);
	gnt_screen_resize_widget(froomlist.window,
			purple_prefs_get_int(PREF_ROOT "/size/width"),
			purple_prefs_get_int(PREF_ROOT "/size/height"));
	g_signal_connect(G_OBJECT(froomlist.window), "size_changed", G_CALLBACK(size_changed_cb), NULL);
	gnt_window_present(froomlist.window);
}

static void
fl_destroy(G_GNUC_UNUSED gpointer data, GObject *list)
{
	if (!froomlist.window) {
		return;
	}

	if (G_OBJECT(froomlist.roomlist) == list) {
		froomlist.roomlist = NULL;
		gnt_tree_remove_all(GNT_TREE(froomlist.tree));
		gnt_widget_draw(froomlist.tree);
	}
}

static void
fl_create(PurpleRoomlist *list)
{
	g_object_set_data(G_OBJECT(list), "finch-ui", &froomlist);
	g_object_weak_ref(G_OBJECT(list), (GWeakNotify)fl_destroy, NULL);
	setup_roomlist(NULL);
	g_set_object(&froomlist.roomlist, list);
}

static void
fl_set_fields(G_GNUC_UNUSED PurpleRoomlist *list, G_GNUC_UNUSED GList *fields)
{
}

static void
fl_add_room(PurpleRoomlist *roomlist, PurpleRoomlistRoom *room)
{
	gchar *category = NULL;
	if (froomlist.roomlist != roomlist)
		return;

	gnt_tree_remove(GNT_TREE(froomlist.tree), room);
	gnt_tree_add_row_after(GNT_TREE(froomlist.tree), room,
			gnt_tree_create_row(GNT_TREE(froomlist.tree),
				purple_roomlist_room_get_name(room), ""),
		NULL, NULL);
	gnt_tree_set_expanded(GNT_TREE(froomlist.tree), room, category == NULL);
}

static PurpleRoomlistUiOps ui_ops = {
	.show_with_account = fl_show_with_account,
	.create = fl_create,
	.set_fields = fl_set_fields,
	.add_room = fl_add_room,
};

PurpleRoomlistUiOps *finch_roomlist_get_ui_ops(void)
{
	return &ui_ops;
}

void finch_roomlist_show_all(void)
{
	purple_roomlist_show_with_account(NULL);
}

void finch_roomlist_init(void)
{
	purple_prefs_add_none(PREF_ROOT);
	purple_prefs_add_none(PREF_ROOT "/size");
	purple_prefs_add_int(PREF_ROOT "/size/width", 60);
	purple_prefs_add_int(PREF_ROOT "/size/height", 15);
}

void finch_roomlist_uninit(void)
{
}

