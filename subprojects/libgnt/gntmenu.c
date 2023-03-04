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
#include "gntmenu.h"

#include "gntmenuitemcheck.h"
#include "gntmenuitemprivate.h"
#include "gntncurses.h"
#include "gntwidgetprivate.h"

#include <ctype.h>
#include <string.h>

struct _GntMenu
{
	GntTree parent;
	GntMenuType type;

	GList *list;
	guint selected;

	/* This will keep track of its immediate submenu which is visible so
	 * that keystrokes can be passed to it. */
	GntMenu *submenu;
	GntMenu *parentmenu;
};

enum
{
	SIGS = 1,
};

enum
{
	ITEM_TEXT = 0,
	ITEM_TRIGGER,
	ITEM_SUBMENU,
	NUM_COLUMNS
};

static void menuitem_activate(GntMenu *menu, GntMenuItem *item);

G_DEFINE_TYPE(GntMenu, gnt_menu, GNT_TYPE_TREE)

static void
menu_hide_all(GntMenu *menu)
{
	while (menu->parentmenu)
		menu = menu->parentmenu;
	gnt_widget_hide(GNT_WIDGET(menu));
}

static void
show_submenu(GntMenu *menu)
{
	GntMenuItem *item;

	if (menu->type != GNT_MENU_TOPLEVEL)
			return;

	item = g_list_nth_data(menu->list, menu->selected);
	if (!item || !gnt_menuitem_get_submenu(item)) {
		return;
	}
	menuitem_activate(menu, item);
}

static void
gnt_menu_draw(GntWidget *widget)
{
	GntMenu *menu = GNT_MENU(widget);
	GList *iter;
	chtype type;
	guint i;

	if (menu->type == GNT_MENU_TOPLEVEL) {
		WINDOW *window = gnt_widget_get_window(widget);
		gint x, y;

		gnt_widget_get_position(widget, &x, &y);

		wbkgdset(window, '\0' | gnt_color_pair(GNT_COLOR_HIGHLIGHT));
		werase(window);

		for (i = 0, iter = menu->list; iter; iter = iter->next, i++) {
			GntMenuItem *item = GNT_MENU_ITEM(iter->data);
			if (!gnt_menuitem_is_visible(item))
				continue;
			type = ' ';
			if (gnt_menuitem_has_callback(item) ||
			    gnt_menuitem_get_submenu(item)) {
				type |= gnt_color_pair(GNT_COLOR_HIGHLIGHT);
			} else {
				type |= gnt_color_pair(GNT_COLOR_DISABLED);
			}
			if (i == menu->selected)
				type |= A_REVERSE;
			gnt_menuitem_set_position(item, getcurx(window) + x,
			                          getcury(window) + y + 1);
			wbkgdset(window, type);
			wprintw(window, " %s   ",
			        C_(gnt_menuitem_get_text(item)));
		}
	} else {
		GNT_WIDGET_CLASS(gnt_menu_parent_class)->draw(widget);
	}
}

static void
gnt_menu_size_request(GntWidget *widget)
{
	GntMenu *menu = GNT_MENU(widget);

	if (menu->type == GNT_MENU_TOPLEVEL) {
		gnt_widget_set_internal_size(widget, getmaxx(stdscr), 1);
	} else {
		gint width;
		GNT_WIDGET_CLASS(gnt_menu_parent_class)->size_request(widget);
		gnt_widget_get_internal_size(widget, &width, NULL);
		gnt_widget_set_internal_size(widget, width,
		                             g_list_length(menu->list) + 2);
	}
}

