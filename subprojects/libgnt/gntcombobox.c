/*
 * GNT - The GLib Ncurses Toolkit
 *
 * GNT is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include "gntinternal.h"
#include "gntbox.h"
#include "gntcombobox.h"
#include "gnttree.h"
#include "gnttreeprivate.h"
#include "gntstyle.h"
#include "gntutils.h"

#include "gntwidgetprivate.h"

#include <string.h>

struct _GntComboBox
{
	GntWidget parent;

	GntWidget *dropdown; /* This is a GntTree */
	gpointer selected;   /* Currently selected key */
};

enum
{
	SIG_SELECTION_CHANGED,
	SIGS,
};

static guint signals[SIGS] = { 0 };
static void (*widget_lost_focus)(GntWidget *widget);

G_DEFINE_TYPE(GntComboBox, gnt_combo_box, GNT_TYPE_WIDGET)

static void
set_selection(GntComboBox *box, gpointer key)
{
	if (box->selected != key)
	{
		/* XXX: make sure the key actually does exist */
		gpointer old = box->selected;
		box->selected = key;
		if (gnt_widget_get_window(GNT_WIDGET(box))) {
			gnt_widget_draw(GNT_WIDGET(box));
		}
		if (box->dropdown)
			gnt_tree_set_selected(GNT_TREE(box->dropdown), key);
		g_signal_emit(box, signals[SIG_SELECTION_CHANGED], 0, old, key);
	}
}

static void
hide_popup(GntComboBox *box, gboolean set)
{
	gint width, height;
	gnt_widget_get_internal_size(box->dropdown, &width, &height);
	gnt_widget_set_size(box->dropdown, width - 1, height);
	if (set)
		set_selection(box, gnt_tree_get_selection_data(GNT_TREE(box->dropdown)));
	else
		gnt_tree_set_selected(GNT_TREE(box->dropdown), box->selected);
	gnt_widget_hide(gnt_widget_get_parent(box->dropdown));
}

static void
gnt_combo_box_draw(GntWidget *widget)
{
	GntComboBox *box = GNT_COMBO_BOX(widget);
	WINDOW *window = gnt_widget_get_window(widget);
	char *text = NULL, *s;
	GntColorType type;
	gint width;
	int len;

	if (box->dropdown && box->selected)
		text = gnt_tree_get_selection_text(GNT_TREE(box->dropdown));

	if (text == NULL)
		text = g_strdup("");

	if (gnt_widget_has_focus(widget))
		type = GNT_COLOR_HIGHLIGHT;
	else
		type = GNT_COLOR_NORMAL;

	wbkgdset(window, '\0' | gnt_color_pair(type));

	gnt_widget_get_internal_size(widget, &width, NULL);
	s = (char *)gnt_util_onscreen_width_to_pointer(text, width - 4, &len);
	*s = '\0';

	mvwaddstr(window, 1, 1, C_(text));
	whline(window, ' ' | gnt_color_pair(type), width - 4 - len);
	mvwaddch(window, 1, width - 3,
	         ACS_VLINE | gnt_color_pair(GNT_COLOR_NORMAL));
	mvwaddch(window, 1, width - 2,
	         ACS_DARROW | gnt_color_pair(GNT_COLOR_NORMAL));
	(void)wmove(window, 1, 1);

	g_free(text);
}

static void
gnt_combo_box_size_request(GntWidget *widget)
{
	if (!gnt_widget_get_mapped(widget)) {
		GntWidget *dd = GNT_COMBO_BOX(widget)->dropdown;
		gint width;
		gnt_widget_size_request(dd);
		gnt_widget_get_internal_size(dd, &width, NULL);
		/* For now, a combobox will have border */
		gnt_widget_set_internal_size(widget, MAX(10, width + 2), 3);
	}
}

static void
gnt_combo_box_map(GntWidget *widget)
{
	gint width, height;

	gnt_widget_get_internal_size(widget, &width, &height);
	if (width == 0 || height == 0) {
		gnt_widget_size_request(widget);
	}
}

