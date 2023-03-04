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
#include "gntmenuitem.h"

typedef struct
{
	/* These will be used to determine the position of the submenu */
	gint x;
	gint y;
	gchar trigger;
	gchar *id;

	gchar *text;

	/* A GntMenuItem can have a callback associated with it.
	 * The callback will be activated whenever the user selects it and
	 * presses enter (or clicks). However, if the GntMenuItem has some
	 * child, then the callback and callbackdata will be ignored. */
	gpointer callbackdata;
	GntMenuItemCallback callback;

	GntMenu *submenu;

	gboolean visible;
} GntMenuItemPrivate;

enum
{
	SIG_ACTIVATE,
	SIGS
};
static guint signals[SIGS] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE(GntMenuItem, gnt_menuitem, G_TYPE_OBJECT)

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/

static void
gnt_menuitem_dispose(GObject *obj)
{
	GntMenuItem *item = GNT_MENU_ITEM(obj);
	GntMenuItemPrivate *priv = gnt_menuitem_get_instance_private(item);

	g_clear_pointer(&priv->text, g_free);
	g_clear_pointer((GntWidget **)&priv->submenu, gnt_widget_destroy);
	g_clear_pointer(&priv->id, g_free);

	G_OBJECT_CLASS(gnt_menuitem_parent_class)->dispose(obj);
}

static void
gnt_menuitem_class_init(GntMenuItemClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->dispose = gnt_menuitem_dispose;

	signals[SIG_ACTIVATE] =
		g_signal_new("activate",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 0, NULL, NULL, NULL,
					 G_TYPE_NONE, 0);
}

static void
gnt_menuitem_init(GntMenuItem *item)
{
	GntMenuItemPrivate *priv = gnt_menuitem_get_instance_private(item);

	priv->visible = TRUE;
}

/******************************************************************************
 * GntMenuItem API
 *****************************************************************************/
GntMenuItem *
gnt_menuitem_new(const gchar *text)
{
	GObject *item = g_object_new(GNT_TYPE_MENU_ITEM, NULL);
	GntMenuItem *menuitem = GNT_MENU_ITEM(item);
	GntMenuItemPrivate *priv = gnt_menuitem_get_instance_private(menuitem);

	priv->text = g_strdup(text);

	return menuitem;
}

/* Internal. */
void
gnt_menuitem_set_position(GntMenuItem *item, gint x, gint y)
{
	GntMenuItemPrivate *priv = NULL;
	g_return_if_fail(GNT_IS_MENU_ITEM(item));
	priv = gnt_menuitem_get_instance_private(item);
	priv->x = x;
	priv->y = y;
}

/* Internal. */
void
gnt_menuitem_get_position(GntMenuItem *item, gint *x, gint *y)
{
	GntMenuItemPrivate *priv = NULL;
	g_return_if_fail(GNT_IS_MENU_ITEM(item));
	priv = gnt_menuitem_get_instance_private(item);

	if (x) {
		*x = priv->x;
	}
	if (y) {
		*y = priv->y;
	}
}

/* Internal. */
gboolean
gnt_menuitem_has_callback(GntMenuItem *item)
{
	GntMenuItemPrivate *priv = NULL;
	g_return_val_if_fail(GNT_IS_MENU_ITEM(item), FALSE);
	priv = gnt_menuitem_get_instance_private(item);
	return priv->callback != NULL;
}

void gnt_menuitem_set_callback(GntMenuItem *item, GntMenuItemCallback callback, gpointer data)
{
	GntMenuItemPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_MENU_ITEM(item));
	priv = gnt_menuitem_get_instance_private(item);

	priv->callback = callback;
	priv->callbackdata = data;
}

void gnt_menuitem_set_submenu(GntMenuItem *item, GntMenu *menu)
{
	GntMenuItemPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_MENU_ITEM(item));
	priv = gnt_menuitem_get_instance_private(item);

	if (priv->submenu) {
		gnt_widget_destroy(GNT_WIDGET(priv->submenu));
	}
	priv->submenu = menu;
}

GntMenu *gnt_menuitem_get_submenu(GntMenuItem *item)
{
	GntMenuItemPrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_MENU_ITEM(item), NULL);
	priv = gnt_menuitem_get_instance_private(item);

	return priv->submenu;
}

void
gnt_menuitem_set_trigger(GntMenuItem *item, gchar trigger)
{
	GntMenuItemPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_MENU_ITEM(item));
	priv = gnt_menuitem_get_instance_private(item);

	priv->trigger = trigger;
}

gchar
gnt_menuitem_get_trigger(GntMenuItem *item)
{
	GntMenuItemPrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_MENU_ITEM(item), 0);
	priv = gnt_menuitem_get_instance_private(item);

	return priv->trigger;
}

void
gnt_menuitem_set_id(GntMenuItem *item, const gchar *id)
{
	GntMenuItemPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_MENU_ITEM(item));
	priv = gnt_menuitem_get_instance_private(item);

	g_free(priv->id);
	priv->id = g_strdup(id);
}

const gchar *
gnt_menuitem_get_id(GntMenuItem *item)
{
	GntMenuItemPrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_MENU_ITEM(item), NULL);
	priv = gnt_menuitem_get_instance_private(item);

	return priv->id;
}

gboolean gnt_menuitem_activate(GntMenuItem *item)
{
	GntMenuItemPrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_MENU_ITEM(item), FALSE);
	priv = gnt_menuitem_get_instance_private(item);

	g_signal_emit(item, signals[SIG_ACTIVATE], 0);
	if (priv->callback) {
		priv->callback(item, priv->callbackdata);
		return TRUE;
	}
	return FALSE;
}

void
gnt_menuitem_set_visible(GntMenuItem *item, gboolean visible)
{
	GntMenuItemPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_MENU_ITEM(item));
	priv = gnt_menuitem_get_instance_private(item);

	priv->visible = visible;
}

gboolean
gnt_menuitem_is_visible(GntMenuItem *item)
{
	GntMenuItemPrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_MENU_ITEM(item), FALSE);
	priv = gnt_menuitem_get_instance_private(item);

	return priv->visible;
}

void
gnt_menuitem_set_text(GntMenuItem *item, const gchar *text)
{
	GntMenuItemPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_MENU_ITEM(item));
	priv = gnt_menuitem_get_instance_private(item);

	g_free(priv->text);
	priv->text = g_strdup(text);
}

/* Internal. */
const gchar *
gnt_menuitem_get_text(GntMenuItem *item)
{
	GntMenuItemPrivate *priv = NULL;
	g_return_val_if_fail(GNT_IS_MENU_ITEM(item), NULL);
	priv = gnt_menuitem_get_instance_private(item);
	return priv->text;
}
