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
#include "gntmenuitemcheck.h"

struct _GntMenuItemCheck
{
	GntMenuItem parent;

	gboolean checked;
};

G_DEFINE_TYPE(GntMenuItemCheck, gnt_menuitem_check, GNT_TYPE_MENU_ITEM)

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/

static void
gnt_menuitem_check_class_init(G_GNUC_UNUSED GntMenuItemCheckClass *klass)
{
}

static void
gnt_menuitem_check_init(G_GNUC_UNUSED GntMenuItemCheck *item)
{
}

/******************************************************************************
 * GntMenuItemCheck API
 *****************************************************************************/
GntMenuItem *gnt_menuitem_check_new(const char *text)
{
	GntMenuItem *item = g_object_new(GNT_TYPE_MENU_ITEM_CHECK, NULL);
	GntMenuItem *menuitem = GNT_MENU_ITEM(item);

	gnt_menuitem_set_text(menuitem, text);

	return item;
}

gboolean gnt_menuitem_check_get_checked(GntMenuItemCheck *item)
{
		return item->checked;
}

void gnt_menuitem_check_set_checked(GntMenuItemCheck *item, gboolean set)
{
		item->checked = set;
}