static void
menu_tree_add(GntMenu *menu, GntMenuItem *item, GntMenuItem *parent)
{
	char trigger[] = "\0 )\0";

	g_return_if_fail(item != NULL);

	if ((trigger[1] = gnt_menuitem_get_trigger(item)) && trigger[1] != ' ')
		trigger[0] = '(';

	if (GNT_IS_MENU_ITEM_CHECK(item)) {
		gnt_tree_add_choice(
		        GNT_TREE(menu), item,
		        gnt_tree_create_row(GNT_TREE(menu),
		                            gnt_menuitem_get_text(item),
		                            trigger, " "),
		        parent, NULL);
		gnt_tree_set_choice(GNT_TREE(menu), item, gnt_menuitem_check_get_checked(GNT_MENU_ITEM_CHECK(item)));
	} else {
		gnt_tree_add_row_last(
		        GNT_TREE(menu), item,
		        gnt_tree_create_row(
		                GNT_TREE(menu), gnt_menuitem_get_text(item),
		                trigger,
		                gnt_menuitem_get_submenu(item) ? ">" : " "),
		        parent);
		if (!gnt_menuitem_has_callback(item) &&
		    !gnt_menuitem_get_submenu(item)) {
			gnt_tree_set_row_color(GNT_TREE(menu), item, GNT_COLOR_DISABLED);
		}
	}

	if (0 && gnt_menuitem_get_submenu(item)) {
		GntMenu *sub = GNT_MENU(gnt_menuitem_get_submenu(item));
		GList *iter;
		for (iter = sub->list; iter; iter = iter->next) {
			GntMenuItem *it = GNT_MENU_ITEM(iter->data);
			menu_tree_add(menu, it, item);
		}
	}
}

#define GET_VAL(ch)  ((ch >= '0' && ch <= '9') ? (ch - '0') : (ch >= 'a' && ch <= 'z') ? (10 + ch - 'a') : 36)

static void
assign_triggers(GntMenu *menu)
{
	GList *iter;
	gboolean bools[37];

	memset(bools, 0, sizeof(bools));
	bools[36] = 1;

	for (iter = menu->list; iter; iter = iter->next) {
		GntMenuItem *item = iter->data;
		char trigger = tolower(gnt_menuitem_get_trigger(item));
		if (trigger == '\0' || trigger == ' ')
			continue;
		bools[(int)GET_VAL(trigger)] = 1;
	}

	for (iter = menu->list; iter; iter = iter->next) {
		GntMenuItem *item = iter->data;
		char trigger = gnt_menuitem_get_trigger(item);
		const gchar *text = gnt_menuitem_get_text(item);
		if (trigger != '\0')
			continue;
		while (*text) {
			char ch = tolower(*text++);
			char t[2] = {ch, '\0'};
			if (ch == ' ' || bools[(int)GET_VAL(ch)] || gnt_bindable_check_key(GNT_BINDABLE(menu), t))
				continue;
			trigger = ch;
			break;
		}
		if (trigger == 0)
			trigger = gnt_menuitem_get_text(item)[0];
		gnt_menuitem_set_trigger(item, trigger);
		bools[(int)GET_VAL(trigger)] = 1;
	}
}

static void
gnt_menu_map(GntWidget *widget)
{
	GntMenu *menu = GNT_MENU(widget);

	if (menu->type == GNT_MENU_TOPLEVEL) {
		gnt_widget_size_request(widget);
	} else {
		/* Populate the tree */
		GList *iter;
		gnt_tree_remove_all(GNT_TREE(widget));
		/* Try to assign some trigger for the items */
		assign_triggers(menu);
		for (iter = menu->list; iter; iter = iter->next) {
			GntMenuItem *item = GNT_MENU_ITEM(iter->data);
			menu_tree_add(menu, item, NULL);
		}
		GNT_WIDGET_CLASS(gnt_menu_parent_class)->map(widget);
		gnt_tree_adjust_columns(GNT_TREE(widget));
	}
}

