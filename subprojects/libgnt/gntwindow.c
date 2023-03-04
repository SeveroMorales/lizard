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
#include "gntstyle.h"
#include "gntwindow.h"

#include "gntmenuprivate.h"

#include <string.h>

typedef struct
{
	GHashTable *accels;   /* key => menuitem-id */
	GntWindowFlags flags;
	GntMenu *menu;
} GntWindowPrivate;

enum
{
	SIG_WORKSPACE_HIDE,
	SIG_WORKSPACE_SHOW,
	SIGS,
};

static guint signals[SIGS] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE(GntWindow, gnt_window, GNT_TYPE_BOX)

static gboolean
show_menu(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntWindow *win = GNT_WINDOW(bind);
	GntWindowPrivate *priv = gnt_window_get_instance_private(win);

	if (priv->menu) {
		GntMenu *menu = priv->menu;

		gnt_screen_menu_show(menu);
		if (gnt_menu_get_menutype(menu) == GNT_MENU_TOPLEVEL) {
			GntMenuItem *item = gnt_menu_get_selected_item(menu);
			if (item && gnt_menuitem_get_submenu(item)) {
				gnt_widget_activate(GNT_WIDGET(menu));
			}
		}
		return TRUE;
	}
	return FALSE;
}

static void
gnt_window_destroy(GntWidget *widget)
{
	GntWindow *window = GNT_WINDOW(widget);
	GntWindowPrivate *priv = gnt_window_get_instance_private(window);

	g_clear_pointer((GntWidget **)&priv->menu, gnt_widget_destroy);
	g_clear_pointer(&priv->accels, g_hash_table_destroy);

	GNT_WIDGET_CLASS(gnt_window_parent_class)->destroy(widget);
}

static void
gnt_window_class_init(GntWindowClass *klass)
{
	GntBindableClass *bindable = GNT_BINDABLE_CLASS(klass);
	GntWidgetClass *widget_class = GNT_WIDGET_CLASS(klass);

	widget_class->destroy = gnt_window_destroy;

	signals[SIG_WORKSPACE_HIDE] =
		g_signal_new("workspace-hidden",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 0,
					 NULL, NULL, NULL,
					 G_TYPE_NONE, 0);

	signals[SIG_WORKSPACE_SHOW] =
		g_signal_new("workspace-shown",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 0,
					 NULL, NULL, NULL,
					 G_TYPE_NONE, 0);

	gnt_bindable_class_register_action(bindable, "show-menu", show_menu,
				GNT_KEY_CTRL_O, NULL);
	gnt_bindable_register_binding(bindable, "show-menu", GNT_KEY_F10, NULL);
	gnt_style_read_actions(G_OBJECT_CLASS_TYPE(klass), bindable);
}

static void
gnt_window_init(GntWindow *win)
{
	GntWidget *widget = GNT_WIDGET(win);
	GntWindowPrivate *priv = gnt_window_get_instance_private(win);

	gnt_widget_set_has_border(widget, TRUE);
	gnt_widget_set_has_shadow(widget, TRUE);
	gnt_widget_set_take_focus(widget, TRUE);

	priv->accels =
	        g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
}

/******************************************************************************
 * GntWindow API
 *****************************************************************************/
GntWidget *
gnt_window_new(void)
{
	GntWidget *widget = g_object_new(GNT_TYPE_WINDOW, NULL);

	return widget;
}

GntWidget *
gnt_window_box_new(gboolean homogeneous, gboolean vert)
{
	GntWidget *wid = g_object_new(GNT_TYPE_WINDOW, "homogeneous",
	                              homogeneous, "vertical", vert, NULL);
	GntBox *box = GNT_BOX(wid);

	gnt_box_set_alignment(box, vert ? GNT_ALIGN_LEFT : GNT_ALIGN_MID);

	return wid;
}

void
gnt_window_workspace_hiding(GntWindow *window)
{
	GntWindowPrivate *priv = gnt_window_get_instance_private(window);
	if (priv->menu)
		gnt_widget_hide(GNT_WIDGET(priv->menu));
	g_signal_emit(window, signals[SIG_WORKSPACE_HIDE], 0);
}

void
gnt_window_workspace_showing(GntWindow *window)
{
	g_signal_emit(window, signals[SIG_WORKSPACE_SHOW], 0);
}

void gnt_window_set_menu(GntWindow *window, GntMenu *menu)
{
	GntWindowPrivate *priv = gnt_window_get_instance_private(window);
	/* If a menu already existed, then destroy that first. */
	const char *name = gnt_widget_get_name(GNT_WIDGET(window));
	if (priv->menu)
		gnt_widget_destroy(GNT_WIDGET(priv->menu));
	priv->menu = menu;
	if (name) {
		if (!gnt_style_read_menu_accels(name, priv->accels)) {
			g_clear_pointer(&priv->accels, g_hash_table_destroy);
		}
	}
}

GntMenu *
gnt_window_get_menu(GntWindow *window)
{
	GntWindowPrivate *priv = gnt_window_get_instance_private(window);

	g_return_val_if_fail(GNT_IS_WINDOW(window), NULL);

	return priv->menu;
}

const char * gnt_window_get_accel_item(GntWindow *window, const char *key)
{
	GntWindowPrivate *priv = gnt_window_get_instance_private(window);
	if (priv->accels)
		return g_hash_table_lookup(priv->accels, key);
	return NULL;
}

void gnt_window_set_maximize(GntWindow *window, GntWindowFlags maximize)
{
	GntWindowPrivate *priv = gnt_window_get_instance_private(window);
	if (maximize & GNT_WINDOW_MAXIMIZE_X)
		priv->flags |= GNT_WINDOW_MAXIMIZE_X;
	else
		priv->flags &= ~GNT_WINDOW_MAXIMIZE_X;

	if (maximize & GNT_WINDOW_MAXIMIZE_Y)
		priv->flags |= GNT_WINDOW_MAXIMIZE_Y;
	else
		priv->flags &= ~GNT_WINDOW_MAXIMIZE_Y;
}

GntWindowFlags gnt_window_get_maximize(GntWindow *window)
{
	GntWindowPrivate *priv = gnt_window_get_instance_private(window);
	return (priv->flags & (GNT_WINDOW_MAXIMIZE_X | GNT_WINDOW_MAXIMIZE_Y));
}

