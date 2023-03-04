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

#if !defined(GNT_GLOBAL_HEADER_INSIDE) && !defined(GNT_COMPILATION)
# error "only <gnt.h> may be included directly"
#endif

#ifndef GNT_MENUITEM_H
#define GNT_MENUITEM_H

#include <glib.h>
#include <glib-object.h>

typedef struct _GntMenuItem GntMenuItem;

#include "gntmenu.h"

#define GNT_TYPE_MENU_ITEM gnt_menuitem_get_type()

/**
 * GntMenuItemCallback:
 * @item: The menu item which was activated.
 * @data: The user data specified in gnt_menuitem_set_callback().
 *
 * A callback for when a menu item is activated.
 */
typedef void (*GntMenuItemCallback)(GntMenuItem *item, gpointer data);

/**
 * GntMenuItemClass:
 *
 * The class structure for #GntMenuItem.
 */
struct _GntMenuItemClass
{
	/*< private >*/
	GObjectClass parent;

	/*< private >*/
	gpointer reserved[4];
};

G_BEGIN_DECLS

G_DECLARE_DERIVABLE_TYPE(GntMenuItem, gnt_menuitem, GNT, MENU_ITEM, GObject)

/**
 * gnt_menuitem_new:
 * @text:   Label for the menuitem.
 *
 * Create a new menuitem.
 *
 * Returns:  The newly created menuitem.
 */
GntMenuItem *gnt_menuitem_new(const gchar *text);

/**
 * gnt_menuitem_set_callback:
 * @item:     The menuitem.
 * @callback: (scope async): The callback function.
 * @data:     Data to send to the callback function.
 *
 * Set a callback function for a menuitem.
 */
void gnt_menuitem_set_callback(GntMenuItem *item, GntMenuItemCallback callback, gpointer data);

/**
 * gnt_menuitem_set_submenu:
 * @item:  The menuitem.
 * @menu:  The submenu.
 *
 * Set a submenu for a menuitem. A menuitem with a submenu cannot have a callback.
 */
void gnt_menuitem_set_submenu(GntMenuItem *item, GntMenu *menu);

/**
 * gnt_menuitem_get_submenu:
 * @item:   The menuitem.
 *
 * Get the submenu for a menuitem.
 *
 * Returns: (transfer none): The submenu, or %NULL.
 *
 * Since: 2.3.0
 */
GntMenu *gnt_menuitem_get_submenu(GntMenuItem *item);

/**
 * gnt_menuitem_set_trigger:
 * @item:     The menuitem
 * @trigger:  The key that will trigger the item when the parent manu is visible
 *
 * Set a trigger key for the item.
 */
void gnt_menuitem_set_trigger(GntMenuItem *item, gchar trigger);

/**
 * gnt_menuitem_get_trigger:
 * @item:   The menuitem
 *
 * Get the trigger key for a menuitem.
 *
 * See gnt_menuitem_set_trigger().
 *
 * Returns: The trigger key for the menuitem.
 */
gchar gnt_menuitem_get_trigger(GntMenuItem *item);

/**
 * gnt_menuitem_set_id:
 * @item:   The menuitem.
 * @id:     The ID for the menuitem.
 *
 * Set an ID for the menuitem.
 *
 * Since: 2.3.0
 */
void gnt_menuitem_set_id(GntMenuItem *item, const gchar *id);

/**
 * gnt_menuitem_get_id:
 * @item:   The menuitem.
 *
 * Get the ID of the menuitem.
 *
 * Returns:  The ID for the menuitem.
 *
 * Since: 2.3.0
 */
const gchar *gnt_menuitem_get_id(GntMenuItem *item);

/**
 * gnt_menuitem_activate:
 * @item:   The menuitem.
 *
 * Activate a menuitem.
 * Activating the menuitem will first trigger the 'activate' signal for the
 * menuitem. Then the callback for the menuitem is triggered, if there is one.
 *
 * Returns:  Whether the callback for the menuitem was called.
 *
 * Since: 2.3.0
 */
gboolean gnt_menuitem_activate(GntMenuItem *item);

/**
 * gnt_menuitem_set_visible:
 * @item: The menuitem.
 * @visible: %TRUE to make @item visible, %FALSE to hide it.
 *
 * Sets @item visible or not.
 *
 * Since: 3.0.0
 */
void
gnt_menuitem_set_visible(GntMenuItem *item, gboolean visible);

/**
 * gnt_menuitem_is_visible:
 * @item: The menuitem.
 *
 * Checks, if the @item is visible.
 *
 * Returns: %TRUE, if the @item is visible.
 *
 * Since: 3.0.0
 */
gboolean
gnt_menuitem_is_visible(GntMenuItem *item);

/**
 * gnt_menuitem_set_text:
 * @item: The menuitem.
 * @text: The new text.
 *
 * Changes the text for an @item.
 *
 * Since: 3.0.0
 */
void
gnt_menuitem_set_text(GntMenuItem *item, const gchar *text);

G_END_DECLS

#endif /* GNT_MENUITEM_H */
