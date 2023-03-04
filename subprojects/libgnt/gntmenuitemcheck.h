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

#ifndef GNT_MENU_ITEM_CHECK_H
#define GNT_MENU_ITEM_CHECK_H

#include "gntcolors.h"
#include "gntkeys.h"
#include "gntmenuitem.h"

#define GNT_TYPE_MENU_ITEM_CHECK gnt_menuitem_check_get_type()

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(GntMenuItemCheck, gnt_menuitem_check, GNT, MENU_ITEM_CHECK,
                     GntMenuItem)

/**
 * gnt_menuitem_check_new:
 * @text:  The text for the menuitem.
 *
 * Create a new menuitem.
 *
 * Returns:  The newly created menuitem.
 */
GntMenuItem * gnt_menuitem_check_new(const char *text);

/**
 * gnt_menuitem_check_get_checked:
 * @item:  The menuitem.
 *
 * Check whether the menuitem is checked or not.
 *
 * Returns: %TRUE if the item is checked, %FALSE otherwise.
 */
gboolean gnt_menuitem_check_get_checked(GntMenuItemCheck *item);

/**
 * gnt_menuitem_check_set_checked:
 * @item:  The menuitem.
 * @set:   %TRUE if the item should be checked, %FALSE otherwise.
 *
 * Set whether the menuitem is checked or not.
 */
void gnt_menuitem_check_set_checked(GntMenuItemCheck *item, gboolean set);

G_END_DECLS

#endif /* GNT_MENU_ITEM_CHECK_H */