static void
popup_dropdown(GntComboBox *box)
{
	GntWidget *widget = GNT_WIDGET(box);
	GntWidget *parent = gnt_widget_get_parent(box->dropdown);
	WINDOW *window;
	gint widgetx, widgety, widgetwidth, widgetheight;
	gint height;
	gint y;

	gnt_widget_get_position(widget, &widgetx, &widgety);
	gnt_widget_get_internal_size(widget, &widgetwidth, &widgetheight);
	height = g_list_length(gnt_tree_get_rows(GNT_TREE(box->dropdown)));
	y = widgety + widgetheight - 1;
	gnt_widget_set_size(box->dropdown, widgetwidth, height + 2);

	if (y + height + 2 >= getmaxy(stdscr))
		y = widgety - height - 1;
	gnt_widget_set_position(parent, widgetx, y);
	window = gnt_widget_get_window(parent);
	if (window) {
		mvwin(window, y, widgetx);
		wresize(window, height + 2, widgetwidth);
	}
	gnt_widget_set_internal_size(parent, widgetwidth, height + 2);

	gnt_widget_set_visible(parent, TRUE);
	gnt_widget_draw(parent);
}

static gboolean
gnt_combo_box_key_pressed(GntWidget *widget, const char *text)
{
	GntComboBox *box = GNT_COMBO_BOX(widget);
	gboolean showing;

	showing = gnt_widget_get_mapped(gnt_widget_get_parent(box->dropdown));

	if (showing) {
		if (text[1] == 0) {
			switch (text[0]) {
				case '\r':
				case '\t':
				case '\n':
					hide_popup(box, TRUE);
					return TRUE;
				case 27:
					hide_popup(box, FALSE);
					return TRUE;
			}
		}
	}

	if (gnt_widget_key_pressed(box->dropdown, text)) {
		if (!showing)
			popup_dropdown(box);
		return TRUE;
	}

	{
#define SEARCH_IN_RANGE(start, end) do { \
		GntTreeRow *row; \
		for (row = start; row != end; \
				row = gnt_tree_row_get_next(tree, row)) { \
			gpointer key = gnt_tree_row_get_key(tree, row); \
			GList *list = gnt_tree_get_row_text_list(tree, key); \
			gboolean found = FALSE; \
			found = (list->data && g_ascii_strncasecmp(text, list->data, len) == 0); \
			g_list_free_full(list, g_free); \
			if (found) { \
				if (!showing) \
					popup_dropdown(box); \
				gnt_tree_set_selected(tree, key); \
				return TRUE; \
			} \
		} \
} while (0)

		int len = strlen(text);
		GntTree *tree = GNT_TREE(box->dropdown);
		GntTreeRow *current = gnt_tree_get_current(tree);

		SEARCH_IN_RANGE(gnt_tree_row_get_next(tree, current), NULL);
		SEARCH_IN_RANGE(gnt_tree_get_top(tree), current);

#undef SEARCH_IN_RANGE
	}

	return FALSE;
}

static void
gnt_combo_box_destroy(GntWidget *widget)
{
	GntComboBox *combo = GNT_COMBO_BOX(widget);
	gnt_widget_destroy(gnt_widget_get_parent(combo->dropdown));
}

static void
gnt_combo_box_lost_focus(GntWidget *widget)
{
	GntComboBox *combo = GNT_COMBO_BOX(widget);
	if (gnt_widget_get_mapped(gnt_widget_get_parent(combo->dropdown))) {
		hide_popup(combo, FALSE);
	}
	widget_lost_focus(widget);
}

static gboolean
gnt_combo_box_clicked(GntWidget *widget, GntMouseEvent event,
                      G_GNUC_UNUSED int x, G_GNUC_UNUSED int y)
{
	GntComboBox *box = GNT_COMBO_BOX(widget);
	gboolean dshowing;

	dshowing = gnt_widget_get_mapped(gnt_widget_get_parent(box->dropdown));

	if (event == GNT_MOUSE_SCROLL_UP) {
		if (dshowing)
			gnt_widget_key_pressed(box->dropdown, GNT_KEY_UP);
	} else if (event == GNT_MOUSE_SCROLL_DOWN) {
		if (dshowing)
			gnt_widget_key_pressed(box->dropdown, GNT_KEY_DOWN);
	} else if (event == GNT_LEFT_MOUSE_DOWN) {
		if (dshowing) {
			hide_popup(box, TRUE);
		} else {
			popup_dropdown(GNT_COMBO_BOX(widget));
		}
	} else
		return FALSE;
	return TRUE;
}