static void
menuitem_activate(GntMenu *menu, GntMenuItem *item)
{
	if (!item)
		return;

	if (gnt_menuitem_activate(item)) {
		menu_hide_all(menu);
	} else {
		GntMenu *sub = GNT_MENU(gnt_menuitem_get_submenu(item));
		if (sub) {
			gint x, y;
			menu->submenu = sub;
			sub->type = GNT_MENU_POPUP;	/* Submenus are *never* toplevel */
			sub->parentmenu = menu;
			if (menu->type != GNT_MENU_TOPLEVEL) {
				GntWidget *widget = GNT_WIDGET(menu);
				gint width;
				gnt_widget_get_position(widget, &x, &y);
				gnt_widget_get_internal_size(widget, &width,
				                             NULL);
				gnt_menuitem_set_position(
				        item, x + width - 1,
				        y + gnt_tree_get_selection_visible_line(
				                    GNT_TREE(menu)));
			}
			gnt_menuitem_get_position(item, &x, &y);
			gnt_widget_set_position(GNT_WIDGET(sub), x, y);
			gnt_widget_set_visible(GNT_WIDGET(sub), TRUE);
			gnt_widget_draw(GNT_WIDGET(sub));
		} else {
			menu_hide_all(menu);
		}
	}
}

static GList*
find_item_with_trigger(GList *start, GList *end, char trigger)
{
	GList *iter;
	for (iter = start; iter != (end ? end : NULL); iter = iter->next) {
		if (gnt_menuitem_get_trigger(iter->data) == trigger)
			return iter;
	}
	return NULL;
}

static gboolean
check_for_trigger(GntMenu *menu, char trigger)
{
	/* check for a trigger key */
	GList *iter;
	GList *find;
	GList *nth = g_list_find(menu->list, gnt_tree_get_selection_data(GNT_TREE(menu)));

	if (nth == NULL)
		return FALSE;

	find = find_item_with_trigger(nth->next, NULL, trigger);
	if (!find)
		find = find_item_with_trigger(menu->list, nth->next, trigger);
	if (!find)
		return FALSE;
	if (find != nth) {
		gnt_tree_set_selected(GNT_TREE(menu), find->data);
		iter = find_item_with_trigger(find->next, NULL, trigger);
		if (iter != NULL && iter != find)
			return TRUE;
		iter = find_item_with_trigger(menu->list, nth, trigger);
		if (iter != NULL && iter != find)
			return TRUE;
	}
	gnt_widget_activate(GNT_WIDGET(menu));
	return TRUE;
}

static gboolean
gnt_menu_key_pressed(GntWidget *widget, const char *text)
{
	GntMenu *menu = GNT_MENU(widget);
	guint current = menu->selected;

	if (menu->submenu) {
		GntMenu *sub = menu;
		do sub = sub->submenu; while (sub->submenu);
		if (gnt_widget_key_pressed(GNT_WIDGET(sub), text))
			return TRUE;
		if (menu->type != GNT_MENU_TOPLEVEL)
			return FALSE;
	}

	if ((text[0] == 27 && text[1] == 0) ||
			(menu->type != GNT_MENU_TOPLEVEL && strcmp(text, GNT_KEY_LEFT) == 0)) {
		/* Escape closes menu */
		GntMenu *par = menu->parentmenu;
		if (par != NULL) {
			par->submenu = NULL;
			gnt_widget_hide(widget);
		} else
			gnt_widget_hide(widget);
		if (par && par->type == GNT_MENU_TOPLEVEL)
			gnt_menu_key_pressed(GNT_WIDGET(par), text);
		return TRUE;
	}

	if (menu->type == GNT_MENU_TOPLEVEL) {
		GntMenuItem *item;
		GList *it;
		if (strcmp(text, GNT_KEY_LEFT) == 0) {
			do {
				if (menu->selected == 0)
					menu->selected = g_list_length(menu->list) - 1;
				else
					menu->selected--;
				it = g_list_nth(menu->list, menu->selected);
				item = it ? it->data : NULL;
			} while (!gnt_menuitem_is_visible(item));
		} else if (strcmp(text, GNT_KEY_RIGHT) == 0) {
			do {
				menu->selected++;
				if (menu->selected >= g_list_length(menu->list))
					menu->selected = 0;
				it = g_list_nth(menu->list, menu->selected);
				item = it ? it->data : NULL;
			} while (!gnt_menuitem_is_visible(item));
		} else if (strcmp(text, GNT_KEY_ENTER) == 0 ||
				strcmp(text, GNT_KEY_DOWN) == 0) {
			gnt_widget_activate(widget);
		}

		if (current != menu->selected) {
			GntMenu *sub = menu->submenu;
			if (sub)
				gnt_widget_hide(GNT_WIDGET(sub));
			show_submenu(menu);
			gnt_widget_draw(widget);
			return TRUE;
		}
	} else {
		if (text[1] == '\0') {
			if (check_for_trigger(menu, text[0]))
				return TRUE;
		} else if (strcmp(text, GNT_KEY_RIGHT) == 0) {
			GntMenuItem *item = gnt_tree_get_selection_data(GNT_TREE(menu));
			if (item && gnt_menuitem_get_submenu(item)) {
				menuitem_activate(menu, item);
				return TRUE;
			}
		}
		if (gnt_bindable_perform_action_key(GNT_BINDABLE(widget), text))
			return TRUE;
		return GNT_WIDGET_CLASS(gnt_menu_parent_class)
		        ->key_pressed(widget, text);
	}

	return gnt_bindable_perform_action_key(GNT_BINDABLE(widget), text);
}

