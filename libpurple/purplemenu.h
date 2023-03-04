/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * Purple is the legal property of its developers, whose names are too numerous
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_MENU_H
#define PURPLE_MENU_H

#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET "dynamic-target"

/**
 * PurpleMenuWalkFunc:
 * @model: The current [class@Gio.MenuModel] being walked.
 * @index: The index of the item.
 * @data: User data.
 *
 * Used as a parameter to [func@Purple.menu_walk]. While walking, @model will
 * be updated to point to the current section or submenu and will only be the
 * model that was passed to [func@Purple.menu_walk] for its immediate
 * children.
 *
 * Since: 3.0.0
 */
typedef void (*PurpleMenuWalkFunc)(GMenuModel *model, gint index, gpointer data);

/**
 * purple_menu_walk:
 * @model: A [class@Gio.MenuModel] to walk.
 * @func: (scope call): The function to call.
 * @data: User data to pass for func.
 *
 * Recursively calls @func for each item in @model and all of its children.
 *
 * Since: 3.0.0
 */
void purple_menu_walk(GMenuModel *model, PurpleMenuWalkFunc func, gpointer data);

/**
 * purple_menu_populate_dynamic_targets:
 * @menu: The menu instance to modify.
 * @first_property: The name of the first property of dynamic targets to
 *                  replace.
 * @...: The value of the first property, followed optionally by more
 *       name/value pairs, followed by %NULL.
 *
 * Updates @menu by adding a target property when an item with an attribute
 * named "dynamic-target" is found.
 *
 * The value for the target is set to the matching value from the passed in
 * parameters.
 *
 * For example, if you need to set the target to an account, you would set
 * the "dynamic-target" attribute of your menu item to "account" and then
 * call purple_menu_populate_dynamic_targets() with a property pair of
 * "account" and [method@Purple.ContactInfo.get_id].
 *
 * Since: 3.0.0
 */
void purple_menu_populate_dynamic_targets(GMenu *menu, const gchar *first_property, ...) G_GNUC_NULL_TERMINATED;

/**
 * purple_menu_populate_dynamic_targetsv: (rename-to purple_menu_populate_dynamic_targets):
 * @menu: The menu instance to modify.
 * @properties: (element-type utf8 utf8): A hash table where the keys are the
 *              names of the properties of dynamic targets to be replaced, and
 *              the values are the replacements.
 *
 * Updates @menu by adding a target property when an item with an attribute
 * named "dynamic-target" is found.
 *
 * The value for the target is set to the matching value from @properties.
 *
 * For example, if you need to set the target to an account, you would set the
 * "dynamic-target" attribute of your menu item to "account" and then call
 * [func@Purple.menu_populate_dynamic_targetsv] with a hash table containing
 * the key `"account"` and value from [method@Purple.ContactInfo.get_id].
 *
 * Since: 3.0.0
 */
void purple_menu_populate_dynamic_targetsv(GMenu *menu, GHashTable *properties);

/**
 * purple_menu_copy:
 * @model: The [class@Gio.MenuModel] instance to copy.
 *
 * Creates a full copy of @model as a new [class@Gio.Menu]. If @model was not
 * a [class@Gio.Menu] instance, any additional functionality will be lost.
 *
 * Returns: (transfer full): The new menu.
 *
 * Since: 3.0.0
 */
GMenu *purple_menu_copy(GMenuModel *model);

G_END_DECLS

#endif /* PURPLE_MENU_H */