static void
gnt_combo_box_size_changed(GntWidget *widget, G_GNUC_UNUSED int oldw,
                           G_GNUC_UNUSED int oldh)
{
	GntComboBox *box = GNT_COMBO_BOX(widget);
	gint width, height;

	gnt_widget_get_internal_size(widget, &width, NULL);
	gnt_widget_get_internal_size(box->dropdown, NULL, &height);
	gnt_widget_set_size(box->dropdown, width - 1, height);
}

static gboolean
dropdown_menu(GntBindable *b, G_GNUC_UNUSED GList *params)
{
	GntComboBox *combo = GNT_COMBO_BOX(b);
	if (gnt_widget_get_mapped(gnt_widget_get_parent(combo->dropdown))) {
		return FALSE;
	}
	popup_dropdown(combo);
	return TRUE;
}

static void
gnt_combo_box_class_init(GntComboBoxClass *klass)
{
	GntBindableClass *bindable = GNT_BINDABLE_CLASS(klass);
	GntWidgetClass *widget_class = GNT_WIDGET_CLASS(klass);

	widget_class->destroy = gnt_combo_box_destroy;
	widget_class->draw = gnt_combo_box_draw;
	widget_class->map = gnt_combo_box_map;
	widget_class->size_request = gnt_combo_box_size_request;
	widget_class->key_pressed = gnt_combo_box_key_pressed;
	widget_class->clicked = gnt_combo_box_clicked;
	widget_class->size_changed = gnt_combo_box_size_changed;

	widget_lost_focus = widget_class->lost_focus;
	widget_class->lost_focus = gnt_combo_box_lost_focus;

	signals[SIG_SELECTION_CHANGED] =
		g_signal_new("selection-changed",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 0,
					 NULL, NULL, NULL,
					 G_TYPE_NONE, 2, G_TYPE_POINTER, G_TYPE_POINTER);

	gnt_bindable_class_register_action(bindable, "dropdown", dropdown_menu,
			GNT_KEY_DOWN, NULL);
	gnt_bindable_register_binding(bindable, "dropdown", GNT_KEY_UP, NULL);

	gnt_style_read_actions(G_OBJECT_CLASS_TYPE(klass), bindable);
}

static void
gnt_combo_box_init(GntComboBox *combo)
{
	GntWidget *widget = GNT_WIDGET(combo);
	GntWidget *box;

	gnt_widget_set_grow_x(widget, TRUE);
	gnt_widget_set_take_focus(widget, TRUE);
	gnt_widget_set_has_shadow(widget, FALSE);
	combo->dropdown = gnt_tree_new();

	box = gnt_box_new(FALSE, FALSE);
	gnt_widget_set_has_shadow(box, FALSE);
	gnt_widget_set_has_border(box, FALSE);
	gnt_widget_set_transient(box, TRUE);
	gnt_box_set_pad(GNT_BOX(box), 0);
	gnt_box_add_widget(GNT_BOX(box), combo->dropdown);

	gnt_widget_set_minimum_size(widget, 4, 3);
}

/******************************************************************************
 * GntComboBox API
 *****************************************************************************/
GntWidget *
gnt_combo_box_new(void)
{
	GntWidget *widget = g_object_new(GNT_TYPE_COMBO_BOX, NULL);

	return widget;
}

GntWidget *
gnt_combo_box_get_dropdown(GntComboBox *box)
{
	g_return_val_if_fail(GNT_IS_COMBO_BOX(box), NULL);

	return box->dropdown;
}

void gnt_combo_box_add_data(GntComboBox *box, gpointer key, const char *text)
{
	gnt_tree_add_row_last(GNT_TREE(box->dropdown), key,
			gnt_tree_create_row(GNT_TREE(box->dropdown), text), NULL);
	if (box->selected == NULL)
		set_selection(box, key);
}

gpointer gnt_combo_box_get_selected_data(GntComboBox *box)
{
	return box->selected;
}

void gnt_combo_box_set_selected(GntComboBox *box, gpointer key)
{
	set_selection(box, key);
}

void gnt_combo_box_remove(GntComboBox *box, gpointer key)
{
	gnt_tree_remove(GNT_TREE(box->dropdown), key);
	if (box->selected == key)
		set_selection(box, NULL);
}

void gnt_combo_box_remove_all(GntComboBox *box)
{
	gnt_tree_remove_all(GNT_TREE(box->dropdown));
	set_selection(box, NULL);
}