static void
gnt_menu_destroy(GntWidget *widget)
{
	GntMenu *menu = GNT_MENU(widget);
	g_list_free_full(menu->list, g_object_unref);
	menu->list = NULL;
	GNT_WIDGET_CLASS(gnt_menu_parent_class)->destroy(widget);
}

static void
gnt_menu_toggled(GntTree *tree, gpointer key)
{
	GntMenuItem *item = GNT_MENU_ITEM(key);
	GntMenu *menu = GNT_MENU(tree);
	gboolean check = gnt_menuitem_check_get_checked(GNT_MENU_ITEM_CHECK(item));
	gnt_menuitem_check_set_checked(GNT_MENU_ITEM_CHECK(item), !check);
	gnt_menuitem_activate(item);
	while (menu) {
		gnt_widget_hide(GNT_WIDGET(menu));
		menu = menu->parentmenu;
	}
}

static void
gnt_menu_activate(GntWidget *widget)
{
	GntMenu *menu = GNT_MENU(widget);
	GntMenuItem *item;

	if (menu->type == GNT_MENU_TOPLEVEL) {
		item = g_list_nth_data(menu->list, menu->selected);
	} else {
		item = gnt_tree_get_selection_data(GNT_TREE(menu));
	}

	if (item) {
		if (GNT_IS_MENU_ITEM_CHECK(item))
			gnt_menu_toggled(GNT_TREE(widget), item);
		else
			menuitem_activate(menu, item);
	}
}

static void
gnt_menu_hide(GntWidget *widget)
{
	GntMenu *sub, *menu = GNT_MENU(widget);

	while ((sub = menu->submenu))
		gnt_widget_hide(GNT_WIDGET(sub));
	if (menu->parentmenu)
		menu->parentmenu->submenu = NULL;
}

static gboolean
gnt_menu_clicked(GntWidget *widget, GntMouseEvent event, int x, int y)
{
	gboolean (*clicked)(GntWidget * w, GntMouseEvent event, int x, int y);

	if (GNT_MENU(widget)->type != GNT_MENU_POPUP) {
		return FALSE;
	}

	clicked = GNT_WIDGET_CLASS(gnt_menu_parent_class)->clicked;
	if (clicked && clicked(widget, event, x, y)) {
		return TRUE;
	}

	gnt_widget_activate(widget);
	return TRUE;
}

