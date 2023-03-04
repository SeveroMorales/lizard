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

#ifndef GNT_MENU_H
#define GNT_MENU_H

#include "gnttree.h"
#include "gntcolors.h"
#include "gntkeys.h"

#define GNT_TYPE_MENU gnt_menu_get_type()

typedef struct _GntMenu			GntMenu;

#include "gntmenuitem.h"

/**
 * GntMenuType:
 * @GNT_MENU_TOPLEVEL: Menu for a toplevel window
 * @GNT_MENU_POPUP:    A popup menu
 *
 * A toplevel-menu is displayed at the top of the screen, and it spans across
 * the entire width of the screen.
 * A popup-menu could be displayed, for example, as a context menu for widgets.
 */
typedef enum
{
	GNT_MENU_TOPLEVEL = 1,
	GNT_MENU_POPUP,
} GntMenuType;

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(GntMenu, gnt_menu, GNT, MENU, GntTree)

/**
 * gnt_menu_new:
 * @type:  The type of the menu, whether it's a toplevel menu or a popup menu.
 *
 * Create a new menu.
 *
 * Returns:  The newly created menu.
 */
GntWidget * gnt_menu_new(GntMenuType type);

/**
 * gnt_menu_add_item:
 * @menu:   The menu.
 * @item:   The item to add to the menu.
 *
 * Add an item to the menu.
 */
void gnt_menu_add_item(GntMenu *menu, GntMenuItem *item);

/**
 * gnt_menu_get_item:
 * @menu:   The menu.
 * @id:     The ID for an item.
 *
 * Return the GntMenuItem with the given ID.
 *
 * Returns: (transfer none): The menuitem with the given ID, or %NULL.
 *
 * Since: 2.3.0
 */
GntMenuItem *gnt_menu_get_item(GntMenu *menu, const char *id);

G_END_DECLS

#endif /* GNT_MENU_H */