static void
gnt_menu_class_init(GntMenuClass *klass)
{
	GntWidgetClass *widget_class = GNT_WIDGET_CLASS(klass);
	GntTreeClass *tree_class = GNT_TREE_CLASS(klass);

	widget_class->destroy = gnt_menu_destroy;
	widget_class->draw = gnt_menu_draw;
	widget_class->map = gnt_menu_map;
	widget_class->size_request = gnt_menu_size_request;
	widget_class->key_pressed = gnt_menu_key_pressed;
	widget_class->activate = gnt_menu_activate;
	widget_class->hide = gnt_menu_hide;
	widget_class->clicked = gnt_menu_clicked;

	tree_class->toggled = gnt_menu_toggled;
}

static void
gnt_menu_init(GntMenu *menu)
{
	GntWidget *widget = GNT_WIDGET(menu);

	gnt_widget_set_has_shadow(widget, FALSE);
	gnt_widget_set_has_border(widget, FALSE);
	gnt_widget_set_take_focus(widget, TRUE);
	gnt_widget_set_transient(widget, TRUE);
	gnt_widget_set_disable_actions(widget, TRUE);
}

/******************************************************************************
 * GntMenu API
 *****************************************************************************/
GntWidget *gnt_menu_new(GntMenuType type)
{
	GntWidget *widget = g_object_new(GNT_TYPE_MENU, NULL);
	GntMenu *menu = GNT_MENU(widget);
	menu->list = NULL;
	menu->selected = 0;
	menu->type = type;

	if (type == GNT_MENU_TOPLEVEL) {
		gnt_widget_set_position(widget, 0, 0);
	} else {
		gnt_tree_set_show_separator(GNT_TREE(widget), FALSE);
		g_object_set(G_OBJECT(widget), "columns", NUM_COLUMNS, NULL);
		gnt_tree_set_col_width(GNT_TREE(widget), ITEM_TRIGGER, 3);
		gnt_tree_set_column_resizable(GNT_TREE(widget), ITEM_TRIGGER, FALSE);
		gnt_tree_set_col_width(GNT_TREE(widget), ITEM_SUBMENU, 1);
		gnt_tree_set_column_resizable(GNT_TREE(widget), ITEM_SUBMENU, FALSE);
		gnt_widget_set_has_border(widget, TRUE);
	}

	return widget;
}

void gnt_menu_add_item(GntMenu *menu, GntMenuItem *item)
{
	menu->list = g_list_append(menu->list, item);
}

GntMenuItem *gnt_menu_get_item(GntMenu *menu, const char *id)
{
	GntMenuItem *item = NULL;
	GList *iter = menu->list;

	if (!id || !*id)
		return NULL;

	for (; iter; iter = iter->next) {
		GntMenu *sub;
		item = iter->data;
		sub = gnt_menuitem_get_submenu(item);
		if (sub) {
			item = gnt_menu_get_item(sub, id);
			if (item)
				break;
		} else {
			const char *itid = gnt_menuitem_get_id(item);
			if (itid && strcmp(itid, id) == 0)
				break;
			/* XXX: Perhaps look at the menu-label as well? */
		}
		item = NULL;
	}

	return item;
}

/* Internal. */
GntMenuType
gnt_menu_get_menutype(GntMenu *menu)
{
	g_return_val_if_fail(GNT_IS_MENU(menu), 0);
	return menu->type;
}

/* Internal. */
GntMenuItem *
gnt_menu_get_selected_item(GntMenu *menu)
{
	g_return_val_if_fail(GNT_IS_MENU(menu), NULL);
	return g_list_nth_data(menu->list, menu->selected);
}

/* Internal. */
GntMenu *
gnt_menu_get_parent_menu(GntMenu *menu)
{
	g_return_val_if_fail(GNT_IS_MENU(menu), NULL);
	return menu->parentmenu;
}

/* Internal. */
GntMenu *
gnt_menu_get_submenu(GntMenu *menu)
{
	g_return_val_if_fail(GNT_IS_MENU(menu), NULL);
	return menu->submenu;
}
